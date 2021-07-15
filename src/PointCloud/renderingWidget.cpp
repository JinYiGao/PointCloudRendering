/*
 * @Descripttion: 点云渲染窗体部件
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-05-29 18:36:41
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-01 14:47:12
 */

#include <renderingWidget.h>

//继承自QOpenGLFunctions可以避免每次调用opengl函数时使用前缀
RenderWidget::RenderWidget(QWidget *parent) : QOpenGLWidget(parent){
}

RenderWidget::RenderWidget(QWidget *parent, PointCloud *pcd) : QOpenGLWidget(parent){
	this->pcd = pcd;
}

void RenderWidget::init(PointCloud *pcd) {
	makeCurrent();
	this->pcd = pcd;
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
	// ------------------------------------------------------------------------------------------------
}

void RenderWidget::initializeGL()
{
	// 初始化
	initializeOpenGLFunctions();
	int data[7];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, data);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, data + 1);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, data + 2);

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, data + 3);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, data + 4);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, data + 5);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, data + 6);

	cout << "GL_MAX_COMPUTE_WORK_GROUP_COUNT:" << data[0] << " " << data[1] << " " << data[2] << endl;
	cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:" << data[3] << endl;
	cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE:" << data[4] << " " << data[5] << " " << data[6] << endl;

	// 初始化相机
	this->camera = new Camera();
	// -----------------------------离屏渲染纹理贴图顶点信息缓冲区设置---------------------------------
	// quadVertices
	float vertices[] = {
		//顶点  //纹理
		-1,-1,0,  0,0,
		1, -1,0,  1,0,
		1,1,0,    1,1,

		-1,-1,0,  0,0,
		1,1,0,    1,1,
		-1,1,0,   0,1
	};
	quadBuffer = new GLBuffer();
	vector<GLBufferAttribute> quadAttributes;
	GLBufferAttribute quadAttribute1("xyz", 0, 3, GL_FLOAT, GL_FALSE, 20, 0);
	quadAttributes.emplace_back(quadAttribute1);
	GLBufferAttribute quadAttributu2("uv", 1, 2, GL_FLOAT, GL_FALSE, 20, 12);
	quadAttributes.emplace_back(quadAttributu2);
	// 传入顶点并设置属性解析
	quadBuffer->set(&vertices, sizeof(vertices), quadAttributes, 6);
	// -------------------------------------------------------------------------------------------------

	// ------------------------------------- 离屏渲染帧缓冲 --------------------------------------------
	fbo = new FrameBuffer();
	fbo->setNumColorAttachments(2); // 一张用于输出纹理图像 一张用于记录索引数据

	fboEDL = new FrameBuffer(); // 绘制EDL处理结果的帧缓冲
	// -------------------------------------------------------------------------------------------------

	// --------------------------------------- 着色器创建 ----------------------------------------------
	reprojectShader = new Shader(":/PointCloud/shaders/reproject.vs", ":/PointCloud/shaders/reproject.fs");
	fillShader = new Shader(":/PointCloud/shaders/fill.vs", ":/PointCloud/shaders/fill.fs");
	createVBOShader = new Shader(":/PointCloud/shaders/create_vbo.cs");
	edlShader = new Shader(":/PointCloud/shaders/edl.vs", ":/PointCloud/shaders/edl.fs");
	edlShaderMSAA = new Shader(":/PointCloud/shaders/edl.vs", ":/PointCloud/shaders/edlMSAA.fs");
	SegmentShader = new Shader(":/PointCloud/shaders/select.cs");
	// -------------------------------------------------------------------------------------------------

	// ---------------------------------------- 纹理创建 -----------------------------------------------
	gradientImage = new Texture(":/PointCloud/images/gradient_spectral_2d.png");
	gradientImage->setTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gradientImage->setTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// -------------------------------------------------------------------------------------------------
}

void RenderWidget::resizeGL(int w, int h)
{
	if (polygon.size() > 0) {
		for (int i = 0; i < polygon.size(); i++) {
			polygon[i].x *= (w / (float)window.width);
			polygon[i].y *= (h / (float)window.height);
		}
	}
	window.width = w;
	window.height = h;

    //用于更新投影矩阵或者其他相关大小设置
    camera->size = {w, h};
    camera->arcball.setSize(Vector2i(w, h));

	// 更新fbo大小
	fbo->setSize(w, h);
	fboEDL->setSize(w, h); // fboEDL大小需要跟fbo一致
}

