#include "segmentationWidget.h"
#include "ui_segmentationWidget.h"

SegmentationWidget::SegmentationWidget(QWidget *parent, RenderWidget *glWidget, DBRoot *dbRoot) :
    QDialog(parent),
    ui(new Ui::SegmentationWidget)
{
    ui->setupUi(this);

	this->glWidget = glWidget;
	this->dbRoot = dbRoot;

	// slots
	connect(ui->cancel, SIGNAL(clicked()), this, SLOT(cancel_segment()));
	connect(ui->inPolygon, SIGNAL(clicked()), this, SLOT(inPolygon_segment()));
	connect(ui->outPolygon, SIGNAL(clicked()), this, SLOT(outPolygon_segment()));
	connect(ui->check, SIGNAL(clicked()), this, SLOT(check_segment()));
}

SegmentationWidget::~SegmentationWidget()
{
    delete ui;
}

// 取消裁剪
void SegmentationWidget::cancel_segment() {
	if (glWidget == nullptr) {
		return;
	}
	std::cout << "cencel segement" << std::endl;
	glWidget->toolManager->changeTool(CameraTool); // 更改当前使用工具
	// 恢复裁剪的点云
	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() == 0) {
		return;
	}
	glWidget->resumeSegment(namelist);
	this->hide();
}

// 保留多边形内部点云
void SegmentationWidget::inPolygon_segment() {
	qDebug() << "InPolygon";
	if (glWidget == nullptr) {
		return;
	}
	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() == 0) {
		return;
	}
	glWidget->startSegment(namelist, 0);
	glWidget->toolManager->changeTool(CameraTool); // 更改当前使用工具
}

// 保留多边形外部点云
void SegmentationWidget::outPolygon_segment() {
	qDebug() << "OutPolygon";
	if (glWidget == nullptr) {
		return;
	}
	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() == 0) {
		return;
	}
	glWidget->startSegment(namelist, 1);
	glWidget->toolManager->changeTool(CameraTool); // 更改当前使用工具
}

// 确定裁剪 保存得到的点云至dbtree
void SegmentationWidget::check_segment() {
	if (glWidget == nullptr) {
		return;
	}
	std::cout << "check segement!" << std::endl;
	auto start = Now_ms();
	vector<QString> namelist = dbRoot->getSelectedPcdName();
	if (namelist.size() == 0) {
		return;
	}
	std::shared_ptr<PointCloud> pcd_ = glWidget->createPcd(namelist);
	// dbRoot add
	dbRoot->add_pointcloud(pcd_);
	this->hide();
	auto end = Now_ms();
	qDebug() << "Segment Used Time: " << end - start << "ms";
}