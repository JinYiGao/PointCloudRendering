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
	
	// toolbar
	toolbtn_segement->setIcon(QIcon(":/PointCloud/images/segement.png"));
	toolbtn_inPolygon->setIcon(QIcon(":/PointCloud/images/polygon_in.png"));
	toolbtn_outPolygon->setIcon(QIcon(":/PointCloud/images/polygon_out.png"));
	toolbtn_check->setIcon(QIcon(":/PointCloud/images/check_deactive.png"));
	toolbtn_check->setEnabled(false);
	toolbtn_cancel->setIcon(QIcon(":/PointCloud/images/cancel.png"));
	ui->toolBar->addWidget(toolbtn_segement);
	ui->toolBar->addWidget(toolbtn_inPolygon);
	ui->toolBar->addWidget(toolbtn_outPolygon);
	ui->toolBar->addWidget(toolbtn_check);
	ui->toolBar->addWidget(toolbtn_cancel);

	// checkbox互斥
	checkBoxGroup = new QButtonGroup(this);
	checkBoxGroup->addButton(ui->showColor);
	checkBoxGroup->addButton(ui->showFromIdensity);
	checkBoxGroup->addButton(ui->showFromLabel);
	checkBoxGroup->addButton(ui->showFromRGB);

	// 相关部件初始状态设置
	ui->isEDL->setChecked(true);
	ui->pointSizeSlider->setMaximum(100);
	ui->pointSizeSlider->setMinimum(1);
	ui->pointSizeSlider->setSingleStep(1);
	ui->pointSizeSlider->setValue(15);

	// 窗体初始最大化
	setWindowState(Qt::WindowMaximized);
	ui->centralwidget->setMouseTracking(true);

	// 定义事件
	connect(ui->actionopen, SIGNAL(triggered()), this, SLOT(openfile()));
	connect(ui->isEDL, SIGNAL(stateChanged(int)), this, SLOT(isEDLChecked()));
	connect(ui->showColor, SIGNAL(stateChanged(int)), this, SLOT(notShowColor()));
	connect(ui->showFromLabel, SIGNAL(stateChanged(int)), this, SLOT(showColorFromLabel()));
	connect(ui->showFromIdensity, SIGNAL(stateChanged(int)), this, SLOT(showColorFromIdensity()));
	connect(ui->showFromRGB, SIGNAL(stateChanged(int)), this, SLOT(showColorFromRGB()));
	connect(ui->val_MSAA, SIGNAL(valueChanged(int)), this, SLOT(changeMSAA()));
	connect(ui->pointSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(changePointSize(int)));
	
	connect(toolbtn_segement, SIGNAL(clicked()), this, SLOT(start_segement()));
	connect(toolbtn_inPolygon, SIGNAL(clicked()), this, SLOT(inPolygon_segement()));
	connect(toolbtn_outPolygon, SIGNAL(clicked()), this, SLOT(outPolygon_segement()));
	connect(toolbtn_check, SIGNAL(clicked()), this, SLOT(check_segement()));
	connect(toolbtn_cancel, SIGNAL(clicked()), this, SLOT(cancel_segement()));
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
		std::cout << path.toStdString() << std::endl;
		// 读取文件
		*pcd = read_las(path.toStdString());
		pcd->name = path.toStdString();
		// 读取完毕 渲染窗口
		renderWidget = ui->openGLWidget;
		renderWidget->init(pcd);
		renderWidget->setMouseTracking(true); // 自动鼠标追踪

		// 设置Tree
		QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
		item->setText(0, QString::fromStdString(pcd->name));
		item->setCheckState(0, Qt::Checked);
		ui->treeWidget->addTopLevelItem(item);
	}
}

// 是否展示EDL
void MainWindow::isEDLChecked() {
	if (renderWidget == nullptr) {
		return;
	}
	bool isEDL = ui->isEDL->isChecked();
	renderWidget->EDL_ENABLE = isEDL;
}

// 是否不展示颜色
void MainWindow::notShowColor() {
	if (renderWidget == nullptr) {
		return;
	}
	bool notShowColor = ui->showColor->isChecked();
	if (notShowColor) {
		renderWidget->ATTRIBUTE_MODE = 2;
	}
}

// 根据分类着色
void MainWindow::showColorFromLabel() {
	if (renderWidget == nullptr) {
		return;
	}
	bool isShowColorFromLabel = ui->showFromLabel->isChecked();
	auto labels = pcd->labels;
	if (isShowColorFromLabel) {
		if (renderWidget->ATTRIBUTE_MODE != 3) {
			renderWidget->upload->uploadChunkAttribute(labels.data(), 0, labels.cols());
			renderWidget->ATTRIBUTE_MODE = 3;
		}
	}
}

