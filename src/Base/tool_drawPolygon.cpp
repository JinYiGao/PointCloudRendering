/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-18 20:46:34
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-18 20:46:35
 */
#include <tool_drawPolygon.h>
#include <PointCloud/renderingWidget.h>

ToolDrawPolygon::ToolDrawPolygon() {
	this->enable = false;
	this->polygon.clear();
}

ToolDrawPolygon::~ToolDrawPolygon(){

}

void ToolDrawPolygon::activate() {
	this->enable = true;
	this->drawing = true;
}

void ToolDrawPolygon::deactivate() {
	reset();
	this->enable = false;
	this->drawing = false;
}

// ÖØÖÃ
void ToolDrawPolygon::reset() {
	this->polygon.clear();
	this->drawing = false;
}

// ÔÝÍ£»æÖÆ
void ToolDrawPolygon::suspend() {
	this->drawing = false;
}

// »Ö¸´»æÖÆ
void ToolDrawPolygon::resume() {
	this->drawing = true;
}

int ToolDrawPolygon::getToolType() {
	return type;
}

// ¼üÊó²Ù×÷
void ToolDrawPolygon::mousePress(QMouseEvent *e) {
	if (!enable) {
		return;
	}
	if (drawing && e->buttons() == Qt::LeftButton) {
		Point point(e->localPos().x(), e->localPos().y());
		polygon.emplace_back(point);
	}
}

void ToolDrawPolygon::mouseMove(QMouseEvent *e) {
	if (!enable) {
		return;
	}
	if (drawing) {
		Point point(e->localPos().x(), e->localPos().y());
		if (polygon.size() == 1) {
			polygon.emplace_back(point);
		}
		else if (polygon.size() > 1) {
			polygon.pop_back();
			polygon.emplace_back(point);
		}
	}
}

void ToolDrawPolygon::mouseDoubleClick(QMouseEvent *e) {
	if (!enable) {
		return;
	}
	if (drawing) {
		polygon.pop_back();
		suspend(); // ÔÝÍ£»æÖÆ
	}
}

void ToolDrawPolygon::draw(QPainter *painter) {
	if (!enable) {
		return;
	}
	if (polygon.size() > 0) {
		painter->setPen(QPen(Qt::green, 1));
		QPolygon pts(polygon.size());
		for (int i = 0; i < polygon.size(); i++) {
			pts.putPoints(i, 1, polygon[i].x, polygon[i].y);
		}
		pts.putPoints(polygon.size(), 1, polygon[0].x, polygon[0].y);
		painter->drawConvexPolygon(pts);
		painter->drawText(50, 50, QString("Drawing"));
		painter->end();
	}
}

void ToolDrawPolygon::gl_draw() {
	if (!enable) {
		return;
	}
}