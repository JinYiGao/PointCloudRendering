/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-28 21:55:25
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-28 21:55:26
 */
#include <Base/tool_drawPolyline.h>
#include <PointCloud/renderingWidget.h>


ToolDrawPolyline::ToolDrawPolyline() {
	this->polyline.clear();
}

ToolDrawPolyline::~ToolDrawPolyline() {

}

void ToolDrawPolyline::activate() {
	this->isActivate = true;
	this->drawing = true;
}

void ToolDrawPolyline::deactivate() {
	reset();
	this->isActivate = false;
}

// 重置
void ToolDrawPolyline::reset() {
	this->polyline.clear();
	this->drawing = false;
}

// 暂停绘制
void ToolDrawPolyline::suspend() {
	this->drawing = false;
}

// 恢复绘制
void ToolDrawPolyline::resume() {
	this->drawing = true;
}

// 获取Tool类型
int ToolDrawPolyline::getToolType() {
	return type;
}

// 键鼠操作
void ToolDrawPolyline::mousePress(QMouseEvent *e) {
	if (!isActivate) {
		return;
	}
	if (drawing && e->buttons() == Qt::LeftButton) {
		Point point(e->localPos().x(), e->localPos().y());
		polyline.emplace_back(point);
	}
	if (e->buttons() == Qt::MiddleButton) {
		this->is_translate = true;
		this->startPos = { e->localPos().x(), e->localPos().y() };
	}
}

void ToolDrawPolyline::mouseRelease(QMouseEvent *e) {
	if (this->is_translate) {
		this->is_translate = false;

		auto translate = this->endPos - this->startPos;
		for (int i = 0; i < polyline.size(); i++) {
			polyline[i].x += translate.x();
			polyline[i].y += translate.y();
		}
	}
}

void ToolDrawPolyline::mouseMove(QMouseEvent *e) {
	if (!isActivate) {
		return;
	}

	if (is_translate) {
		this->endPos = { e->localPos().x(), e->localPos().y() };
	}

	if (drawing && !is_translate) {
		Point point(e->localPos().x(), e->localPos().y());
		if (polyline.size() == 1) {
			polyline.emplace_back(point);
		}
		else if (polyline.size() > 1) {
			polyline.pop_back();
			polyline.emplace_back(point);
		}
	}
}

void ToolDrawPolyline::mouseDoubleClick(QMouseEvent *e) {
	if (!isActivate) {
		return;
	}
	if (drawing) {
		polyline.pop_back();
		suspend(); // 暂停绘制

		emit endDraw(this->polyline); // 停止绘制
	}
}

void ToolDrawPolyline::wheelEvent(QWheelEvent *e) {
	float coeff = e->angleDelta().y() > 0 ? 1.1 : 0.9;
	// 中心缩放
	for (int i = 0; i < polyline.size(); i++) {
		double x = (polyline[i].x - width / 2.0) * coeff;
		double y = (height / 2.0 - polyline[i].y) * coeff;
		polyline[i].x = x + width / 2.0;
		polyline[i].y = height / 2.0 - y;
	}
}

// 键盘事件
void ToolDrawPolyline::keyPress(QKeyEvent *e) {
	if (!isActivate) {
		return;
	}
	//if (e->key() == Qt::Key_Escape) {
	//	this->deactivate();

	//	emit exit(); // 退出工具信号
	//}
}

// QPainter 绘制
void ToolDrawPolyline::draw(QPainter *painter) {
	if (!isActivate) {
		return;
	}
	if (!visible) {
		return;
	}
	// Text
	auto device = painter->device();
	width = device->width();
	height = device->height();
	painter->setPen(QPen(Qt::white, 1));
	// 状态显示
	painter->drawText(QRectF(QPointF(width / 2.0 - 150, 10), QPointF(width / 2.0 + 150, 30)), "Drawing Polyline ...");

	if (polyline.size() > 0 && !is_translate) {
		painter->setPen(QPen(Qt::green, 1));
		painter->setBrush(QBrush(QColor(0, 255, 0, 120)));
		QPolygon pts(polyline.size());
		for (int i = 0; i < polyline.size(); i++) {
			pts.putPoints(i, 1, polyline[i].x, polyline[i].y);
		}
		//pts.putPoints(polyline.size(), 1, polyline[0].x, polyline[0].y);
		painter->drawPolyline(pts);
	}
}

void ToolDrawPolyline::gl_draw() {
	if (!isActivate) {
		return;
	}
}

void ToolDrawPolyline::setVisible(bool visible) {
	this->visible = visible;
}

vector<Point> ToolDrawPolyline::getPolyline() const {
	return this->polyline;
}
