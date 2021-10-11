/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:42:00
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:42:00
 */
#include <Base/tool_delete_pcd.h>

#include <PointCloud/renderingWidget.h>
#include <PointCloud/progressiveRender.h>

ToolDeletePcd:: ToolDeletePcd(RenderWidget *glWidget) {
	this->toolDrawPolygon = new ToolDrawPolygon();
	this->glWidget = glWidget;
}

ToolDeletePcd::~ToolDeletePcd() {
	delete this->toolDrawPolygon;
}

void ToolDeletePcd::setInfo(vector<QString> rendernamelist, int mode) {
	this->mode = mode;
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

void ToolDeletePcd::activate() {
	this->toolDrawPolygon->activate();
	this->activated = true;
}

void ToolDeletePcd::deactivate() {
	this->toolDrawPolygon->deactivate();
	this->reset();
	this->activated = false;
}

void ToolDeletePcd::suspend() {
	this->toolDrawPolygon->suspend();
	this->activated = false;
}

void ToolDeletePcd::resume() {
	this->toolDrawPolygon->resume();
	this->activated = true;
}

void ToolDeletePcd::reset() {
	this->toolDrawPolygon->reset();
}

int ToolDeletePcd::getToolType() {
	return type;
}

void ToolDeletePcd::draw(QPainter *painter) {
	this->toolDrawPolygon->draw(painter);
}

void ToolDeletePcd::gl_draw() {
	this->toolDrawPolygon->gl_draw();
}

void ToolDeletePcd::mousePress(QMouseEvent *e) {
	if (!activated) {
		return;
	}
	this->toolDrawPolygon->mousePress(e);
}

void ToolDeletePcd::mouseMove(QMouseEvent *e) {
	if (!activated) {
		return;
	}
	this->toolDrawPolygon->mouseMove(e);
}

void ToolDeletePcd::mouseDoubleClick(QMouseEvent *e) {
	if (!activated) {
		return;
	}
	this->toolDrawPolygon->mouseDoubleClick(e);

	// 执行删除所选点云
	QUndoStack *undoStack = MainWindow::mainWindow->getUndoStack();
	undoStack->push(new DeletePcdCommand(this->glWidget, this->renderlist, toolDrawPolygon->polygon, mode));
	
	// 结束后恢复为相机
	this->glWidget->toolManager->changeTool(CameraTool);
}

void ToolDeletePcd::keyPress(QKeyEvent *e) {
	if (!activated) {
		return;
	}
	if (e->key() == Qt::Key_Escape) {
		this->glWidget->toolManager->changeTool(CameraTool);
	}
}

DeletePcdCommand::DeletePcdCommand(RenderWidget *glWidget, vector<ProgressiveRender*> renderList, vector<Point> polygon, int deleteMode) {
	this->glWidget = glWidget;
	this->renderList = renderList;
	this->polygon = polygon;
	this->deleteMode = deleteMode;
}

int DeletePcdCommand::id() const{
	return 0;
}

void DeletePcdCommand::undo() {
	for (int i = 0; i < renderList.size(); i++) {
		renderList[i]->resumeSegment();
	}
}

void DeletePcdCommand::redo() {
	if (this->transform.isIdentity()) {
		this->transform = glWidget->camera->getTransform();
	}
	for (int i = 0; i < renderList.size(); i++) {
		renderList[i]->Segment(this->polygon, this->deleteMode, this->transform);
	}
	setText(QObject::tr("Delete PointCloud"));
}