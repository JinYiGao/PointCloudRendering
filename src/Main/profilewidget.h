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

	// ������Ϣ
	struct ProfileInfo {
		QString name; // ��������
		QChart *chart; // �������Ӧͼ����Ϣ
		ToolDrawPolyline *toolDrawPolyline; // �����߶�Ӧ�Ķ������Ϣ
		float Alllength; // �����ܳ���

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
	vector<ProfileInfo*> profileInfoList; // ������Ϣ����
	int profileNumber = 0;
	QChart *emptyChart = new QChart();
	ToolDrawProfile *toolDrawProfile = nullptr;

	Tooltip *tooltip = nullptr;

public:
	void setGLWindow(RenderWidget *glWidget);
	void setEffectPcd(QString pcdName); // �������õ� ����

private:
	void closeEvent(QCloseEvent *e) override;
	void showEvent(QShowEvent *e) override;
	void setPolylineVisible(bool visible);
	ProfileInfo* getProfileInfoByName(QString name);

public slots:
	void startDrawPolyline(); // ��ʼ���ƶ����
	void drawProfile(vector<Point> polyline); // ������������
	void deleteProfile(); // ɾ��һ��������Ϣ
	void showSelectedChart();
	void showTooltip(QPointF point, bool state);
};

#endif // PROFILEWIDGET_H
