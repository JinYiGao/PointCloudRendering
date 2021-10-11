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
	this->polygon.clear();
}

ToolDrawPolygon::~ToolDrawPolygon(){

}

void ToolDrawPolygon::activate() {
	this->isActivate = true;
	this->drawing = true;
}

void ToolDrawPolygon::deactivate() {
	reset();
	this->isActivate = false;
}

// 重置
void ToolDrawPolygon::reset() {
	this->polygon.clear();
	this->drawing = false;
}

// 暂停绘制
void ToolDrawPolygon::suspend() {
	this->drawing = false;
}

// 恢复绘制
void ToolDrawPolygon::resume() {
	this->drawing = true;
}

// 获取Tool类型
int ToolDrawPolygon::getToolType() {
	return type;
}

// 键鼠操作
void ToolDrawPolygon::mousePress(QMouseEvent *e) {
	if (!isActivate) {
		return;
	}
	if (drawing && e->buttons() == Qt::LeftButton) {
		Point point(e->localPos().x(), e->localPos().y());
		polygon.emplace_back(point);
	}
}

void ToolDrawPolygon::mouseMove(QMouseEvent *e) {
	if (!isActivate) {
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
	if (!isActivate) {
		return;
	}
	if (drawing) {
		polygon.pop_back();
		suspend(); // 暂停绘制
	}
}

// 键盘事件
void ToolDrawPolygon::keyPress(QKeyEvent *e) {
	if (!isActivate) {
		return;
	}
	if (e->key() == Qt::Key_Escape) {
		this->deactivate();
	}
}

// QPainter 绘制
void ToolDrawPolygon::draw(QPainter *painter) {
	if (!isActivate) {
		return;
	}
	// Text
	auto device = painter->device();
	int width = device->width();
	int height = device->height();
	painter->setPen(QPen(Qt::white, 1));
	// 状态显示
	painter->drawText(QRectF(QPointF(width / 2.0 - 150, 10), QPointF(width / 2.0 + 150, 30)), "Drawing Polygon ... Press [ESC] Exit");

	if (polygon.size() > 0) {
		painter->setPen(QPen(Qt::green, 1));
		painter->setBrush(QBrush(QColor(0, 255, 0, 120)));
		QPolygon pts(polygon.size());
		for (int i = 0; i < polygon.size(); i++) {
			pts.putPoints(i, 1, polygon[i].x, polygon[i].y);
		}
		pts.putPoints(polygon.size(), 1, polygon[0].x, polygon[0].y);
		painter->drawPolygon(pts);
	}
}

void ToolDrawPolygon::gl_draw() {
	if (!isActivate) {
		return;
	}
}