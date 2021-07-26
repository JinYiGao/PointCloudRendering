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
	// 初始化相机
	this->camera = new Camera();
	this->toolManager = nullptr;
}

RenderWidget::RenderWidget(QWidget *parent, PointCloud *pcd) : QOpenGLWidget(parent){
	//this->pcd = pcd;
	addPointCloud(pcd);
	// 初始化相机
	this->camera = new Camera();
}

// 析构函数
RenderWidget::~RenderWidget() {
	if (toolManager) {
		delete toolManager;
	}
}

// 初始化工具
void RenderWidget::initTools() {
	toolManager = new ToolManager(this);

	toolCamera = new ToolCamera(this);
	toolDrawPolygon = new ToolDrawPolygon();

	toolManager->register_tool(CameraTool, toolCamera);
	toolManager->register_tool(DrawPolygonTool, toolDrawPolygon);

	toolManager->changeTool(CameraTool);
}

// 添加点云
void RenderWidget::addPointCloud(PointCloud *pcd) {
	ProgressiveRender *progressiveRender = new ProgressiveRender(this, pcd);
	renderList.emplace_back(progressiveRender);
}

void RenderWidget::initializeGL()
{
	// 初始化
	initializeOpenGLFunctions();
	// 初始化工具
	initTools();
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
	if (toolDrawPolygon->polygon.size() > 0) {
		for (int i = 0; i < toolDrawPolygon->polygon.size(); i++) {
			toolDrawPolygon->polygon[i].x *= (w / (float)window->width);
			toolDrawPolygon->polygon[i].y *= (h / (float)window->height);
		}
	}
	window->width = w;
	window->height = h;

    //用于更新投影矩阵或者其他相关大小设置
    camera->size = {w, h};
    camera->arcball.setSize(Vector2i(w, h));

	// 更新fbo大小
	fbo->setSize(w, h);
	fboEDL->setSize(w, h); // fboEDL大小需要跟fbo一致
}

void RenderWidget::paintGL()
{
	// 无点云 返回
	if (renderList.size() == 0) {
		return;
	}
	// 开启着色器点大小获取
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	// 开启深度检测
	glEnable(GL_DEPTH_TEST);
	// 设置帧缓冲样本数 --- 采样方式
	fbo->setSamples(MSAA_SAMPLES);
	// 绑定当前渲染到帧缓冲 ------------------- 离屏渲染
	fbo->bind();
	//缓存清除
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < renderList.size(); i++) {
		renderList[i]->renderPointCloudProgressive();
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
		shader->setUniformBlockValue(uniformBlocks, "shader_data", "shader_data.edlStrength",&edlStrength);
		float samples = fbo->samples;
		shader->setUniformBlockValue(uniformBlocks, "shader_data", "shader_data.msaaSampleCount", &samples);
		// 绘制
		glBindVertexArray(quadBuffer->VAO);
		glDrawArrays(GL_TRIANGLES, 0, quadBuffer->vertexCount);
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);

		// 复制fbo帧缓冲内容到屏幕帧缓冲
		glBlitNamedFramebuffer(fboEDL->fbo, defaultFramebufferObject(),
			0, 0, fboEDL->width, fboEDL->height,
			0, 0, window->width, window->height,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
	}
	else {
		// 复制fbo帧缓冲内容到屏幕帧缓冲
		glBlitNamedFramebuffer(fbo->fbo, defaultFramebufferObject(),
			0, 0, fbo->width, fbo->height,
			0, 0, window->width, window->height,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	// toolManager绘制二维图形
	painter = new QPainter(this);
	painter->setPen(QPen(Qt::green, 1));
	toolManager->draw(painter);

	update();
}

// 相关设置
void RenderWidget::setMSAAsamples(int samples) {
	this->MSAA_SAMPLES = samples;
}

void RenderWidget::setEDL(bool enable) {
	this->EDL_ENABLE = enable;
}

// 鼠标交互控制
void RenderWidget::mousePressEvent(QMouseEvent *e) {
	toolManager->mousePress(e);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *e)
{
	toolManager->mouseRelease(e);
}

void RenderWidget::mouseMoveEvent(QMouseEvent *e)
{
	toolManager->mouseMove(e);
}

void RenderWidget::mouseDoubleClickEvent(QMouseEvent *e) {
	toolManager->mouseDoubleClick(e);
}

void RenderWidget::wheelEvent(QWheelEvent *e)
{
	toolManager->wheelEvent(e);
}

// 调用progressive函数进行裁剪
void RenderWidget::startSegment(vector<std::string> name) {
	if (toolManager->getToolType() != DrawPolygonTool) {
		return;
	}
	for (int i = 0; i < name.size(); i++) {
		for (int j = 0; j < renderList.size(); j++) {
			if (name[i] == renderList[j]->name) {
				renderList[j]->Segment(toolDrawPolygon->polygon);
			}
		}
	}
}