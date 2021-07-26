/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-19 15:44:30
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-19 15:44:30
 */
#include <progressiveRender.h>
#include <PointCloud/renderingWidget.h>

static Eigen::MatrixXf getColorStrip() {
	Eigen::MatrixXf colors;
	float colorBarLength = 343.0;//������ɫ���ĳ���
	colors.resize(4, colorBarLength);
	//------����Ϊjet��ɫ��---------//
	float tempLength = colorBarLength / 4;
	for (int i = 0; i < tempLength / 2; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f(0, 0, (tempLength / 2 + i) / tempLength, 1);
	}
	for (int i = tempLength / 2 + 1; i < tempLength / 2 + tempLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f(0, (i - tempLength / 2) / tempLength, 1, 1);
	}
	for (int i = tempLength / 2 + tempLength + 1; i < tempLength / 2 + 2 * tempLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f((i - tempLength - tempLength / 2) / tempLength, 1, (tempLength * 2 + tempLength / 2 - i) / tempLength, 1);
	}
	for (int i = tempLength / 2 + 2 * tempLength + 1; i < tempLength / 2 + 3 * tempLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f(1, (tempLength * 3 + tempLength / 2 - i) / tempLength, 0, 1);
	}
	for (int i = tempLength / 2 + 3 * tempLength + 1; i < colorBarLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f((colorBarLength - i + tempLength / 2) / tempLength, 0, 0, 1);
	}

	return colors;
}

ProgressiveRender::ProgressiveRender(RenderWidget *glWidget, PointCloud *pcd) {
	initializeOpenGLFunctions();

	this->glWidget = glWidget;
	this->pcd = pcd;

	this->fbo = glWidget->fbo;
	this->camera = glWidget->camera;
	this->window = glWidget->window;
	this->gradientImage = glWidget->gradientImage;

	this->reprojectShader = glWidget->reprojectShader;
	this->fillShader = glWidget->fillShader;
	this->createVBOShader = glWidget->createVBOShader;

	this->SegmentShader = glWidget->SegmentShader;

	this->name = pcd->name;

	this->currentAttributeMode = pcd->getAttributeMode();

	glWidget->makeCurrent();
	fbo->bind();
	init();
}

ProgressiveRender::~ProgressiveRender() {

}

// ��ʼ������
void ProgressiveRender::init() {
	std::cout << "PointsNum: " << pcd->points_num << std::endl;
	// �ͷ�OpenGL����(ԭ�ȵ������ݻ�����)
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glDeleteBuffers(1, &pointcloudBuffers[i]->VBO);
		glDeleteVertexArrays(1, &pointcloudBuffers[i]->VAO);
	}
	pointcloudBuffers.clear();
	//upload Data
	upload = new UpLoader(pcd);
	// --------------------------- ��������ɫ�����Ҷ������ƻ��������� -------------------------------
	vector<GLBufferAttribute> pcdAttributes;
	GLBufferAttribute pcdAttribute1("position", 0, 3, GL_FLOAT, GL_FALSE, 20, 0);
	pcdAttributes.emplace_back(pcdAttribute1);
	GLBufferAttribute pcdAttribute2("color", 1, 1, GL_INT, GL_FALSE, 20, 12);
	pcdAttributes.emplace_back(pcdAttribute2);
	GLBufferAttribute pcdAttribute3("show", 2, 1, GL_INT, GL_FALSE, 20, 16);
	pcdAttributes.emplace_back(pcdAttribute3);

	int maxPointsPerBuffer = upload->maxPointsPerBuffer;
	int numPointsLeft = pcd->points_num;
	for (int i = 0; i < upload->vertexBuffers.size(); i++) {
		int numPointsInBuffer = numPointsLeft > maxPointsPerBuffer ? maxPointsPerBuffer : numPointsLeft;
		GLBuffer *pcdBuffer = new GLBuffer(upload->vertexBuffers[i]);
		pcdBuffer->setInterleaved(pcdAttributes);
		pcdBuffer->vertexCount = numPointsInBuffer;
		numPointsLeft -= numPointsInBuffer;
		pointcloudBuffers.emplace_back(pcdBuffer);
	}
}
// ------------------------------------------------------------------------------- Core Method --------------------------------------------------------------------------------------------
// ����ʽ��Ⱦ
void ProgressiveRender::renderPointCloudProgressive() {
	if (!pcd->getVisible()) {
		return;
	}

	if (pcd->getAttributeMode() != currentAttributeMode) {
		// uploadAttributeMode
		uploadAttribute();
	}

	// ����Ƭ����ɫ������� (locationָ��buffer����) д�뵽֡����fbo�ڵ�������������
	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, buffers);

	reproject(); // ��ͶӰ
	fillFixed(); // �ն����
	createVBO(); // �����µ���ͶӰVBO

	glUseProgram(0);
	// ������Ȼ���д��
	glDepthMask(true);
}

