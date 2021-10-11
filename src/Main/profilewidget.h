#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QDialog>
#include <Base/tool_drawPolyline.h>
#include <Base/tool_draw_profile.h>
#include <Main/TooltipWidget.h>

class RenderWidget;
class ToolDrawPolyline;

struct Point;

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class ProfileWidget;
}

class ProfileWidget : public QDialog{
	Q_OBJECT

	// 剖线信息
	struct ProfileInfo {
		QString name; // 剖面名字
		QChart *chart; // 该剖面对应图表信息
		ToolDrawPolyline *toolDrawPolyline; // 该剖线对应的多段线信息
		float Alllength; // 剖面总长度

		ProfileInfo() {
			this->name = "";
			this->chart = nullptr;
			this->toolDrawPolyline = nullptr;
			this->Alllength = 0.0f;
		}
	};

public:
    explicit ProfileWidget(QWidget *parent = nullptr);
    ~ProfileWidget();

private:
    Ui::ProfileWidget *ui;

	RenderWidget *glWidget = nullptr;
	ProgressiveRender *proRender = nullptr;
	vector<ProfileInfo*> profileInfoList; // 剖线信息集合
	int profileNumber = 0;
	QChart *emptyChart = new QChart();
	ToolDrawProfile *toolDrawProfile = nullptr;

	Tooltip *tooltip = nullptr;

public:
	void setGLWindow(RenderWidget *glWidget);
	void setEffectPcd(QString pcdName); // 设置作用的 点云

private:
	void closeEvent(QCloseEvent *e) override;
	void showEvent(QShowEvent *e) override;
	void setPolylineVisible(bool visible);
	ProfileInfo* getProfileInfoByName(QString name);

public slots:
	void startDrawPolyline(); // 开始绘制多段线
	void drawProfile(vector<Point> polyline); // 绘制剖面数据
	void deleteProfile(); // 删除一条剖面信息
	void showSelectedChart();
	void showTooltip(QPointF point, bool state);
};

#endif // PROFILEWIDGET_H
