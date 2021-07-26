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

class ToolCamera :public Tool {
	Q_OBJECT

public:
	ToolCamera();
	ToolCamera(RenderWidget *glWidget);
	~ToolCamera();

	// 重写父类虚函数
	int getToolType() override;

	void mousePress(QMouseEvent *e) override;
	void mouseRelease(QMouseEvent *e) override;
	void mouseMove(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

protected:
	RenderWidget *glWidget = nullptr;
	Camera *camera = nullptr;

	int type = CameraTool;
};