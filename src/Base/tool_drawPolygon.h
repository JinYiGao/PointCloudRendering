/*
 * @Descripttion: 多边形绘制工具类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-18 20:46:22
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-18 20:47:59
 */
#pragma once 

#include <vector>
#include <Base/tool.h>

using std::vector;

class RenderWidget;

class ToolDrawPolygon :public Tool {
	Q_OBJECT
public:
	ToolDrawPolygon();
	~ToolDrawPolygon();

public:
	vector<Point> polygon;
	bool isActivate = false;  // 是否启用工具

protected:
	bool drawing = false; // 是否处于绘制状态

	int type = DrawPolygonTool;

public:
	// 重写父类虚函数
	void activate() override; // 启用工具
	void deactivate() override; // 退出工具
	void suspend() override; // 暂停工具
	void resume() override; // 恢复工具

	int getToolType() override;

	void reset() override; // 重置

	void draw(QPainter *painter) override;
	void gl_draw() override;

	// 键鼠操作
	void mousePress(QMouseEvent *e) override;
	void mouseMove(QMouseEvent *e) override;
	void mouseDoubleClick(QMouseEvent *e) override;
	// 键盘操作
	void keyPress(QKeyEvent *e) override;
};