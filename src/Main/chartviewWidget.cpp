/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-10-09 15:27:35
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-10-09 15:27:36
 */
#include <Main/chartviewWidget.h>
#include <qtooltip.h>

ChartView::ChartView(QWidget *parent) :QChartView(parent) {

}

ChartView::ChartView(QChart *chart, QWidget *parent) : QChartView(chart, parent) {

}

ChartView::~ChartView() {

}

float ChartView::getYValue(QPointF &p1, QPointF &p2, float x) {
	float x1 = p1.x();
	float y1 = p1.y();
	float x2 = p2.x();
	float y2 = p2.y();

	return (y2 - y1) / (x2 - x1) * (x - x1) + y1;
}

void ChartView::mousePressEvent(QMouseEvent *e) {
	if (e->buttons() == Qt::MiddleButton) {
		this->is_translate = true;

		this->startPos.setX(e->localPos().x());
		this->startPos.setY(e->localPos().y());
	}
}

void ChartView::mouseReleaseEvent(QMouseEvent *e) {
	this->is_translate = false;
}

void ChartView::mouseMoveEvent(QMouseEvent *e) {
	if (chart()->series().size() == 0) {
		return;
	}

	auto const widgetPos = e->localPos();

	// �м�����ƽ��
	if (e->buttons() == Qt::MiddleButton) {
		auto translation = widgetPos - startPos;

		auto axisX = (QValueAxis*)chart()->axisX();
		auto axisY = (QValueAxis*)chart()->axisY();
		float minX = axisX->min();
		float maxX = axisX->max();
		float minY = axisY->min();
		float maxY = axisY->max();
		QRectF plotArea = chart()->plotArea(); // ��ͼ����
		float scaleX = (maxX - minX) / plotArea.width();
		float scaleY = (maxY - minY) / plotArea.height();
		float translationX = translation.x() * scaleX;
		float translationY = translation.y() * scaleY;
		axisX->setRange(minX - translationX, maxX - translationX);
		axisY->setRange(minY + translationY, maxY + translationY);
		this->startPos = widgetPos;
		return;
	}

	// �ж�����Ƿ����������ڲ�
	bool flag = chart()->plotArea().contains(widgetPos);
	if (!flag) {
		emit mouseMove(QPointF(0.0, 0.0), flag);
		return;
	}
	auto const valueGivenSeries = chart()->mapToValue(widgetPos); // chartȡֵ

	QPointF point;
	flag = false;
	// ����series���ݼ�
	for (int i = 0; i < chart()->series().size(); i++) {
		QLineSeries *series = (QLineSeries*)chart()->series()[i];
		auto points = series->pointsVector();
		// ����ÿ�����ݼ��ϵĵ�
		for (int j = 0; j < points.size(); j++) {
			if (points[j].x() >= valueGivenSeries.x()) {
				// ��ʾ�����ݼ����
				if (j == 0) {
					break;
				}
				if (points[j].x() == valueGivenSeries.x()) {
					point.setX(valueGivenSeries.x());
					point.setY(points[j].y());
				}
				else {
					float y = this->getYValue(points[j - 1], points[j], valueGivenSeries.x()); // ���㵱ǰx��Ӧ�� yֵ ����
					point.setX(valueGivenSeries.x());
					point.setY(y);
				}
				flag = true;
				break;
			}
		}
	}
	emit mouseMove(point, flag);
}

void ChartView::wheelEvent(QWheelEvent *e) {
	float coeff = e->angleDelta().y() > 0 ? 1.25 : 0.8;
	chart()->zoom(coeff);
}