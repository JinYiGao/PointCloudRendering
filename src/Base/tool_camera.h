/*
 * @Descripttion: 相机工具类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-18 20:45:22
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-18 20:45:36
 */
#pragma once

#include <Base/tool.h>
#include <Base/camera.h>

 // 互相包含 提前声明 在cpp内引用头文件
class RenderWidget;

enum CameraType {
	Camera2D = 0,
	Camera3D
};

enum CameraProjection {
	Ortho = 0,
	Perspective
};

class ToolCamera :public Tool {
	Q_OBJECT

public:
	ToolCamera();
	ToolCamera(RenderWidget *glWidget);
	~ToolCamera();

	void setCameraType(int cameratype); // 设置相机类型
	void setProjection(int projMethod); // 设置相机投影方式 正射或者透视投影 

	// 重写父类虚函数
	void suspend() override;
	void resume() override;

	int getToolType() override;

	void mousePress(QMouseEvent *e) override;
	void mouseRelease(QMouseEvent *e) override;
	void mouseMove(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

protected:
	RenderWidget *glWidget = nullptr;
	Camera *camera = nullptr;

	int toolType = CameraTool;
	int cameraType = Camera3D;
	bool isSuspend = false;
};