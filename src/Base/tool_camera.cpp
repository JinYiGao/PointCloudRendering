/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-18 20:45:28
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-18 20:45:28
 */
#include <tool_camera.h>
#include <PointCloud/renderingWidget.h>

ToolCamera::ToolCamera(RenderWidget *glWidget) {
	this->glWidget = glWidget;
	this->camera = glWidget->camera;
}

ToolCamera::ToolCamera() {

}

ToolCamera::~ToolCamera() {

}

void ToolCamera::setCameraType(int cameratype) {
	this->cameraType = cameratype;
	//this->camera->position = Eigen::Vector3f(0, 0, 1.0);
	this->camera->arcball.setState(Quaternionf::Identity());
}

void ToolCamera::setProjection(int projMethod) {
	if (projMethod == Perspective) {
		this->camera->is_ortho = false;
	}
	else if (projMethod == Ortho) {
		this->camera->is_ortho = true;
	}
}

void ToolCamera::suspend() {
	this->isSuspend = true;
}

void ToolCamera::resume() {
	this->isSuspend = false;
}

int ToolCamera::getToolType() {
	return this->toolType;
}

void ToolCamera::mousePress(QMouseEvent *e){
	if (isSuspend) {
		return;
	}
	//左键按下
	if (e->buttons() == Qt::LeftButton && this->cameraType != Camera2D)
	{
		camera->is_rotate = true;
		Vector2i screen(e->localPos().x(), e->localPos().y());
		camera->start_rotate(screen); //开始旋转
	}
	//中键按下
	if (e->buttons() == Qt::MiddleButton)
	{
		camera->is_translate = true;
		Vector2f screen(e->localPos().x(), e->localPos().y());
		camera->start_translate(screen); //开始平移
	}
	glWidget->update();
}

void ToolCamera::mouseMove(QMouseEvent *e) {
	if (isSuspend) {
		return;
	}
	if (!camera->is_rotate && !camera->is_translate) {
		return;
	}
	// 左键按下移动
	if (e->type() == QEvent::MouseMove && (e->buttons() == Qt::LeftButton))
	{
		Eigen::Vector2i screen(e->localPos().x(), e->localPos().y());
		camera->motion_rotate(screen);
	}
	// 中键按下移动
	if (e->type() == QEvent::MouseMove && (e->buttons() == Qt::MiddleButton))
	{
		Eigen::Vector2f screen(e->localPos().x(), e->localPos().y());
		camera->motion_translate(screen);
	}

	glWidget->update();
}

void ToolCamera::mouseRelease(QMouseEvent *e) {
	if (isSuspend) {
		return;
	}
	Eigen::Vector2i screen(e->localPos().x(), e->localPos().y());
	camera->end_rotate(screen);    //结束旋转
	camera->end_translate(screen); //结束平移

	glWidget->update();
}

void ToolCamera::wheelEvent(QWheelEvent *e) {
	if (isSuspend) {
		return;
	}
	// 缩放
	camera->zoom = camera->zoom * (e->angleDelta().y() > 0 ? 1.1 : 0.9);
	//通过改变相机视野 来缩放 效果好
	//camera->view_angle = std::max(0.001, camera->view_angle * (e->angleDelta().y() < 0 ? 1.1 : 0.9));
	//camera->view_angle = std::min(1000.0f, camera->view_angle);
}