/*
 * @Descripttion: �Զ�����һЩChartView����Ϊ
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-10-09 15:27:29
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-10-09 15:27:29
 */
#pragma once

#include <qevent.h>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include <iostream>

#include <Main/TooltipWidget.h>

QT_CHARTS_USE_NAMESPACE

class ChartView :public QChartView {
	Q_OBJECT
public:
	ChartView(QWidget *parent = nullptr);
	ChartView(QChart *chart, QWidget *parent = nullptr);
	~ChartView();

protected:
	// ����¼�
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

private:
	Tooltip *tooltip = nullptr;
	bool is_translate = false;
	QPointF startPos;

	float getYValue(QPointF &p1, QPointF &p2, float x); // ��ֱ֪������ ��ȡx�� y����

signals:
	void mouseMove(QPointF point, bool state); // ��ǰ���λ������ x y ��ֵ
};