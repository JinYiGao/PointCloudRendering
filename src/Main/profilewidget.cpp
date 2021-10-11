#include "profileWidget.h"
#include "ui_profilewidget.h"

#include <PointCloud/renderingWidget.h>

ProfileWidget::ProfileWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileWidget)
{
    ui->setupUi(this);

	setAutoFillBackground(true);

	Qt::WindowFlags flags = this->windowFlags();
	setWindowFlags(flags&~Qt::WindowContextHelpButtonHint);

	setFixedSize(QSize(550, 260));

	// empty chart settings
	this->emptyChart->setTitle("profile #-1");
	this->emptyChart->createDefaultAxes();
	this->emptyChart->setBackgroundBrush(QBrush(QColor(102, 102, 102, 42)));
	this->emptyChart->setAutoFillBackground(true);
	ui->chartView->setChart(this->emptyChart);
	ui->chartView->setRenderHint(QPainter::Antialiasing);

	// connect
	connect(ui->btn_AddProfile, SIGNAL(clicked()), this, SLOT(startDrawPolyline())); // 开始绘制
	connect(ui->btn_deleteProfile, SIGNAL(clicked()), this, SLOT(deleteProfile())); // 删除一条剖面信息
	connect(ui->profileTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(showSelectedChart()));// 切换显示chart

	connect(ui->chartView, SIGNAL(mouseMove(QPointF, bool)), this, SLOT(showTooltip(QPointF, bool)));
}

ProfileWidget::~ProfileWidget()
{
	this->glWidget = nullptr;
	this->proRender = nullptr;
	delete this->emptyChart;
	this->emptyChart = nullptr;
    delete ui;
}

// 设置OpenGL widget
void ProfileWidget::setGLWindow(RenderWidget *glWidget) {
	this->glWidget = glWidget;
}

void ProfileWidget::setEffectPcd(QString pcdName) {
	auto renderlist = this->glWidget->getRenderList();
	for (int i = 0; i < renderlist.size(); i++) {
		if (renderlist[i]->getCurrentPcd()->name == pcdName) {
			this->proRender = renderlist[i];
			break;
		}
	}
}

// 拦截关闭事件
void ProfileWidget::closeEvent(QCloseEvent *e) {
	if (this->glWidget) {
		this->glWidget->toolCamera->setCameraType(Camera3D);
	}
	this->setPolylineVisible(false);

	e->ignore();
	this->hide();
}

// 拦截show事件
void ProfileWidget::showEvent(QShowEvent *e) {
	if (this->glWidget) {
		this->glWidget->toolCamera->setCameraType(Camera2D);
		this->glWidget->toolCamera->setProjection(Ortho);
	}
	this->setPolylineVisible(true);
}

// 开始绘制多段线
void ProfileWidget::startDrawPolyline() {
	if (this->glWidget) {
		ProfileInfo *profileInfo = new ProfileInfo();
		ToolDrawPolyline *toolDrawPolyline = new ToolDrawPolyline();
		// 设置profileInfo toolDrawPolyline
		profileInfo->toolDrawPolyline = toolDrawPolyline;
		
		this->profileInfoList.emplace_back(profileInfo);

		this->glWidget->RegisterExtraTool(toolDrawPolyline); // glWidget 内注册该多段线绘制工具

		// 连接结束绘制信号 根据绘制结果生成剖面
		connect(toolDrawPolyline, SIGNAL(endDraw(vector<Point>)), this, SLOT(drawProfile(vector<Point>)));
	}
	qDebug() << "Add Polyline!";
}

// 根据绘制好的polyline获取对应剖面点云
void ProfileWidget::drawProfile(vector<Point> polyline) {
	if (this->proRender == nullptr) {
		qDebug() << "ERROE: Not Set the Effect Pcd!";
		return;
	}
	if (toolDrawProfile == nullptr) {
		toolDrawProfile = new ToolDrawProfile(this->glWidget);
	}
	float Alllength = 0.0f;
	QChart *chart = toolDrawProfile->drawProfile(this->proRender, polyline, Alllength);
	auto *profileInfo = this->profileInfoList.back();
	// 设置Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->ui->profileTreeWidget);
	QString name = "profile #" + QString::number(this->profileNumber++);
	item->setText(0, name);
	this->ui->profileTreeWidget->addTopLevelItem(item);
	// 设置profile name
	profileInfo->name = name;
	// 设置profile 总长度
	profileInfo->Alllength = Alllength;
	// 设置profile chart
	profileInfo->chart = chart;
	chart->setTitle(name);

	ui->chartView->setChart(chart);
}

