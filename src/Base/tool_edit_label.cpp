/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 17:34:25
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 17:34:26
 */
#include <Base/tool_edit_label.h>
#include <PointCloud/renderingWidget.h>
#include <PointCloud/progressiveRender.h>
#include <Main/mainwindow.h>
#include <omp.h>

ToolEditLabel::ToolEditLabel(RenderWidget *glWidget) {
	this->glWidget = glWidget;
}

ToolEditLabel::~ToolEditLabel() {

}

int ToolEditLabel::getToolType() {
	return this->tooltype;
}

void ToolEditLabel::setInfo(vector<QString> rendernamelist, int classification) {
	this->classification = classification;

	renderlist.clear();
	auto allRenderList = this->glWidget->getRenderList();
	for (int i = 0; i < rendernamelist.size(); i++) {
		for (int j = 0; j < allRenderList.size(); j++) {
			if (rendernamelist[i] == allRenderList[j]->name) {
				renderlist.emplace_back(allRenderList[j]);
			}
		}
	}
}

void ToolEditLabel::ApplyEditLabel() {
	// 执行编辑点云分类操作
	QUndoStack *undoStack = MainWindow::mainWindow->getUndoStack();
	undoStack->push(new EditLabelCommand(this->renderlist, this->glWidget, this->classification));
}


EditLabelCommand::EditLabelCommand(vector<ProgressiveRender*> renderlist, RenderWidget *glWidget, int classification) {
	for (int i = 0; i < renderlist.size(); i++) {
		this->editInfoList.emplace_back(new EditInfo(renderlist[i]));
	}
	this->glWidget = glWidget;
	this->classification = classification;
}

EditLabelCommand::~EditLabelCommand() {

}

int EditLabelCommand::id() const {
	return -1;
}

void EditLabelCommand::undo() {
	for (int i = 0; i < this->editInfoList.size(); i++) {
		auto editInfo = this->editInfoList[i];
		if (editInfo->undoSelectBuffer.empty()) {
			return;
		}
		vector<uint> selectBuffer = editInfo->undoSelectBuffer;

		// 根据selectBuffer编辑对应点的label
		auto pcd = editInfo->proRender->getCurrentPcd();
		omp_set_num_threads(8);
#pragma omp parallel
		{
#pragma omp for
			for (int j = 0; j < selectBuffer.size(); j++) {
				int index = pcd->indexTable[selectBuffer[j]];
				pcd->labels(0, index) = editInfo->originlabels[j];
			}
		}

		editInfo->undoSelectBuffer.clear(); // 清空
		editInfo->redoSelectBuffer = selectBuffer; 

		editInfo->proRender->uploadAttribute(FROM_Label); // update
		pcd->setAttributeMode(FROM_Label);
	}
}

void EditLabelCommand::redo() {
	for (int i = 0; i < this->editInfoList.size(); i++) {
		auto editInfo = this->editInfoList[i];
		vector<uint> selectBuffer;
		if (editInfo->redoSelectBuffer.empty()) {
			selectBuffer = editInfo->proRender->selectPointsByPolygon(glWidget->toolDrawPolygon->polygon);
			editInfo->originlabels.resize(selectBuffer.size()); 
		}
		else {
			selectBuffer = editInfo->redoSelectBuffer;
		}
		// 根据selectBuffer编辑对应点的label
		auto pcd = editInfo->proRender->getCurrentPcd();
		omp_set_num_threads(8);
#pragma omp parallel
		{
#pragma omp for
			for (int j = 0; j < selectBuffer.size(); j++) {
				int index = pcd->indexTable[selectBuffer[j]];
				editInfo->originlabels[j] = pcd->labels(0, index);
				pcd->labels(0, index) = this->classification;
			}
		}

		editInfo->undoSelectBuffer = selectBuffer;
		editInfo->redoSelectBuffer.clear(); // 清空

		editInfo->proRender->uploadAttribute(FROM_Label); // update
		pcd->setAttributeMode(FROM_Label);
	}
	setText(QObject::tr("Edit Classification"));
}