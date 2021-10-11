/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-01-30 22:47:17
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-05-29 11:52:08
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qfiledialog.h>
#include <memory>

#include <Main/DBRoot.h>
#include <Main/segmentationwidget.h>
#include <Main/editpropertywidget.h>
#include <Main/profileWidget.h>
#include <PointCloud/renderingWidget.h>

MainWindow* MainWindow::mainWindow = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	// Widget
	renderWidget = new RenderWidget(this);
	auto layout = ui->centralwidget->layout();
	layout->addWidget(renderWidget);
	// dbRoot
	dbRoot = new DBRoot(ui->treeWidget, ui->propertiesView, renderWidget);

	//***************************** toolbar *********************************
	// Action
	ui->toolbtn_segment->setEnabled(false);
	ui->toolbtn_delete->setEnabled(false);
	ui->toolbtn_add->setEnabled(false);
	ui->toolbtn_pick->setEnabled(false);
	ui->toolbtn_edit->setEnabled(false);
	ui->toolbtn_drawProfile->setEnabled(false);

	// 相关部件初始状态设置
	ui->isEDL->setChecked(true);

	// 窗体初始最大化
	setWindowState(Qt::WindowMaximized);
	// 鼠标追踪
	ui->centralwidget->setMouseTracking(true);
	// ----------------------------------------------------------------------

	// ------------------------ Command History -----------------------------
	undoStack = new QUndoStack(this);
	ui->undoView->setStack(undoStack);
	undoAction = undoStack->createUndoAction(this, tr("&Undo"));
	undoAction->setShortcuts(QKeySequence::Undo);
	redoAction = undoStack->createRedoAction(this, tr("&Redo"));
	redoAction->setShortcuts(QKeySequence::Redo);
	ui->menuEdit->addAction(undoAction);
	ui->menuEdit->addAction(redoAction);
	ui->menuEdit->addSeparator();
	// ----------------------------------------------------------------------

	// 定义事件
	connect(dbRoot, SIGNAL(selectionChanged()), this, SLOT(enableUIItems()));
	connect(ui->actionopen, SIGNAL(triggered()), this, SLOT(openfile()));
	connect(ui->save, SIGNAL(triggered()), this, SLOT(savefile()));
	connect(ui->isEDL, SIGNAL(stateChanged(int)), this, SLOT(isEDLChecked()));
	connect(ui->val_MSAA, SIGNAL(valueChanged(int)), this, SLOT(changeMSAA()));
	
	connect(ui->toolbtn_segment, SIGNAL(triggered()), this, SLOT(start_segement())); // 打开裁剪 删除框选部分点云
	connect(ui->toolbtn_delete, SIGNAL(triggered()), this, SLOT(deletePcd())); // delete pointcloud 
	connect(ui->toolbtn_add, SIGNAL(triggered()), this, SLOT(addPcdToOriginal())); // add Pcd to original
	connect(ui->toolbtn_pick, SIGNAL(triggered()), this, SLOT(pickPoint())); // pick Point
	connect(ui->toolbtn_edit, SIGNAL(triggered()), this, SLOT(showEditWidget())); // show edit Property
	connect(ui->toolbtn_drawProfile, SIGNAL(triggered()), this, SLOT(start_drawProfile())); // start Draw Profile

	// Static
	mainWindow = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 打开文件
void MainWindow::openfile() {
	QString path = QFileDialog::getOpenFileName(
		this,
		tr("open a file."),
		"D:/Study/Programming/QT/pointscloud");
	if (path.isEmpty()) {
		QMessageBox::warning(this, "Warning!", "Failed to open the file!\n");
	}
	else {
		// 读取文件
		//PointCloud *pcd = new PointCloud();
		std::shared_ptr<PointCloud> pcd(new PointCloud());
		read_las(path.toStdString(), pcd);
		// 设置Tree
		dbRoot->add_pointcloud(pcd);
	}
}

// 保存文件
void MainWindow::savefile() {
	qDebug() << "Save!";
	
}

// 是否展示EDL
void MainWindow::isEDLChecked() {
	if (renderWidget == nullptr) {
		return;
	}
	bool enableEDL = ui->isEDL->isChecked();
	renderWidget->setEDL(enableEDL);
}

// 修改MSAA
void MainWindow::changeMSAA() {
	int value = ui->val_MSAA->value();
	ui->val_MSAA->setSingleStep(value);
	if (renderWidget == nullptr) {
		return;
	}
	renderWidget->setMSAAsamples(value);
}

