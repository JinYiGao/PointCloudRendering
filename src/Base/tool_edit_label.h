/*
 * @Descripttion: 点云类别属性修改工具
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 17:34:15
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 17:34:16
 */
#pragma once
#include <QUndoCommand>
#include <QStack>
#include <vector>

#include <Base/tool.h>

using std::vector;

class RenderWidget;
class ProgressiveRender;

class ToolEditLabel :public Tool {
Q_OBJECT
public:
	ToolEditLabel(RenderWidget *glWidget);
	~ToolEditLabel();

	int getToolType();

	void setInfo(vector<QString> rendernamelist, int classification);
	void ApplyEditLabel();

protected:
	int tooltype = EditLabelTool;
	RenderWidget *glWidget = nullptr;

	vector<ProgressiveRender*> renderlist; // 作用的 pcd
	int classification = -1;
};

class EditLabelCommand :public QUndoCommand {
public:
	EditLabelCommand(vector<ProgressiveRender*> renderlist, RenderWidget *glWidget, int classification);
	~EditLabelCommand();

	void undo() override;
	void redo() override;
	int id() const;

private:
	struct EditInfo {
		ProgressiveRender *proRender; // 编辑的点云
		vector<uint> undoSelectBuffer;
		vector<uint> redoSelectBuffer;
		vector<uint> originlabels; // undo时的labels信息,即存储编辑前点云分类信息

		EditInfo(ProgressiveRender *proRender) {
			this->proRender = proRender;
		}
	};

	vector<EditInfo*> editInfoList; // 编辑点云信息列表
	RenderWidget *glWidget = nullptr;
	int classification = -1;
};