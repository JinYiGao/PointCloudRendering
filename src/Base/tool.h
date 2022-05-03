/*
 * @Descripttion: 基本工具管理类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-17 15:24:44
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-17 15:24:57
 */
#pragma once

#include <Base/common.h>
#include <QMouseEvent>
#include <QObject>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QPainter>

enum ToolType {
	CameraTool = 0,    // 相机工具
	DrawPolygonTool,   // 多边形绘制工具
	DrawRectangleTool, // 矩形绘制工具
	DrawPolylineTool,  // 多段线绘制工具
	DeletePcdTool,     // 点云框选删除工具
	AddPcdTool,        // 点云追加添加工具
	PickPointTool,     // 点云选择(选点)工具
	EditLabelTool,     // 点云类别修改工具
	DrawProfileTool,   // 绘制剖线工具
	NoneTool = 0xFFFFFFFF // 不选择任何工具
};

struct Point {
	float x;
	float y;
	Point() {

	}
	Point(float x, float y) {
		this->x = x;
		this->y = y;
	}
};

class Tool;

// 工具管理类
class ToolManager : public QObject{
	Q_OBJECT
public:
	ToolManager(QObject *parent);
	~ToolManager();

public:
	Tool *get_tool(int id) const; // 获取对应id的工具
	Tool *get_current() const; // 获取当前工具
	int getToolType() const; // 获取工具类型
	void suspend_current(); // 暂停当前工具使用
	void resume_current(); // 恢复当前工具使用
	void reset_current(); // 重置当前使用工具
	bool isSuspended() const; // 当前工具是否暂停使用

	void register_tool(int id, Tool *tool); // 注册工具
	void removeAll(); // 删除所有工具
	void changeTool(int id); // 更换当前工具

public:
	void draw(QPainter *painter); // 绘制
	void gl_draw(); // 以OpenGL方式绘制

	void mousePress(QMouseEvent *e); // 鼠标按下
	void mouseRelease(QMouseEvent *e); // 鼠标抬起
	void mouseMove(QMouseEvent *e); // 鼠标移动
	void mouseDoubleClick(QMouseEvent *e); // 鼠标双击
	void wheelEvent(QWheelEvent *e);
	void keyDown(QKeyEvent *e); // 键盘按下
	void keyUp(QKeyEvent *e); // 键盘抬起
	void keyPress(QKeyEvent *e); // 键盘完整按下事件

protected:
	std::map<int, Tool*> tools; // tools
	Tool *currentTool; // 当前tool

	bool Suspended; // 当前工具是否处于暂停状态
};

// 父类 不实现具体函数定义 由子类继承实现
class Tool : public QObject, public QOpenGLFunctions_4_5_Core {
	Q_OBJECT
public:
	Tool();
	~Tool();

public:
	virtual void activate() {}; // 启用工具
	virtual void deactivate() { reset(); }; // 退出工具
	virtual void suspend() {}; // 暂停工具
	virtual void resume() {}; // 恢复工具
	virtual int getToolType() { return -1; }; // 获取工具类型

	// 重置工具
	virtual void reset() {};

	// QPainter绘制
	virtual void draw(QPainter *painter) {};
	// OpenGL方式绘制
	virtual void gl_draw() {};

	virtual void mousePress(QMouseEvent *e) {}; // 鼠标按下
	virtual void mouseRelease(QMouseEvent *e) {}; // 鼠标抬起
	virtual void mouseMove(QMouseEvent *e) {}; // 鼠标移动
	virtual void mouseDoubleClick(QMouseEvent *e) {}; // 鼠标双击
	virtual void wheelEvent(QWheelEvent *e) {};
	virtual void keyDown(QKeyEvent *e) {}; // 键盘按下
	virtual void keyUp(QKeyEvent *e) {}; // 键盘抬起
	virtual void keyPress(QKeyEvent *e) {}; // 键盘完整按下事件
};