// 删除选中的剖面信息
void ProfileWidget::deleteProfile() {
	auto selectedItems = ui->profileTreeWidget->selectedItems();
	for (int i = 0; i < selectedItems.size(); i++) {
		QString profileName = selectedItems[i]->text(0);
		delete selectedItems[i];
		qDebug() << "Delete Profile Info: " << profileName;

		// 迭代 删除 Profile Info
		auto it = this->profileInfoList.begin();
		for (int j = 0; j < this->profileInfoList.size(), it != this->profileInfoList.end(); j++) {
			if (this->profileInfoList[j]->name == profileName) {
				auto profileInfo = this->profileInfoList[j];

				//delete chart
				ui->chartView->setChart(this->emptyChart);
				delete profileInfo->chart;
				profileInfo->chart = nullptr;
				// delete polylineTool
				profileInfo->toolDrawPolyline->deactivate();
				this->glWidget->RemoveExtraTool(profileInfo->toolDrawPolyline);
				profileInfo->toolDrawPolyline = nullptr;

				// delete Profile Info
				delete profileInfo;

				it = this->profileInfoList.erase(it);
				j--;
			}
			else {
				++it;
			}
		}
	}

	// 删除后 显示当前所存的最后一个节点剖面信息
	if (this->profileInfoList.size() > 0) {
		ui->chartView->setChart(this->profileInfoList.back()->chart);
	}
}

// 切换显示选中的 chart
void ProfileWidget::showSelectedChart() {
	auto selectedItems = ui->profileTreeWidget->selectedItems();
	if (selectedItems.size() != 1) {
		return;
	}
	QString profileName = selectedItems[0]->text(0);
	ProfileInfo *selectedProfile = this->getProfileInfoByName(profileName);

	ui->chartView->setChart(selectedProfile->chart);
}

// 设置所有多段线可见性
void ProfileWidget::setPolylineVisible(bool visible) {
	for (int i = 0; i < this->profileInfoList.size(); i++) {
		this->profileInfoList[i]->toolDrawPolyline->setVisible(visible);
	}
}

// 获取剖面信息
ProfileWidget::ProfileInfo* ProfileWidget::getProfileInfoByName(QString name) {
	for (int i = 0; i < this->profileInfoList.size(); i++) {
		if (this->profileInfoList[i]->name == name) {
			return this->profileInfoList[i];
		}
	}

	return nullptr;
}

// ChartView SLOT
void ProfileWidget::showTooltip(QPointF point, bool state) {
	if (this->glWidget == nullptr) {
		return;
	}

	if (!state && this->tooltip != nullptr) {
		this->tooltip->close();
	}
	else {
		if (this->tooltip == nullptr) {
			this->tooltip = new Tooltip(this->glWidget);
		}
		auto chartTitle = ui->chartView->chart()->title();
		ProfileInfo *profileInfo = this->getProfileInfoByName(chartTitle);

		// 计算当前鼠标点对应的polyline上的位置
		float scale = point.x() / profileInfo->Alllength; // 当前位置占整个实际剖面长度的比例

		auto polyline = profileInfo->toolDrawPolyline->getPolyline(); // polyline坐标系是屏幕窗体坐标系
		float AllLength = 0; // 多段线的总长度
		vector<float> subLengthArray; // 每一段长度集合
		// 计算多段线每一段长度 以及 总长度 顺便把polyline转成屏幕中心的笛卡尔坐标系
		for (int i = 0; i < polyline.size() - 1; i++) {
			if (i == 0) {
				polyline[i].x = polyline[i].x - (float)this->glWidget->width() / 2.0;
				polyline[i].y = (float)this->glWidget->height() / 2.0 - polyline[i].y;
			}
			float x1 = polyline[i].x;
			float y1 = polyline[i].y;

			polyline[i + 1].x = polyline[i + 1].x - (float)this->glWidget->width() / 2.0;
			polyline[i + 1].y = (float)this->glWidget->height() / 2.0 - polyline[i + 1].y;
			float x2 = polyline[i + 1].x;
			float y2 = polyline[i + 1].y;

			float subLength = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
			subLengthArray.emplace_back(subLength);
			AllLength += subLength;
		}

		float length = scale * AllLength; // 当前位置应该所处于的多段线长度位置

		// 寻找对应的屏幕位置
		QPointF screenPoint;
		for (int i = 0; i < subLengthArray.size(); i++) {
			length -= subLengthArray[i];
			if (length <= 0) {
				length += subLengthArray[i];
				// polyline已经是屏幕中心笛卡尔坐标系
				float x1 = polyline[i].x;
				float y1 =  polyline[i].y;
				float x2 = polyline[i + 1].x;
				float y2 = polyline[i + 1].y;
				Eigen::Vector2f vec = { x2 - x1,y2 - y1 };
				vec.normalize(); // 标准化
				vec *= length;
				vec.x() += x1;
				vec.y() += y1;
				// 转回屏幕坐标
				float screenPx = vec.x() + (float)this->glWidget->width() / 2.0;
				float screenPy = (float)this->glWidget->height() / 2.0 - vec.y();
				screenPoint.setX(screenPx);
				screenPoint.setY(screenPy);
				break;
			}
		}

		// set tooltip
		tooltip->setText("x:" + QString::number(point.x()) + "\n" + "y:" + QString::number(point.y()));
		tooltip->setAnchor(screenPoint);
		tooltip->updateGeometry();
		tooltip->show();
	}
}

