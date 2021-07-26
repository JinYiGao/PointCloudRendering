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

// ȡ���ü�
void SegmentationWidget::cancel_segment() {
	if (glWidget == nullptr) {
		return;
	}
	std::cout << "cencel segement" << std::endl;
	glWidget->toolManager->changeTool(CameraTool); // ���ĵ�ǰʹ�ù���
	this->hide();
	// �ָ��ü��ĵ���
}

// ����������ڲ�����
void SegmentationWidget::inPolygon_segment() {
	qDebug() << "InPolygon";
	if (glWidget == nullptr) {
		return;
	}
	vector<PointCloud*> pcdSelected = dbRoot->getSelectedPcd();
	if (pcdSelected.size() == 0) {
		return;
	}
	vector<std::string> namelist;
	for (int i = 0; i < pcdSelected.size(); i++) {
		namelist.emplace_back(pcdSelected[i]->name);
	}
	glWidget->startSegment(namelist);
}

// ����������ⲿ����
void SegmentationWidget::outPolygon_segment() {
	if (glWidget == nullptr) {
		return;
	}
}

// ȷ���ü� ����õ��ĵ�����dbtree
void SegmentationWidget::check_segment() {
	if (glWidget == nullptr) {
		return;
	}
	std::cout << "check segement!" << std::endl;
}