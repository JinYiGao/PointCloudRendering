/*
 * @Descripttion: 多段线绘制工具
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-28 21:55:17
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-28 21:55:18
 */
#pragma once 

#include <vector>
#include <Base/tool.h>
#include <Eigen/Eigen>

using std::vector;

class RenderWidget;

class ToolDrawPolyline :public Tool {
	Q_OBJECT
public:
	ToolDrawPolyline();
	~ToolDrawPolyline();

protected:
	vector<Point> polyline; // 多段线数据
	bool isActivate = false;  // 是否启用工具

	bool visible = true; // 多段线可见性
	bool drawing = false; // 是否处于绘制状态
	bool is_translate = false; // 是否处于平移状态

	int type = DrawPolylineTool;
	int width;
	int height;

	Eigen::Vector2i startPos;
	Eigen::Vector2i endPos;

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

	void setVisible(bool visible);
	vector<Point> getPolyline() const;

	// 键鼠操作
	void mousePress(QMouseEvent *e) override;
	void mouseRelease(QMouseEvent *e) override;
	void mouseMove(QMouseEvent *e) override;
	void mouseDoubleClick(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;
	// 键盘操作
	void keyPress(QKeyEvent *e) override;

signals:
	void endDraw(vector<Point> polyline);
	void exit();
};