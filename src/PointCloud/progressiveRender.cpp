/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-19 15:44:30
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-31 23:33:25
 */
#include <progressiveRender.h>
#include <PointCloud/renderingWidget.h>
#include <Eigen/Dense>
#include <Eigen/Cholesky>
#include <omp.h>

static Eigen::MatrixXf getColorStrip() {
	Eigen::MatrixXf colors;
	float colorBarLength = 343.0;
	colors.resize(4, colorBarLength);

	float tempLength = colorBarLength / 4;
	for (int i = 0; i < tempLength / 2; i++)
	{
		colors.col(i) = Eigen::Vector4f(0, 0, (tempLength / 2 + i) / tempLength, 1);
	}
	for (int i = tempLength / 2 + 1; i < tempLength / 2 + tempLength; i++)
	{
		colors.col(i) = Eigen::Vector4f(0, (i - tempLength / 2) / tempLength, 1, 1);
	}
	for (int i = tempLength / 2 + tempLength + 1; i < tempLength / 2 + 2 * tempLength; i++)
	{
		colors.col(i) = Eigen::Vector4f((i - tempLength - tempLength / 2) / tempLength, 1, (tempLength * 2 + tempLength / 2 - i) / tempLength, 1);
	}
	for (int i = tempLength / 2 + 2 * tempLength + 1; i < tempLength / 2 + 3 * tempLength; i++)
	{
		colors.col(i) = Eigen::Vector4f(1, (tempLength * 3 + tempLength / 2 - i) / tempLength, 0, 1);
	}
	for (int i = tempLength / 2 + 3 * tempLength + 1; i < colorBarLength; i++)
	{
		colors.col(i) = Eigen::Vector4f((colorBarLength - i + tempLength / 2) / tempLength, 0, 0, 1);
	}

	return colors;
}

ProgressiveRender::ProgressiveRender(RenderWidget *glWidget, std::shared_ptr<PointCloud> &pcd) {
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
	this->resumeSegmentShader = glWidget->resumeSegmentShader;

	this->selectShader = glWidget->selectShader;

	this->name = pcd->name;

	this->currentAttributeMode = pcd->getAttributeMode();

	glWidget->makeCurrent();
	fbo->bind();
	init();
}

ProgressiveRender::~ProgressiveRender() {
	glWidget->makeCurrent();
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glDeleteBuffers(1, &pointcloudBuffers[i]->VBO);
		glDeleteVertexArrays(1, &pointcloudBuffers[i]->VAO);
		delete pointcloudBuffers[i];
	}
	for (int i = 0; i < uniformBlocks.size(); i++) {
		delete uniformBlocks[i];
	}
	glDeleteBuffers(1, &upload->indexBuffer);
	glDeleteBuffers(1, &upload->Chunk4B);
	glDeleteBuffers(1, &upload->Chunk16B);
	delete upload;
	delete renderState;
}

void ProgressiveRender::init() {
	glWidget->makeCurrent();
	fbo->bind();
	std::cout << "PointsNum: " << pcd->points_num << std::endl;
	
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glDeleteBuffers(1, &pointcloudBuffers[i]->VBO);
		glDeleteVertexArrays(1, &pointcloudBuffers[i]->VAO);
		delete pointcloudBuffers[i];
	}
	pointcloudBuffers.clear();
	//upload Data
	if (upload != nullptr) {
		glDeleteBuffers(1, &upload->indexBuffer);
		glDeleteBuffers(1, &upload->Chunk4B);
		glDeleteBuffers(1, &upload->Chunk16B);
		delete upload;
	}
	upload = new UpLoader(pcd);
	// ---------------------------  -------------------------------
	vector<GLBufferAttribute> pcdAttributes;
	GLBufferAttribute pcdAttribute1("position", 0, 3, GL_FLOAT, GL_FALSE, 20, 0); // xyz 12字节
	pcdAttributes.emplace_back(pcdAttribute1);
	GLBufferAttribute pcdAttribute2("attribute", 1, 1, GL_INT, GL_FALSE, 20, 12); // color或其他自定义属性 4字节
	pcdAttributes.emplace_back(pcdAttribute2);
	GLBufferAttribute pcdAttribute3("show", 2, 1, GL_INT, GL_FALSE, 20, 16); // show属性 4字节(shader里只能4字节)
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
void ProgressiveRender::renderPointCloudProgressive() {
	if (!pcd->getVisible()) {
		return;
	}

	// 属性更改 上载属性
	if (pcd->getAttributeMode() != currentAttributeMode) {
		// uploadAttributeMode
		int AttributeMode = pcd->getAttributeMode();
		uploadAttribute(AttributeMode);
		this->currentAttributeMode = AttributeMode;
	}

	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, buffers);

	reproject(); // 重投影
	fillFixed(); // 空洞填充
	createVBO(); // 创建VBO

	glUseProgram(0);
	
	glDepthMask(true);
}