// DBTree改变时有一些ui的变化
void MainWindow::enableUIItems() {
	vector<std::shared_ptr<PointCloud>> pcdSelected = dbRoot->getSelectedPcd();
	
	// Add PointCloud , Edit , Draw Profile
	if (pcdSelected.size() == 1) {
		ui->toolbtn_add->setEnabled(true);
		ui->toolbtn_edit->setEnabled(true);
		ui->toolbtn_drawProfile->setEnabled(true);
	}
	else {
		ui->toolbtn_add->setEnabled(false);
		ui->toolbtn_edit->setEnabled(false);
		ui->toolbtn_drawProfile->setEnabled(false);
	}
	// Segment , Delete
	if (pcdSelected.size() > 0) {
		ui->toolbtn_segment->setEnabled(true);
		ui->toolbtn_delete->setEnabled(true);
		ui->toolbtn_pick->setEnabled(true);
	}
	else {
		ui->toolbtn_segment->setEnabled(false);
		ui->toolbtn_delete->setEnabled(false);
		ui->toolbtn_pick->setEnabled(false);
	}
}

// 开始裁剪
void MainWindow::start_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	std::cout << "start segement" << std::endl;
	// 弹出裁剪面板
	/*if (!segmentationWidget) {
		segmentationWidget = new SegmentationWidget(renderWidget, renderWidget, dbRoot);
		segmentationWidget->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::CustomizeWindowHint);
		segmentationWidget->setGeometry(renderWidget->width(), 100, segmentationWidget->width(), segmentationWidget->height());
		segmentationWidget->show();
	}
	else {
		segmentationWidget->show();
	}*/
	//renderWidget->toolManager->changeTool(DrawPolygonTool); // 更改当前使用工具

	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() == 0) {
		return;
	}
	
	renderWidget->toolDeletePcd->setInfo(namelist, 1);
	renderWidget->toolManager->changeTool(DeletePcdTool);
}

// delete PointCloud
void MainWindow::deletePcd() {
	dbRoot->removeSelectedPcd();
}

 // 在现有点云之上添加点云
void MainWindow::addPcdToOriginal() {
	if (renderWidget == nullptr) {
		return;
	}
	qDebug() << "Start Add" ;
	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() != 1) {
		return;
	}
	renderWidget->toolAddPcd->setInfo(namelist[0]);
	renderWidget->toolManager->changeTool(AddPcdTool);
}

// Pick 选点
void MainWindow::pickPoint() {
	if (renderWidget == nullptr) {
		return;
	}
	qDebug() << "Start Pick";
	renderWidget->toolManager->changeTool(PickPointTool);
}

// Edit Widget Show
void MainWindow::showEditWidget() {
	if (this->editPropertyWidget == nullptr) {
		this->editPropertyWidget = new EditPropertyWidget(this->renderWidget);

		connect(this->editPropertyWidget, SIGNAL(applyEdit(int)), this, SLOT(applyEditProperty(int)));
		connect(this->editPropertyWidget, SIGNAL(quitEdit()), this, SLOT(quitEdit()));
	}
	this->editPropertyWidget->move(QPoint(4, 4));
	this->editPropertyWidget->show();

	// Draw Polygon
	renderWidget->toolManager->changeTool(DrawPolygonTool);
}

// Apply Edit
void MainWindow::applyEditProperty(int classification) {
	if (renderWidget == nullptr) {
		return;
	}
	qDebug() << classification;
	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() != 1) {
		return;
	}
	renderWidget->toolEditLabel->setInfo(namelist, classification);
	renderWidget->toolEditLabel->ApplyEditLabel();
	// 换回opengl相机
	renderWidget->toolManager->changeTool(CameraTool);
}

// Quit Edit
void MainWindow::quitEdit() {
	qDebug() << "Quit Edit!";
	renderWidget->toolManager->changeTool(CameraTool);
}

// 开始剖线绘制
void MainWindow::start_drawProfile() {
	if (renderWidget == nullptr) {
		return;
	}
	//renderWidget->toolManager->changeTool(DrawPolylineTool);
	if (this->profileWidget == nullptr) {
		this->profileWidget = new ProfileWidget(this);
		this->profileWidget->move(QPoint(200, 200));

		this->profileWidget->setGLWindow(this->renderWidget);		
	}

	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() != 1) {
		return;
	}
	this->profileWidget->setEffectPcd(namelist[0]); // 设置作用的 pcd

	this->profileWidget->show();
}

QUndoStack* MainWindow::getUndoStack() {
	return this->undoStack;
}
