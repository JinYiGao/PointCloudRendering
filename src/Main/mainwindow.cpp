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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	// Widget
	renderWidget = ui->openGLWidget;
	dbRoot = new DBRoot(ui->treeWidget, ui->propertiesView);

	connect(dbRoot, SIGNAL(selectionChanged()), this, SLOT(enableUIItems()));

	// toolbar
	toolbtn_segment->setIcon(QIcon(":/PointCloud/images/segment_deactive.png"));
	toolbtn_segment->setEnabled(false);
	
	ui->toolBar->addWidget(toolbtn_segment);

	// 相关部件初始状态设置
	ui->isEDL->setChecked(true);

	// 窗体初始最大化
	setWindowState(Qt::WindowMaximized);
	ui->centralwidget->setMouseTracking(true);

	// 定义事件
	connect(ui->actionopen, SIGNAL(triggered()), this, SLOT(openfile()));
	connect(ui->isEDL, SIGNAL(stateChanged(int)), this, SLOT(isEDLChecked()));
	connect(ui->val_MSAA, SIGNAL(valueChanged(int)), this, SLOT(changeMSAA()));
	
	connect(toolbtn_segment, SIGNAL(clicked()), this, SLOT(start_segement())); // 打开裁剪
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
		PointCloud *pcd = new PointCloud();
		*pcd = read_las(path.toStdString());
		// 设置Tree
		dbRoot->add_pointcloud(pcd);
		renderWidget->addPointCloud(pcd);
		renderWidget->setMouseTracking(true);
	}
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

void MainWindow::enableUIItems() {
	vector<PointCloud*> pcdSelected = dbRoot->getSelectedPcd();
	if (pcdSelected.size() > 0) {
		toolbtn_segment->setEnabled(true);
		toolbtn_segment->setIcon(QIcon(":/PointCloud/images/segment.png"));
	}
	else {
		toolbtn_segment->setEnabled(false);
		toolbtn_segment->setIcon(QIcon(":/PointCloud/images/segment_deactive.png"));
	}
}

// 开始裁剪
void MainWindow::start_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	std::cout << "start segement" << std::endl;
	// 弹出裁剪面板
	if (!segmentationWidget) {
		segmentationWidget = new SegmentationWidget(renderWidget, renderWidget, dbRoot);
		segmentationWidget->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::CustomizeWindowHint);
		segmentationWidget->setGeometry(renderWidget->width(), 100, segmentationWidget->width(), segmentationWidget->height());
		segmentationWidget->show();
	}
	else {
		segmentationWidget->show();
	}
	renderWidget->toolManager->changeTool(DrawPolygonTool); // 更改当前使用工具
}
