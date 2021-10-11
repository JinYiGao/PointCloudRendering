/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:42:42
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:42:42
 */
#include <Base/tool_add_pcd.h>
#include <Main/mainwindow.h>
#include <PointCloud/pointclouds.h>
#include <PointCloud/renderingWidget.h>
#include <PointCloud/progressiveRender.h>

#include <qfiledialog.h>
#include <QMessageBox>


ToolAddPcd::ToolAddPcd(RenderWidget *glWidget) {
	this->glWidget = glWidget;
}

ToolAddPcd::~ToolAddPcd() {

}

void ToolAddPcd::setInfo(QString pcdname) {
	auto RenderList = this->glWidget->getRenderList();
	for (int i = 0; i < RenderList.size(); i++) {
		if (RenderList[i]->name == pcdname) {
			this->proRender = RenderList[i];
			return;
		}
	}
}

// 激活工具 打开文件对话框
void ToolAddPcd::activate() {
	QString path = QFileDialog::getOpenFileName(
		MainWindow::mainWindow,
		tr("open a file."),
		"D:/Study/Programming/QT/pointscloud");
	if (path.isEmpty()) {
		QMessageBox::warning(MainWindow::mainWindow, "Warning!", "Failed to open the file!\n");

		// Change Tool to CameraTool
		glWidget->toolManager->changeTool(CameraTool);
	}
	else {
		// 读取文件
		std::shared_ptr<PointCloud> pcd(new PointCloud());
		read_las(path.toStdString(), pcd);

		// AddPcdCommand
		MainWindow::mainWindow->getUndoStack()->push(new AddPcdCommand(this->proRender, pcd));

		// Change Tool to CameraTool
		glWidget->toolManager->changeTool(CameraTool);
	}
}

// --------------------------- UndoCommand-----------------------------
AddPcdCommand::AddPcdCommand(ProgressiveRender *proRender, std::shared_ptr<PointCloud> pcd) {
	this->proRender = proRender;
	this->pcd = pcd;
}

void AddPcdCommand::undo() {
	auto currentPcd = this->proRender->getCurrentPcd();
	currentPcd->removeLastPcd();
	// update
	this->proRender->update();
}

void AddPcdCommand::redo() {
	auto currentPcd = this->proRender->getCurrentPcd();
	currentPcd->addPcd(this->pcd);// 添加点云
	// update
	this->proRender->update();

	setText(QObject::tr("Add PointCloud"));
}

int AddPcdCommand::id() const {
	return 0;
}