void RenderWidget::paintGL()
{
	update();
	if (pcd == nullptr) {
		return;
	}
	
	//uint64_t start = Now_us();
	// 开启着色器点大小获取
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	// 开启深度检测
	glEnable(GL_DEPTH_TEST);
	// 设置帧缓冲样本数 --- 采样方式
	fbo->setSamples(MSAA_SAMPLES);
	// 绑定当前渲染到帧缓冲 ------------------- 离屏渲染
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	//缓存清除
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (pcd != nullptr) {
		// *** 渐进渲染 *** 
		renderPointCloudProgressive();
	}

	// 裁剪
	if (segement) {
		Segement();
	}

	// 开启 EDL
	if (EDL_ENABLE) {
		// 根据fbo样本数选择采用哪个shader
		Shader *shader = fbo->samples == 1 ? edlShader : edlShaderMSAA;
		// 切换当前渲染帧缓冲为EDL的fbo
		glBindFramebuffer(GL_FRAMEBUFFER, fboEDL->fbo);
		// 切换EDL处理着色器
		shader->bind();
		GLenum TextureType = fbo->samples == 1 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
		// 纹理传入着色器
		shader->setUniformValue("uColor", 0); // 颜色纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(TextureType, fbo->textures[0]);
		shader->setUniformValue("uDepth", 1); // 深度图
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(TextureType, fbo->depth);
		
		glDisable(GL_DEPTH_TEST); // 禁止深度测试
		glDepthMask(false); // 禁止深度写入
		glDisable(GL_CULL_FACE); // 禁止面剔除

		// 写入uniform块数据
		float edlStrength = 0.4;
		shader->setUniformBlockValue(&uniformBlocks, "shader_data", "shader_data.edlStrength",&edlStrength);
		float samples = fbo->samples;
		shader->setUniformBlockValue(&uniformBlocks, "shader_data", "shader_data.msaaSampleCount", &samples);
		// 绘制
		glBindVertexArray(quadBuffer->VAO);
		glDrawArrays(GL_TRIANGLES, 0, quadBuffer->vertexCount);
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);
		
		// 复制fbo帧缓冲内容到屏幕帧缓冲
		glBlitNamedFramebuffer(fboEDL->fbo, defaultFramebufferObject(),
			0, 0, fboEDL->width, fboEDL->height,
			0, 0, window.width, window.height,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
	}
	else {
		// 复制fbo帧缓冲内容到屏幕帧缓冲
		glBlitNamedFramebuffer(fbo->fbo, defaultFramebufferObject(),
			0, 0, fbo->width, fbo->height,
			0, 0, window.width, window.height,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	/*glFinish();
	uint64_t end = Now_us();
	std::cout << "One Frame: " << (end - start) / 1000.0 << "ms" << std::endl;*/

	if (polygon.size() > 0 && enableDraw) {
		glDisable(GL_DEPTH_TEST);
		painter = new QPainter(this);
		painter->setPen(QPen(Qt::green, 1));
		QPolygon pts(polygon.size());
		for (int i = 0; i < polygon.size(); i++) {
			pts.putPoints(i, 1, polygon[i].x, polygon[i].y);
		}
		pts.putPoints(polygon.size(), 1, polygon[0].x, polygon[0].y);
		painter->drawConvexPolygon(pts);
		painter->drawText(50, 50, QString("Drawing"));
		painter->end();
		glEnable(GL_DEPTH_TEST);
	}
}

// 渐进式渲染
void RenderWidget::renderPointCloudProgressive() {
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
void RenderWidget::reproject() {
	ProgressiveRenderState *state = getRenderState();

	reprojectShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	// 颜色从纹理采样 否则直接从原始传入数据读取
	if (ATTRIBUTE_MODE == 0) {
		reprojectShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// 颜色按照色带采样 按强度着色
	if (ATTRIBUTE_MODE == 4) {
		reprojectShader->setUniformBlockValue(&uniformBlocks, "colorStrip", "colorStrip.maxValue", &maxValue);
		reprojectShader->setUniformBlockValue(&uniformBlocks, "colorStrip", "colorStrip.minValue", &minValue);
		reprojectShader->setUniformBlockValue(&uniformBlocks, "colorStrip", "colorStrip.colors", colorStrip.data());
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
void RenderWidget::fillFixed() {
	ProgressiveRenderState *state = getRenderState();

	fillShader->bind();
	Eigen::Matrix4f transform = camera->getTransform() * pcd->getModelMatrix();
	// 颜色从纹理采样 否则直接从原始传入数据读取
	if (ATTRIBUTE_MODE == 0) {
		fillShader->setUniformValue("uGradient", 0);
		gradientImage->bind(0);
	}
	// 颜色按照色带采样 按强度着色
	if (ATTRIBUTE_MODE == 4) {
		fillShader->setUniformBlockValue(&uniformBlocks, "colorStrip", "colorStrip.maxValue", &maxValue);
		fillShader->setUniformBlockValue(&uniformBlocks, "colorStrip", "colorStrip.minValue", &minValue);
		fillShader->setUniformBlockValue(&uniformBlocks, "colorStrip", "colorStrip.colors", colorStrip.data());
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
	else{
		// 点云数量极多 存在多个缓冲区的情况
		int maxBufferSize = pointcloudBuffers[0]->vertexCount; // 1340,000,000
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
void RenderWidget::createVBO() {
	ProgressiveRenderState *state = getRenderState();

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
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + i,0);
	}
}

ProgressiveRenderState* RenderWidget::getRenderState() {
	if (renderState == nullptr) {
		renderState = new ProgressiveRenderState();
	}
	return renderState;
}

void RenderWidget::startDrawPolygon() {
	this->enableDraw = true;
	this->drawingPolygon = true;
}

void RenderWidget::endDrawPolygon() {
	this->enableDraw = false;
	this->drawingPolygon = false;
}

bool first = true; //标志鼠标是否第一次按下
//鼠标交互控制
void RenderWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (enableDraw) {
		if (drawingPolygon) {
			Point point(e->localPos().x(), e->localPos().y());
			polygon.emplace_back(point);
			std::cout << e->localPos().x() << "," << e->localPos().y() << std::endl;
		}
	}
	else {
		first = true;
		Eigen::Vector2i screen(e->localPos().x(), e->localPos().y());
		camera->end_rotate(screen);    //结束旋转
		camera->end_translate(screen); //结束平移
	}
}

void RenderWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (enableDraw) {
		if (drawingPolygon) {
			Point point(e->localPos().x(), e->localPos().y());
			if (polygon.size() == 1) {
				polygon.emplace_back(point);
			}
			else if (polygon.size() > 1) {
				polygon.pop_back();
				polygon.emplace_back(point);
			}
			//std::cout << e->localPos().x() << "," << e->localPos().y() << std::endl;
		}
	}
	else
	{
		//左键按下
		if (e->buttons() == Qt::LeftButton && first)
		{
			camera->is_rotate = true;
			Vector2i screen(e->localPos().x(), e->localPos().y());
			camera->start_rotate(screen); //开始旋转
			first = false;
		}

		//中键按下
		if (e->buttons() == Qt::MiddleButton && first)
		{
			camera->is_translate = true;
			Vector2f screen(e->localPos().x(), e->localPos().y());
			camera->start_translate(screen); //开始平移
			first = false;
		}

		if (e->type() == QEvent::MouseMove && (e->buttons() == Qt::LeftButton))
		{
			Eigen::Vector2i screen(e->localPos().x(), e->localPos().y());
			camera->motion_rotate(screen);
		}

		if (e->type() == QEvent::MouseMove && (e->buttons() == Qt::MiddleButton))
		{
			Eigen::Vector2f screen(e->localPos().x(), e->localPos().y());
			camera->motion_translate(screen);
		}
		//std::cout << 4;
	}
}

void RenderWidget::mouseDoubleClickEvent(QMouseEvent *e) {
	if (drawingPolygon) {
		polygon.pop_back();
		this->drawingPolygon = false;
	}
}

void RenderWidget::wheelEvent(QWheelEvent *e)
{
	if (!enableDraw) {
		// 缩放 这种效果不好 形变大
		camera->zoom = std::max(0.0001, camera->zoom * (e->angleDelta().y() > 0 ? 1.1 : 0.9));
		camera->zoom = std::min(1000.0f, camera->zoom);
	}

    //通过改变相机视野 来缩放 效果好
   /* camera->view_angle = std::max(0.001, camera->view_angle * (e->angleDelta().y() < 0 ? 1.1 : 0.9));
    camera->view_angle = std::min(1000.0f, camera->view_angle);*/
}

void RenderWidget::polygonSegement(int mode) {
	if (this->polygon.size() == 0) {
		return;
	}
	this->segement = true;
	this->segementMode = mode;
	this->endDrawPolygon();
}

void RenderWidget::checkSegement() {
	
}

void RenderWidget::cancelSegement() {
	this->polygon.clear();
	this->endDrawPolygon();
}

void RenderWidget::Segement() {
	std::cout << "Segement!" << std::endl;
	for (int i = 0; i < polygon.size(); i++) {
		polygon[i].x = polygon[i].x / (float)window.width * 2 - 1.0;
		polygon[i].y = - polygon[i].y / (float)window.height * 2 + 1.0;
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
	segement = false;
}