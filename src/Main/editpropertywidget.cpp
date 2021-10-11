#include "editpropertywidget.h"
#include "ui_editpropertywidget.h"

EditPropertyWidget::EditPropertyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditPropertyWidget)
{
    ui->setupUi(this);

	setAutoFillBackground(true);

	// comboBox
	ui->comboBox->setToolTip("Select Calssfication");
	ui->comboBox->addItem("0 Never Classified");
	ui->comboBox->addItem("1 Unassigned");
	ui->comboBox->addItem("2 Ground");
	ui->comboBox->addItem("3 Low Vegetation");
	ui->comboBox->addItem("4 Medium Vegetation");
	ui->comboBox->addItem("5 High Vegetation");
	ui->comboBox->addItem("6 Building");
	ui->comboBox->addItem("7 Noise");
	ui->comboBox->addItem("8 Model Key/Reserved");
	ui->comboBox->addItem("9 Water");
	ui->comboBox->addItem("10 Rail");
	ui->comboBox->addItem("11 Road Surface");
	ui->comboBox->addItem("12 Overlap/Reserved");
	ui->comboBox->addItem("13 Wire -- Guard");
	ui->comboBox->addItem("14 Wire -- Conductor");
	ui->comboBox->addItem("15 Transmission Tower");
	ui->comboBox->addItem("16 Wire -- Connector");
	ui->comboBox->addItem("17 Bridge Deck");
	ui->comboBox->addItem("18 High Noise");

	// SIGNAL SLOT
	connect(ui->btn_Cancel, SIGNAL(clicked()), this, SLOT(click_cancel()));
	connect(ui->btn_Apply, SIGNAL(clicked()), this, SLOT(click_apply()));
}

EditPropertyWidget::~EditPropertyWidget()
{
    delete ui;
}

void EditPropertyWidget::click_cancel() {
	this->hide();

	emit quitEdit(); // ·¢ËÍÐÅºÅ ÍË³ö±à¼­ÊôÐÔ×´Ì¬
}

void EditPropertyWidget::click_apply() {
	int classificationValue = ui->comboBox->currentIndex();

	this->hide();

	emit applyEdit(classificationValue);
}