void ProgressiveRender::reproject() {
	ProgressiveRenderState *state = getRenderState();

	reprojectShader->bind();
	Eigen::Matrix4f transform = camera->getTransform();
	int ATTRIBUTE_MODE = pcd->getAttributeMode();
	float pointSize = pcd->getPointSize();
	// 
	if (ATTRIBUTE_MODE == FROM_GRADIENT) {
		reprojectShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// 
	if (ATTRIBUTE_MODE == FROM_INTENSITY) {
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
	// 
	glBindVertexArray(state->reprojectBuffer->VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, state->IndirectCommand);
	glDrawArraysIndirect(GL_POINTS, 0); //

	glBindVertexArray(0);
	reprojectShader->unbind();
}

void ProgressiveRender::fillFixed() {
	ProgressiveRenderState *state = getRenderState();
	fillShader->bind();
	Eigen::Matrix4f transform = camera->getTransform();
	int ATTRIBUTE_MODE = pcd->getAttributeMode();
	float pointSize = pcd->getPointSize();
	// 
	if (ATTRIBUTE_MODE == FROM_GRADIENT) {
		fillShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// 
	if (ATTRIBUTE_MODE == FROM_INTENSITY) {
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

	// 
	int budget = 1000000; // 
	int numPoints = pcd->points_num; // 
	if (pointcloudBuffers.size() == 1) {
		GLBuffer *pcdBuffer = pointcloudBuffers[0];
		glBindVertexArray(pcdBuffer->VAO);
		int left = numPoints - state->fillOffset; // 
		int count = min(left, budget);
		fillShader->setUniformValue("uOffset", 0);
		glDrawArrays(GL_POINTS, state->fillOffset, count);
		// 
		state->fillOffset = (state->fillOffset + count) % numPoints;
	}
	else {
		// 
		int maxBufferSize = pointcloudBuffers[0]->vertexCount; // 1000,000,000
		vector<int> cumBufferOffsets = { 0 }; // 
		vector<int> cumBufferSizes = { pointcloudBuffers[0]->vertexCount }; // 
		for (int i = 1; i < pointcloudBuffers.size(); i++) {
			cumBufferOffsets.emplace_back(cumBufferOffsets[i - 1] + pointcloudBuffers[i - 1]->vertexCount);
			cumBufferSizes.emplace_back(cumBufferSizes[i - 1] + pointcloudBuffers[i]->vertexCount);
		}
		//
		int bufferIndex = int(state->fillOffset / maxBufferSize);
		GLBuffer *pcdBuffer = pointcloudBuffers[bufferIndex];
		int count = min(budget, cumBufferSizes[bufferIndex] - state->fillOffset);
		glBindVertexArray(pcdBuffer->VAO);
		fillShader->setUniformValue("uOffset", cumBufferOffsets[bufferIndex]);
		glDrawArrays(GL_POINTS, state->fillOffset - cumBufferOffsets[bufferIndex], count);
		state->fillOffset = (state->fillOffset + count) % numPoints;
	}
	glBindVertexArray(0);
	fillShader->unbind();
}

void ProgressiveRender::createVBO() {
	ProgressiveRenderState *state = getRenderState();
	// 
	createVBOShader->bind();
	UINT32 indirectData[5] = { 0, 1, 0, 0, 0 }; // 
	glNamedBufferSubData(state->IndirectCommand, 0, sizeof(indirectData), indirectData);
	// 
	createVBOShader->setUniformValue("uIndices", 0);
	createVBOShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	// 从纹理texture创建一个Image
	glBindImageTexture(0, fbo->textures[1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	// 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, state->IndirectCommand);
	// 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, state->reprojectBuffer->VBO);
	// 
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	// 
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

	//
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
	createVBOShader->unbind();
}

ProgressiveRenderState* ProgressiveRender::getRenderState() {
	if (renderState == nullptr) {
		renderState = new ProgressiveRenderState();
	}
	return renderState;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 裁剪 删除
// 裁剪区域 裁剪模式 裁剪时的相机转换矩阵(因为回退的时候需要保持前后转换矩阵一致)
void ProgressiveRender::Segment(vector<Point> polygon, int Selectmode, Eigen::Matrix4f segmentTransform) {
	std::cout << "Start Segement!" << std::endl;
	// polygon转换到标准化设备坐标下
	for (int i = 0; i < polygon.size(); i++) {
		polygon[i].x = polygon[i].x / (float)window->width * 2 - 1.0;
		polygon[i].y = - polygon[i].y / (float)window->height * 2 + 1.0;
		//std::cout << polygon[i].x << "," << polygon[i].y << std::endl;
	}
	SegmentShader->bind();
	if (segmentTransform.isIdentity()) {
		segmentTransform = camera->getTransform();
	}
	SegmentShader->setUniformValue("transform", segmentTransform);
	SegmentShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	SegmentShader->setUniformValue("SelectMode", Selectmode);

	// 存储polygon坐标
	GLuint polygonBuffer;
	glCreateBuffers(1, &polygonBuffer);
	glNamedBufferData(polygonBuffer, 4 + polygon.size() * 8, nullptr, GL_STATIC_DRAW);
	int num = polygon.size();
	glNamedBufferSubData(polygonBuffer, 0, 4, &num);
	glNamedBufferSubData(polygonBuffer, 4, polygon.size() * 8, polygon.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, polygonBuffer);
	
	// 无缓冲区则创建
	GLuint selectBuffer = 0;
	glCreateBuffers(1, &selectBuffer);
	glNamedBufferData(selectBuffer, pcd->points_num * 4, nullptr, GL_STATIC_DRAW);

	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selectBuffer);

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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
	glDeleteBuffers(1, &polygonBuffer);
	SegmentShader->unbind();
	//segement = false;
	selectBufferStack.push(selectBuffer); // selectBuffer压入栈
}

// 恢复裁剪区域 —— 可以指定selectBuffer
void ProgressiveRender::resumeSegment() {
	if (selectBufferStack.isEmpty()){
		return;
	}
	GLuint selectBuffer = selectBufferStack.pop(); // 出栈

	resumeSegmentShader->bind();
	resumeSegmentShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selectBuffer);

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
	
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	resumeSegmentShader->unbind();

	glDeleteBuffers(1, &selectBuffer); // 销毁
}

// 根据Polygon区域选择点云
// polygon为屏幕坐标
vector<uint> ProgressiveRender::selectPointsByPolygon(vector<Point> polygon) {
	std::cout << "Start Select!" << std::endl;
	// polygon转换到标准化设备坐标下
	for (int i = 0; i < polygon.size(); i++) {
		polygon[i].x = polygon[i].x / (float)window->width * 2 - 1.0;
		polygon[i].y = -polygon[i].y / (float)window->height * 2 + 1.0;
		//std::cout << polygon[i].x << "," << polygon[i].y << std::endl;
	}

	selectShader->bind();
	selectShader->setUniformValue("transform", camera->getTransform());
	selectShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	int SelectMode = 0;
	selectShader->setUniformValue("SelectMode", SelectMode);

	// 存储polygon坐标
	GLuint polygonBuffer = 0;
	glCreateBuffers(1, &polygonBuffer);
	glNamedBufferData(polygonBuffer, 4 + polygon.size() * 8, nullptr, GL_STATIC_DRAW);
	int num = polygon.size();
	glNamedBufferSubData(polygonBuffer, 0, 4, &num);
	glNamedBufferSubData(polygonBuffer, 4, polygon.size() * 8, polygon.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, polygonBuffer);

	// 创建点云区域选择点索引缓冲区
	GLuint selectBuffer = 0;
	glCreateBuffers(1, &selectBuffer);
	glNamedBufferData(selectBuffer, pcd->points_num * 4, nullptr, GL_DYNAMIC_DRAW);
	int counter = 0;
	glNamedBufferSubData(selectBuffer, 0, 4, &counter); // counter初始化为0

	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selectBuffer);

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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
	selectShader->unbind();
	glDeleteBuffers(1, &polygonBuffer);

	void *data = glMapNamedBuffer(selectBuffer, GL_READ_ONLY);
	int count = *(int *)(data);
	qDebug() << "Count: " << count;

	vector<uint> selectIndexList;
	for (int i = 1; i <= count; i++) {
		selectIndexList.emplace_back(*((int *)(data)+i));
	}
	glUnmapNamedBuffer(selectBuffer);
	glDeleteBuffers(1, &selectBuffer);

	return selectIndexList;
}

// 根据Polyline区域选择点云
vector<int> ProgressiveRender::selectPointsByPolyline(vector<Point> polyline, vector<Point> &gridPolyline) {
	qDebug() << "Start Select Polyline Pts";
	// polyline转换到屏幕中心右手坐标系下
	for (int i = 0; i < polyline.size(); i++) {
		polyline[i].x = polyline[i].x - (float)window->width / 2.0;
		polyline[i].y = (float)window->height / 2.0 - polyline[i].y;
	}

	// 栅格化 polyline  AX + BY + C = 0
	for (int i = 0; i < polyline.size() - 1; i++) {
		float x1 = polyline[i].x;
		float x2 = polyline[i + 1].x;
		float y1 = polyline[i].y;
		float y2 = polyline[i + 1].y;
		float A = y2 - y1;
		float B = x1 - x2;
		float C = x2 * y1 - x1 * y2;
		if (abs(x2 - x1) >= abs(y2 - y1)) {
			for (int j = x1; x1 < x2 ? j < x2 : j > x2; x1 < x2 ? j++ : j--) {
				float x = j;
				float y = -(A*x + C) / B;
				gridPolyline.emplace_back(Point(x, y));
			}
		}
		else if (abs(x2 - x1) < abs(y2 - y1)) {
			for (int j = y1; y1 < y2 ? j < y2 : j > y2; y1 < y2 ? j++ : j--) {
				float y = j;
				float x = -(B * y + C) / A;
				gridPolyline.emplace_back(Point(x, y));
			}
		}
		
		gridPolyline.emplace_back(polyline[i + 1]);
	}

	selectShader->bind();
	selectShader->setUniformValue("transform", camera->getTransform());
	selectShader->setUniformValue("maxPointsPerBuffer", upload->maxPointsPerBuffer);
	int SelectMode = 2;
	selectShader->setUniformValue("SelectMode", SelectMode);

	// 存储栅格化后polyline屏幕像素坐标
	GLuint polylineBuffer = 0;
	glCreateBuffers(1, &polylineBuffer);
	glNamedBufferData(polylineBuffer, 12 + gridPolyline.size() * 8, nullptr, GL_STATIC_DRAW);
	int num = gridPolyline.size();
	glNamedBufferSubData(polylineBuffer, 0, 4, &(window->width));
	glNamedBufferSubData(polylineBuffer, 4, 8, &(window->height));
	glNamedBufferSubData(polylineBuffer, 8, 12, &num);
	glNamedBufferSubData(polylineBuffer, 12, gridPolyline.size() * 8, gridPolyline.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, polylineBuffer); // binding = 2

	// 创建点云区域选择点索引缓冲区
	GLuint selectBuffer = 0;
	vector<int> select;
	select.resize(num, -1);
	glCreateBuffers(1, &selectBuffer);
	glNamedBufferData(selectBuffer, 4 + num * 4, nullptr, GL_DYNAMIC_DRAW);
	int counter = 0;
	glNamedBufferSubData(selectBuffer, 0, 4, &counter); // counter初始化为0
	glNamedBufferSubData(selectBuffer, 4, num * 4, select.data()); // counter初始化为0

	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, pointcloudBuffers[i]->VBO);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selectBuffer);

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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(groups[0], groups[1], groups[2]);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	for (int i = 0; i < pointcloudBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i, 0);
	}
	selectShader->unbind();
	glDeleteBuffers(1, &polylineBuffer);

	void *data = glMapNamedBuffer(selectBuffer, GL_READ_ONLY);
	int count = *(int *)(data);
	qDebug() << "Count: " << count;

	vector<int> selectIndexList;
	for (int i = 1; i <= num; i++) {
		selectIndexList.emplace_back(*((int *)(data)+i));
	}
	glUnmapNamedBuffer(selectBuffer);
	glDeleteBuffers(1, &selectBuffer);

	return selectIndexList;
}

// 根据selectBuffer内创建点云
std::shared_ptr<PointCloud> ProgressiveRender::createPcdFromBuffer() {
	if (selectBufferStack.isEmpty()) {
		return nullptr;
	}
	GLuint selectBuffer = selectBufferStack.pop(); // 出栈(会在外部条件限制只允许框选一次)

	std::shared_ptr<PointCloud> pcd_(new PointCloud());
	// 获取selectBuffer指针
	void *data = glMapNamedBuffer(selectBuffer, GL_READ_ONLY);
	vector<uint> indexlist; // 选取的打乱后索引集合
	vector<uint> remainIndexlist; // 剩余的打乱后点索引
	auto start = Now_ms();
	for (int i = 0; i < pcd->points_num; i++) {
		int index = *((int *)(data)+i);
		if (index != -1) {
			indexlist.emplace_back(index);
		}
		else {
			remainIndexlist.emplace_back(i);;
		}
	}
	glUnmapNamedBuffer(selectBuffer);

	pcd_->points_num = indexlist.size();
	// Attribute
	pcd_->position.resize(3, pcd_->points_num);
	pcd_->colors.resize(4, pcd_->points_num);
	pcd_->labels.resize(1, pcd_->points_num);
	pcd_->intensity.resize(1, pcd_->points_num);

	pcd->points_num = remainIndexlist.size();
	Eigen::MatrixXf position;
	Eigen::MatrixXi_8 colors;
	Eigen::MatrixXi labels;
	Eigen::MatrixXi intensity;
	position.resize(3, pcd->points_num);
	colors.resize(4, pcd->points_num);
	labels.resize(1, pcd->points_num);
	intensity.resize(1, pcd->points_num);

	auto end1 = Now_ms();
	qDebug() << "Select: " << end1 - start << "ms";

	// 并行for
	int num = pcd_->points_num < pcd->points_num ? pcd->points_num : pcd_->points_num;
	omp_set_num_threads(8);
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < num; i++) {
			// create New Pcd
			if (i < pcd_->points_num) {
				int targetIndex = indexlist[i];
				int originIndex = pcd->indexTable[targetIndex];
				pcd_->position.col(i) = pcd->position.col(originIndex);
				pcd_->colors.col(i) = pcd->colors.col(originIndex);
				pcd_->labels.col(i) = pcd->labels.col(originIndex);
				pcd_->intensity.col(i) = pcd->intensity.col(originIndex);
			}
			// Update Old Pcd
			if (i < pcd->points_num) {
				int targetIndex = remainIndexlist[i];
				int originIndex = pcd->indexTable[targetIndex];
				position.col(i) = pcd->position.col(originIndex);
				colors.col(i) = pcd->colors.col(originIndex);
				labels.col(i) = pcd->labels.col(originIndex);
				intensity.col(i) = pcd->intensity.col(originIndex);
			}
			//printf("i = %d, I am Thread %d\n", i, omp_get_thread_num());
		}
	}
	// ************************ Old Pcd ***************************
	pcd->position = position;
	pcd->colors = colors;
	pcd->labels = labels;
	pcd->intensity = intensity;
	// BoundingBox
	Eigen::Vector3f minp = pcd->position.rowwise().minCoeff();
	Eigen::Vector3f maxp = pcd->position.rowwise().maxCoeff();
	pcd->boundingBox.extend(minp);
	pcd->boundingBox.extend(maxp);
	// Max Min Idensity
	pcd->setmaxIdensity(pcd->intensity.rowwise().maxCoeff()(0, 0));
	pcd->setminIdensity(pcd->intensity.rowwise().minCoeff()(0, 0));
	// Index Table
	delete pcd->indexTable;
	pcd->indexTable = new uint32_t[pcd->points_num];

	// ************************ New Pcd ***************************
	pcd_->name = pcd->name + "-sub";
	// BoundingBox
	Eigen::Vector3f minp_ = pcd_->position.rowwise().minCoeff();
	Eigen::Vector3f maxp_ = pcd_->position.rowwise().maxCoeff();
	pcd_->boundingBox.extend(minp_);
	pcd_->boundingBox.extend(maxp_);

	// Max Min Idensity
	pcd_->setmaxIdensity(pcd_->intensity.rowwise().maxCoeff()(0, 0));
	pcd_->setminIdensity(pcd_->intensity.rowwise().minCoeff()(0, 0));
	// Index Table
	pcd_->indexTable = new uint32_t[pcd_->points_num];
	// offset
	pcd_->offset = pcd->offset;

	auto end2 = Now_ms();
	qDebug() << "creatPcd: " << end2 - end1 << "ms";

	glDeleteBuffers(1, &selectBuffer);
	selectBuffer = 0;

	init();
	currentAttributeMode = -1;

	return pcd_;
}

// 上传更新属性信息
void ProgressiveRender::uploadAttribute(int AttributeMode) {
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
	case FROM_INTENSITY: {
		auto idensity = pcd->intensity;
		upload->uploadChunkAttribute(idensity.data(), 0, idensity.cols());
		Eigen::MatrixXf colorStrip = getColorStrip();
		pcd->setColorStrip(colorStrip.cols(), colorStrip);
	}
	break;
	default:
		break;
	}
}

std::shared_ptr<PointCloud> ProgressiveRender::getCurrentPcd() {
	return this->pcd;
}

// 更新渲染
void ProgressiveRender::update() {
	this->init();
	uploadAttribute(this->pcd->getAttributeMode());
}