// ��ͶӰ
void ProgressiveRender::reproject() {
	ProgressiveRenderState *state = getRenderState();

	reprojectShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	int ATTRIBUTE_MODE = pcd->getAttributeMode();
	float pointSize = pcd->getPointSize();
	// ��ɫ��������� ����ֱ�Ӵ�ԭʼ�������ݶ�ȡ
	if (ATTRIBUTE_MODE == FROM_GRADIENT) {
		reprojectShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// ��ɫ����ɫ������ ��ǿ����ɫ
	if (ATTRIBUTE_MODE == FROM_IDENTISITY) {
		int maxValue = pcd->getmaxIdensity();
		int minValue = pcd->getminIdensity();
		void* colorStrip = pcd->getColorStrip();
		reprojectShader->setUniformBlockValue(uniformBlocks, "colorStrip", "colorStrip.maxValue", &maxValue);
		reprojectShader->setUniformBlockValue(uniformBlocks, "colorStrip", "colorStrip.minValue", &minValue);
		reprojectShader->setUniformBlockValue(uniformBlocks, "colorStrip", "colorStrip.colors", colorStrip);
	}
	reprojectShader->setUniformValue("uAttributeMode", ATTRIBUTE_MODE);
	reprojectShader->setUniformValue("uWorldViewProj", transform);
	reprojectShader->setUniformValue("pointSize", pointSize);
	// ����
	glBindVertexArray(state->reprojectBuffer->VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, state->IndirectCommand);
	glDrawArraysIndirect(GL_POINTS, 0); // ��ӻ��� ���������ɻ�����
	glBindVertexArray(0);
}

// �ն����
void ProgressiveRender::fillFixed() {
	ProgressiveRenderState *state = getRenderState();
	fillShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	int ATTRIBUTE_MODE = pcd->getAttributeMode();
	float pointSize = pcd->getPointSize();
	// ��ɫ��������� ����ֱ�Ӵ�ԭʼ�������ݶ�ȡ
	if (ATTRIBUTE_MODE == FROM_GRADIENT) {
		fillShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// ��ɫ����ɫ������ ��ǿ����ɫ
	if (ATTRIBUTE_MODE == FROM_IDENTISITY) {
		int maxValue = pcd->getmaxIdensity();
		int minValue = pcd->getminIdensity();
		void* colorStrip = pcd->getColorStrip();
		fillShader->setUniformBlockValue(uniformBlocks, "colorStrip", "colorStrip.maxValue", &maxValue);
		fillShader->setUniformBlockValue(uniformBlocks, "colorStrip", "colorStrip.minValue", &minValue);
		fillShader->setUniformBlockValue(uniformBlocks, "colorStrip", "colorStrip.colors", colorStrip);
	}
	fillShader->setUniformValue("uAttributeMode", ATTRIBUTE_MODE);
	fillShader->setUniformValue("uWorldViewProj", transform);
	fillShader->setUniformValue("pointSize", pointSize);

	// ����(���Һ�Ļ�����)
	int budget = 1000000; // �������Ԥ��
	int numPoints = pcd->points_num; // ��������
	if (pointcloudBuffers.size() == 1) {
		GLBuffer *pcdBuffer = pointcloudBuffers[0];
		glBindVertexArray(pcdBuffer->VAO);
		int left = numPoints - state->fillOffset; // ʣ��δ��Ⱦ���ĵ� = ���� - ƫ��
		int count = min(left, budget);
		fillShader->setUniformValue("uOffset", 0);
		glDrawArrays(GL_POINTS, state->fillOffset, count);
		// ����ƫ��
		state->fillOffset = (state->fillOffset + count) % numPoints;
	}
	else {
		// ������������ ���ڶ�������������
		int maxBufferSize = pointcloudBuffers[0]->vertexCount; // 1000,000,000
		vector<int> cumBufferOffsets = { 0 }; // �ۻ���������ʼƫ�� buffer1: 0 buffer2: 0 + sizeof(buffer1) ...
		vector<int> cumBufferSizes = { pointcloudBuffers[0]->vertexCount }; // �ۻ���������С
		for (int i = 1; i < pointcloudBuffers.size(); i++) {
			cumBufferOffsets.emplace_back(cumBufferOffsets[i - 1] + pointcloudBuffers[i - 1]->vertexCount);
			cumBufferSizes.emplace_back(cumBufferSizes[i - 1] + pointcloudBuffers[i]->vertexCount);
		}
		// ����ƫ�Ƽ��㵱ǰ��Ⱦ�ĸ�buffer
		int bufferIndex = int(state->fillOffset / maxBufferSize);
		GLBuffer *pcdBuffer = pointcloudBuffers[bufferIndex];
		int count = min(budget, cumBufferSizes[bufferIndex] - state->fillOffset);
		glBindVertexArray(pcdBuffer->VAO);
		fillShader->setUniformValue("uOffset", cumBufferOffsets[bufferIndex]);
		glDrawArrays(GL_POINTS, state->fillOffset - cumBufferOffsets[bufferIndex], count);
		state->fillOffset = (state->fillOffset + count) % numPoints;
	}
	glBindVertexArray(0);
}

// �����µ�vbo ����ͶӰʹ��
void ProgressiveRender::createVBO() {
	ProgressiveRenderState *state = getRenderState();
	// ����ɫ��
	createVBOShader->bind();
	UINT32 indirectData[5] = { 0, 1, 0, 0, 0 }; // count��ԭ�Ӽ����������õ� 1 Ϊ��ʵ����Ⱦ����
	glNamedBufferSubData(state->IndirectCommand, 0, sizeof(indirectData), indirectData);
	// ������Imageʵ�ֶ�д
	createVBOShader->setUniformValue("uIndices", 0);
	createVBOShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	glBindImageTexture(0, fbo->textures[1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	// �󶨼�ӻ�������嵽��ɫ�� ����д���޸�
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, state->IndirectCommand);
	// ����ͶӰ����VBO����ɫ����������д��
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, state->reprojectBuffer->VBO);
	// �󶨵������ݻ��嵽��ɫ������������Դ��ȡ
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	// ���ع������С
	struct localSize {
		int x = 16;
		int y = 16;
	};
	localSize localsize;
	int groups[3] = {
		int(fbo->width / localsize.x + 1),
		int(fbo->height / localsize.y + 1),
		1
	};

	if (fbo->samples == 2) {
		groups[0] *= 2;
	}
	else if (fbo->samples == 4) {
		groups[0] *= 2;
		groups[1] *= 2;
	}
	else if (fbo->samples == 8) {
		groups[0] *= 4;
		groups[1] *= 2;
	}
	else if (fbo->samples == 16) {
		groups[0] *= 4;
		groups[1] *= 4;
	}

	// �ڴ�����
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// ��󻺳���
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
}

ProgressiveRenderState* ProgressiveRender::getRenderState() {
	if (renderState == nullptr) {
		renderState = new ProgressiveRenderState();
	}
	return renderState;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ������ɫ�����Ʋü�
void ProgressiveRender::Segment(vector<Point> polygon) {
	std::cout << "Start Segement!" << std::endl;
	for (int i = 0; i < polygon.size(); i++) {
		polygon[i].x = polygon[i].x / (float)window->width * 2 - 1.0;
		polygon[i].y = - polygon[i].y / (float)window->height * 2 + 1.0;
		std::cout << polygon[i].x << "," << polygon[i].y << std::endl;
	}
	SegmentShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	SegmentShader->setUniformValue("transform", transform);
	SegmentShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	// ����polygon����
	GLuint polygonBuffer;
	glCreateBuffers(1, &polygonBuffer);
	glNamedBufferData(polygonBuffer, 4 + polygon.size() * 8, nullptr, GL_STATIC_DRAW);
	int num = polygon.size();
	glNamedBufferSubData(polygonBuffer, 0, 4, &num);
	glNamedBufferSubData(polygonBuffer, 4, polygon.size() * 8, polygon.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, polygonBuffer);

	// �󶨵������ݻ��嵽��ɫ������������Դ��ȡ
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	// ���ع������С
	struct localSize {
		int x = 1024;
		int y = 1;
	};
	localSize localsize;
	int groups[3] = {
		pcd->points_num / localsize.x + 1,
		1,
		1
	};
	// �ڴ�����
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// ��󻺳���
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
	glDeleteBuffers(1, &polygonBuffer);
	//segement = false;
}

// �ϴ��滻��������
void ProgressiveRender::uploadAttribute() {
	int AttributeMode = pcd->getAttributeMode();
	switch (AttributeMode) {
	case FROM_RBG:{
		auto color = pcd->colors;
		Eigen::MatrixXi int_color;
		int_color.resize(1, color.cols());
		memcpy((void*)int_color.data(), (void*)color.data(), color.cols() * 4);
		upload->uploadChunkAttribute(int_color.data(), 0, int_color.cols());
	}
	break;
	case FROM_Label: {
		auto labels = pcd->labels;
		upload->uploadChunkAttribute(labels.data(), 0, labels.cols());
	}
	break;
	case FROM_IDENTISITY: {
		auto idensity = pcd->intensity;
		upload->uploadChunkAttribute(idensity.data(), 0, idensity.cols());
		Eigen::MatrixXf colorStrip = getColorStrip();
		pcd->setColorStrip(colorStrip.cols(), colorStrip);
	}
	break;
	default:
		break;
	}
	currentAttributeMode = AttributeMode;
}

