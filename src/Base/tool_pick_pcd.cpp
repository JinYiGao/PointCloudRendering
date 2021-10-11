/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:44:33
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:44:33
 */
#include <Base/tool_pick_pcd.h>
#include <PointCloud/renderingWidget.h>
#include <PointCloud/progressiveRender.h>
#include <Base/shader.h>
#include <Base/glbuffer.h>

static const char *vertShader = "#version 450 core \n"
"layout(location = 0) in vec3 aPos; \n"
"uniform mat4 transform; \n"
"void main() \n"
"{ \n"
"	gl_Position = transform * vec4(aPos, 1.0); \n"
"	gl_PointSize = 6.0; \n"
"}\n";

static const char *fragShader = "#version 450 core \n"
"out vec4 fragColor; \n"
"void main() \n"
"{ \n"
"	fragColor = vec4(0.0, 1.0, 0.0, 1.0); \n"
"} \n";

ToolPick::ToolPick(RenderWidget *glWidget) {
	initializeOpenGLFunctions();

	this->glWidget = glWidget;

	this->drawPointShader = new Shader();
	this->drawPointShader->compileShaderFromSourceCode(vertShader, fragShader);

	this->glbuffer = new GLBuffer();
	glBufferAttributes.emplace_back(GLBufferAttribute("position", 0, 3, GL_FLOAT, GL_FALSE, 3, 0));
}

ToolPick::~ToolPick() {
	delete drawPointShader;
}

int ToolPick::getToolType() {
	return this->toolType;
}

void ToolPick::activate() {
	this->activated = true;
}

void ToolPick::deactivate() {
	this->reset();
}

void ToolPick::suspend() {
	this->activated = false;
}

void ToolPick::resume() {
	this->activated = true;
}

void ToolPick::reset() {
	this->activated = false;
	this->point = Eigen::Vector3f::Zero();
}

void ToolPick::draw(QPainter *painter) {
	int width = this->glWidget->window->width;
	int height = this->glWidget->window->height;
	painter->setPen(QPen(Qt::white, 1));
	// 状态显示
	painter->drawText(QRectF(QPointF(width / 2.0 - 100, 10), QPointF(width / 2.0 + 100, 30)), "Picking ... Press [ESC] Exit");
	// 点选坐标显示
	auto offset = this->glWidget->getRenderList()[0]->getCurrentPcd()->offset;
	auto point = this->point + offset;
	painter->drawText(QRectF(QPointF(width - 300, height - 30), QPointF(width -10, height - 8)), "Pick Point X:" + QString::number(point.x()) + " Y:" + QString::number(point.y()) + " Z:" + QString::number(point.z()));
}

void ToolPick::gl_draw() {
	if (!activated) {
		return;
	}
	// 取消深度测试 点选的点永远在前面
	glDisable(GL_DEPTH_TEST);
	this->glbuffer->set(Eigen::MatrixXf(point), 3 * GL_FLOAT, this->glBufferAttributes, 1);
	this->drawPointShader->bind();
	this->drawPointShader->setUniformValue("transform", this->glWidget->camera->getTransform());
	this->glbuffer->bind();
	glDrawArrays(GL_POINTS, 0, this->glbuffer->vertexCount);
	glEnable(GL_DEPTH_TEST);
}

// 鼠标事件
static int clickX = -1, clickY = -1;
void ToolPick::mousePress(QMouseEvent *e) {
	if (!activated) {
		return;
	}
	// 左键按下
	if (e->buttons() == Qt::LeftButton) {
		clickX = e->localPos().x();
		clickY = e->localPos().y();
	}
	else {
		clickX = -1;
		clickY = -1;
	}

	this->glWidget->toolCamera->mousePress(e);
}

void ToolPick::mouseRelease(QMouseEvent *e) {
	this->glWidget->toolCamera->mouseRelease(e);
	
	if (clickX == e->localPos().x() && clickY == e->localPos().y()) {
		FrameBuffer *fbo = this->glWidget->fbo;
		auto x = e->localPos().x();
		auto y = fbo->height - e->localPos().y();

		GLubyte *indexData = new GLubyte[fbo->width * fbo->height * 4];
		glGetTextureImage(fbo->textures[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, fbo->height * fbo->width * 4, indexData);
		int i = 4 * fbo->width * y + 4 * x; // image index
		uint index = indexData[i] | (indexData[i + 1] << 8) | (indexData[i + 2] << 16) | (indexData[i + 3] << 24); // 打乱后的 index

		auto positions = glWidget->getRenderList()[0]->getCurrentPcd()->position; // Get Pick Point
		auto indexTable = glWidget->getRenderList()[0]->getCurrentPcd()->indexTable;
		if (index < positions.cols() && index > 0) {
			this->point = positions.col(indexTable[index]);
			//std::cout << index << std::endl;
			//std::cout << this->point << std::endl;
		}

		delete indexData;
	}
}

void ToolPick::mouseMove(QMouseEvent *e) {
	this->glWidget->toolCamera->mouseMove(e);
}

void ToolPick::wheelEvent(QWheelEvent *e) {
	this->glWidget->toolCamera->wheelEvent(e);
}

// 键盘事件
void ToolPick::keyPress(QKeyEvent *e) {
	if (!activated) {
		return;
	}
	if (e->key() == Qt::Key_Escape) {
		this->glWidget->toolManager->changeTool(CameraTool);
	}
}