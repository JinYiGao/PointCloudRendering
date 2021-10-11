/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:47:24
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:47:24
 */
#include <Base/tool_draw_profile.h>
#include <PointCloud/renderingWidget.h>
#include <PointCloud/progressiveRender.h>

ToolDrawProfile::ToolDrawProfile(RenderWidget *glWidget) {
	this->glWidget = glWidget;
}

ToolDrawProfile::~ToolDrawProfile(){

}

vector<int> ToolDrawProfile::getPointsByPolyline(ProgressiveRender *proRender, vector<Point> polyline, vector<Point> &gridPolyline) {
	return proRender->selectPointsByPolyline(polyline, gridPolyline);
}

QChart* ToolDrawProfile::drawProfile(ProgressiveRender *proRender, vector<Point> polyline,float &Alllength) {
	vector<Point> gridPolyline;
	auto pointsIndex = this->getPointsByPolyline(proRender, polyline, gridPolyline);

	QLineSeries *series = new QLineSeries();
	series->setName("line");

	Alllength = 0;
	float maxZval = -100000;
	float minZval = 100000;
	auto pcd = proRender->getCurrentPcd();

	Eigen::Vector3f startPoint = Eigen::Vector3f::Identity();
	Eigen::Vector3f endPoint = Eigen::Vector3f::Identity();
	for (int i = 0; i < pointsIndex.size(); i++) {
		// ������·����������դ���ʵ��3d xy���� �������
		if (i > 0) {
			Eigen::Vector2f screenP1 = Eigen::Vector2f::Identity();
			screenP1.x() = gridPolyline[i - 1].x + this->glWidget->width() / 2.0;
			screenP1.y() = this->glWidget->height() / 2.0 - gridPolyline[i - 1].y;

			Eigen::Vector2f screenP2 = Eigen::Vector2f::Identity();
			screenP2.x() = gridPolyline[i].x + this->glWidget->width() / 2.0;
			screenP2.y() = this->glWidget->height() / 2.0 - gridPolyline[i].y;

			Eigen::Vector3f p1 = this->glWidget->camera->convert_2dTo3d(screenP1);
			Eigen::Vector3f p2 = this->glWidget->camera->convert_2dTo3d(screenP2);
			Alllength += sqrt(pow(p2.x() - p1.x(), 2) + pow(p2.y() - p1.y(), 2));
		}

		if (pointsIndex[i] != -1) {
			uint targetIndex = pcd->indexTable[pointsIndex[i]];
			Eigen::Vector3f point = pcd->position.col(targetIndex);

			point.z() > maxZval ? maxZval = point.z() : maxZval = maxZval;
			point.z() < minZval ? minZval = point.z() : minZval = minZval;

			// ������յ����ռλ
			if (startPoint.isIdentity()) {
				startPoint = point;
				series->append(Alllength, point.z()); // ���ֻ�����ڳ�ʼΪIdentity ����ȫ����endPoint��ֵ
			}
			else if (endPoint.isIdentity()) {
				endPoint = point;
			}
			// ������յ� ���е� ����
			if (!startPoint.isIdentity() && !endPoint.isIdentity()) {
				double zVal = endPoint.z();
				//Alllength += sqrt(pow(endPoint.x() - startPoint.x(), 2) + pow(endPoint.y() - startPoint.y(), 2));
				series->append(Alllength, zVal);

				startPoint = endPoint;
				endPoint = Eigen::Vector3f::Identity();
			}
		}
	}

	QChart *chart = new QChart();
	chart->legend()->hide();
	chart->addSeries(series);
	chart->createDefaultAxes();
	//chart->axisY()->setRange(minZval, maxZval);

	chart->setBackgroundBrush(QBrush(QColor(102, 102, 102, 42)));

	return chart;
}