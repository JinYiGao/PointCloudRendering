/*
 * @Descripttion: 点云框选删除工具
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:41:48
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:41:48
 */
#pragma once

#include <QUndoCommand>

#include <Base/tool.h>
#include <Base/tool_drawPolygon.h>

#include <Main/mainwindow.h>

class RenderWidget;
class ProgressiveRender;

class ToolDeletePcd : public Tool {
	Q_OBJECT
public:
	ToolDeletePcd(RenderWidget *glWidget);
	~ToolDeletePcd();

public:
	void setInfo(vector<QString> rendernamelist, int mode);

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
	
	void keyPress(QKeyEvent *e) override;

public:
	vector<ProgressiveRender*> renderlist; // 作用的 pcd
	int mode;
	ToolDrawPolygon *toolDrawPolygon = nullptr;

protected:
	RenderWidget *glWidget = nullptr;
	int type;
	bool activated;
};


class DeletePcdCommand :public QUndoCommand {
public:
	DeletePcdCommand(RenderWidget *glWidget, vector<ProgressiveRender*> renderList, vector<Point> polygon, int deleteMode);

	void undo() override;
	void redo() override;
	int id() const;

private:
	RenderWidget *glWidget = nullptr;
	vector<ProgressiveRender*> renderList;
	vector<Point> polygon; // 执行点云删除的区域
	int deleteMode;
	Eigen::Matrix4f transform = Eigen::Matrix4f::Identity(); // 记录点云此刻相机状态
};