// 根据强度着色
void MainWindow::showColorFromIdensity() {
	if (renderWidget == nullptr) {
		return;
	}
	bool isShowColorFromLabel = ui->showFromIdensity->isChecked();
	auto idensity = pcd->intensity;
	if (isShowColorFromLabel) {
		if (renderWidget->ATTRIBUTE_MODE != 4) {
			renderWidget->upload->uploadChunkAttribute(idensity.data(), 0, idensity.cols());
			renderWidget->ATTRIBUTE_MODE = 4;

			renderWidget->maxValue = pcd->intensity.rowwise().maxCoeff()(0,0);
			renderWidget->minValue = pcd->intensity.rowwise().minCoeff()(0,0);
			Eigen::MatrixXf colorStrip = getColorStrip();
			renderWidget->colorStrip = colorStrip;
			//std::cout << renderWidget->maxValue << "," << renderWidget->minValue << colorStrip << std::endl;
		}
	}
}

// 按RGB着色
void MainWindow::showColorFromRGB() {
	if (renderWidget == nullptr) {
		return;
	}
	bool isShowColorFromRGB = ui->showFromRGB->isChecked();
	auto color = pcd->colors;
	if (isShowColorFromRGB) {
		if (renderWidget->ATTRIBUTE_MODE != 1) {
			Eigen::MatrixXi int_color;
			int_color.resize(1, color.cols());
			memcpy((void*)int_color.data(), (void*)color.data(), color.cols() * 4);
			renderWidget->upload->uploadChunkAttribute(int_color.data(), 0, int_color.cols());
			renderWidget->ATTRIBUTE_MODE = 1;
		}
	}
}

// 修改MSAA
void MainWindow::changeMSAA() {
	int value = ui->val_MSAA->value();
	ui->val_MSAA->setSingleStep(value);
	if (renderWidget == nullptr) {
		return;
	}
	renderWidget->MSAA_SAMPLES = value;
}

// 修改渲染点大小
void MainWindow::changePointSize(int value){
	std::cout << value << std::endl;
	if (renderWidget == nullptr) {
		return;
	}
	renderWidget->pointSize = value / 10.0;
}

// 生成色带
Eigen::MatrixXf MainWindow::getColorStrip() {
	Eigen::MatrixXf colors;
	float colorBarLength = 343.0;//设置颜色条的长度
	colors.resize(4, colorBarLength);
	//------设置为jet颜色条---------//
	float tempLength = colorBarLength / 4;
	for (int i = 0; i < tempLength / 2; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f(0, 0, (tempLength / 2 + i) / tempLength, 1);
	}
	for (int i = tempLength / 2 + 1; i < tempLength / 2 + tempLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f(0, (i - tempLength / 2) / tempLength, 1, 1);
	}
	for (int i = tempLength / 2 + tempLength + 1; i < tempLength / 2 + 2 * tempLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f((i - tempLength - tempLength / 2) / tempLength, 1, (tempLength * 2 + tempLength / 2 - i) / tempLength, 1);
	}
	for (int i = tempLength / 2 + 2 * tempLength + 1; i < tempLength / 2 + 3 * tempLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f(1, (tempLength * 3 + tempLength / 2 - i) / tempLength, 0, 1);
	}
	for (int i = tempLength / 2 + 3 * tempLength + 1; i < colorBarLength; i++)// jet
	{
		colors.col(i) = Eigen::Vector4f((colorBarLength - i + tempLength / 2) / tempLength, 0, 0, 1);
	}

	return colors;
}

// 开始裁剪
void MainWindow::start_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	std::cout << "start segement" << std::endl;
	toolbtn_check->setIcon(QIcon(":/PointCloud/images/check_active.png"));
	toolbtn_check->setEnabled(true);
	renderWidget->startDrawPolygon();
}

// 取消裁剪
void MainWindow::cancel_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	std::cout << "cencel segement" << std::endl;
	renderWidget->cancelSegement();
}

// 截取多边形内部点云
void MainWindow::inPolygon_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	renderWidget->polygonSegement(0);
}

// 截取多边形外部点云
void MainWindow::outPolygon_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	renderWidget->polygonSegement(1);
}

// 确定裁剪
void MainWindow::check_segement() {
	if (renderWidget == nullptr) {
		return;
	}
	std::cout << "check segement!" << std::endl;
	toolbtn_check->setIcon(QIcon(":/PointCloud/images/check_deactive.png"));
	toolbtn_check->setEnabled(false);
	renderWidget->checkSegement();
}