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

int ToolCamera::getToolType() {
	return type;
}

void ToolCamera::mousePress(QMouseEvent *e){
	//�������
	if (e->buttons() == Qt::LeftButton)
	{
		camera->is_rotate = true;
		Vector2i screen(e->localPos().x(), e->localPos().y());
		camera->start_rotate(screen); //��ʼ��ת
	}
	//�м�����
	if (e->buttons() == Qt::MiddleButton)
	{
		camera->is_translate = true;
		Vector2f screen(e->localPos().x(), e->localPos().y());
		camera->start_translate(screen); //��ʼƽ��
	}
	glWidget->update();
}

void ToolCamera::mouseMove(QMouseEvent *e) {
	if (!camera->is_rotate && !camera->is_translate) {
		return;
	}
	// ��������ƶ�
	if (e->type() == QEvent::MouseMove && (e->buttons() == Qt::LeftButton))
	{
		Eigen::Vector2i screen(e->localPos().x(), e->localPos().y());
		camera->motion_rotate(screen);
	}
	// �м������ƶ�
	if (e->type() == QEvent::MouseMove && (e->buttons() == Qt::MiddleButton))
	{
		Eigen::Vector2f screen(e->localPos().x(), e->localPos().y());
		camera->motion_translate(screen);
	}

	glWidget->update();
}

void ToolCamera::mouseRelease(QMouseEvent *e) {
	Eigen::Vector2i screen(e->localPos().x(), e->localPos().y());
	camera->end_rotate(screen);    //������ת
	camera->end_translate(screen); //����ƽ��

	glWidget->update();
}

void ToolCamera::wheelEvent(QWheelEvent *e) {
	// ����
	camera->zoom = std::max(0.0001, camera->zoom * (e->angleDelta().y() > 0 ? 1.1 : 0.9));
	camera->zoom = std::min(1000.0f, camera->zoom);
}