/*
 * @Descripttion: 同一点云添加工具
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:42:34
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:42:35
 */
#pragma once

#include <Base/tool.h>
#include <QUndoCommand>

class PointCloud;
class RenderWidget;
class ProgressiveRender;

class ToolAddPcd :public Tool {
	Q_OBJECT
public:
	ToolAddPcd(RenderWidget *glWidget);
	~ToolAddPcd();

	void setInfo(QString pcdname);

	void activate() override; // 启用工具

protected:
	RenderWidget *glWidget = nullptr;
	ProgressiveRender *proRender = nullptr;
};

class AddPcdCommand :public QUndoCommand {
public:
	AddPcdCommand(ProgressiveRender *proRender, std::shared_ptr<PointCloud> pcd);

	void undo() override;
	void redo() override;
	int id() const;

protected:
	ProgressiveRender *proRender = nullptr;// 添加到的渲染实例
	std::shared_ptr<PointCloud> pcd = nullptr; // 待添加的点云
};