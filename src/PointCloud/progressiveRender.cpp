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
	float colorBarLength = 343.0;//设置颜色条的长度
	colors.resize(4, colorBarLength);
	//------设置为jet颜色条---------//
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

// 初始化点云
void ProgressiveRender::init() {
	std::cout << "PointsNum: " << pcd->points_num << std::endl;
	// 释放OpenGL缓冲(原先点云数据缓冲区)
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glDeleteBuffers(1, &pointcloudBuffers[i]->VBO);
		glDeleteVertexArrays(1, &pointcloudBuffers[i]->VAO);
	}
	pointcloudBuffers.clear();
	//upload Data
	upload = new UpLoader(pcd);
	// --------------------------- 经计算着色器打乱顶点后点云缓冲区管理 -------------------------------
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
// 渐进式渲染
void ProgressiveRender::renderPointCloudProgressive() {
	if (!pcd->getVisible()) {
		return;
	}

	if (pcd->getAttributeMode() != currentAttributeMode) {
		// uploadAttributeMode
		uploadAttribute();
	}

	// 声明片段着色器的输出 (location指定buffer索引) 写入到帧缓冲fbo内的两个纹理附件中
	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, buffers);

	reproject(); // 重投影
	fillFixed(); // 空洞填充
	createVBO(); // 创建新的重投影VBO

	glUseProgram(0);
	// 启用深度缓冲写入
	glDepthMask(true);
}

// 重投影
void ProgressiveRender::reproject() {
	ProgressiveRenderState *state = getRenderState();

	reprojectShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	int ATTRIBUTE_MODE = pcd->getAttributeMode();
	float pointSize = pcd->getPointSize();
	// 颜色从纹理采样 否则直接从原始传入数据读取
	if (ATTRIBUTE_MODE == FROM_GRADIENT) {
		reprojectShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// 颜色按照色带采样 按强度着色
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
	// 绘制
	glBindVertexArray(state->reprojectBuffer->VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, state->IndirectCommand);
	glDrawArraysIndirect(GL_POINTS, 0); // 间接绘制 绘制命令由缓存获得
	glBindVertexArray(0);
}

// 空洞填充
void ProgressiveRender::fillFixed() {
	ProgressiveRenderState *state = getRenderState();
	fillShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	int ATTRIBUTE_MODE = pcd->getAttributeMode();
	float pointSize = pcd->getPointSize();
	// 颜色从纹理采样 否则直接从原始传入数据读取
	if (ATTRIBUTE_MODE == FROM_GRADIENT) {
		fillShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// 颜色按照色带采样 按强度着色
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

	// 绘制(打乱后的缓冲区)
	int budget = 1000000; // 单次填充预算
	int numPoints = pcd->points_num; // 点云总量
	if (pointcloudBuffers.size() == 1) {
		GLBuffer *pcdBuffer = pointcloudBuffers[0];
		glBindVertexArray(pcdBuffer->VAO);
		int left = numPoints - state->fillOffset; // 剩下未渲染过的点 = 总量 - 偏移
		int count = min(left, budget);
		fillShader->setUniformValue("uOffset", 0);
		glDrawArrays(GL_POINTS, state->fillOffset, count);
		// 更新偏移
		state->fillOffset = (state->fillOffset + count) % numPoints;
	}
	else {
		// 点云数量极多 存在多个缓冲区的情况
		int maxBufferSize = pointcloudBuffers[0]->vertexCount; // 1000,000,000
		vector<int> cumBufferOffsets = { 0 }; // 累积缓冲区起始偏移 buffer1: 0 buffer2: 0 + sizeof(buffer1) ...
		vector<int> cumBufferSizes = { pointcloudBuffers[0]->vertexCount }; // 累积缓冲区大小
		for (int i = 1; i < pointcloudBuffers.size(); i++) {
			cumBufferOffsets.emplace_back(cumBufferOffsets[i - 1] + pointcloudBuffers[i - 1]->vertexCount);
			cumBufferSizes.emplace_back(cumBufferSizes[i - 1] + pointcloudBuffers[i]->vertexCount);
		}
		// 根据偏移计算当前渲染哪个buffer
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

// 创建新的vbo 供重投影使用
void ProgressiveRender::createVBO() {
	ProgressiveRenderState *state = getRenderState();
	// 绑定着色器
	createVBOShader->bind();
	UINT32 indirectData[5] = { 0, 1, 0, 0, 0 }; // count由原子计数器计数得到 1 为多实例渲染个数
	glNamedBufferSubData(state->IndirectCommand, 0, sizeof(indirectData), indirectData);
	// 绑定纹理到Image实现读写
	createVBOShader->setUniformValue("uIndices", 0);
	createVBOShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	glBindImageTexture(0, fbo->textures[1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	// 绑定间接绘制命令缓冲到着色器 进行写入修改
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, state->IndirectCommand);
	// 绑定重投影数据VBO到着色器进行数据写入
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, state->reprojectBuffer->VBO);
	// 绑定点云数据缓冲到着色器进行数据来源读取
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	// 本地工作组大小
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

	// 内存屏障
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// 解绑缓冲区
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
// 计算着色器控制裁剪
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
	// 传递polygon数据
	GLuint polygonBuffer;
	glCreateBuffers(1, &polygonBuffer);
	glNamedBufferData(polygonBuffer, 4 + polygon.size() * 8, nullptr, GL_STATIC_DRAW);
	int num = polygon.size();
	glNamedBufferSubData(polygonBuffer, 0, 4, &num);
	glNamedBufferSubData(polygonBuffer, 4, polygon.size() * 8, polygon.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, polygonBuffer);

	// 绑定点云数据缓冲到着色器进行数据来源读取
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	// 本地工作组大小
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
	// 内存屏障
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// 解绑缓冲区
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
	glDeleteBuffers(1, &polygonBuffer);
	//segement = false;
}

// 上传替换属性数据
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

