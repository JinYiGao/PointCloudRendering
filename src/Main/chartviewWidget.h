/*
 * @Descripttion: 自定义了一些ChartView的行为
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
	// 鼠标事件
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

private:
	Tooltip *tooltip = nullptr;
	bool is_translate = false;
	QPointF startPos;

	float getYValue(QPointF &p1, QPointF &p2, float x); // 已知直线两点 获取x处 y坐标

signals:
	void mouseMove(QPointF point, bool state); // 当前鼠标位置折线 x y 数值
};