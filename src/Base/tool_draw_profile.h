/*
 * @Descripttion: 点云剖线绘制
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:47:13
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:47:14
 */
#pragma once 
#include <Base/tool.h>
#include <vector>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

using std::vector;

class RenderWidget;
class ProgressiveRender;

QT_CHARTS_USE_NAMESPACE

class ToolDrawProfile :public Tool {
	Q_OBJECT
public:
	ToolDrawProfile(RenderWidget *glWidget);
	~ToolDrawProfile();

	vector<int> getPointsByPolyline(ProgressiveRender *proRender, vector<Point> polyline, vector<Point> &gridPolyline);

	QChart *drawProfile(ProgressiveRender *proRender, vector<Point> polyline, float &Alllength);

private:
	RenderWidget *glWidget = nullptr;
};