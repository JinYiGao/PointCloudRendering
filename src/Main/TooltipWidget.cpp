/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-10-09 22:16:00
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-10-09 22:16:00
 */
#include <TooltipWidget.h>

Tooltip::Tooltip(QWidget *parent) :
	QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
}

// 设置锚点 即气泡指向点
void Tooltip::setAnchor(QPointF point)
{
	m_anchor = point;
}

void Tooltip::setText(const QString &text)
{
	m_text = text;
	QFontMetrics metrics(m_font);
	m_textRect = metrics.boundingRect(QRect(0, 0, 150, 150), Qt::AlignLeft, m_text);
	m_textRect.translate(5, 5);
	m_rect = m_textRect.adjusted(-5, -5, 5, 5);
}

void Tooltip::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	QPainterPath path;
	path.addRoundedRect(m_rect, 5, 5);

	QPointF anchor = m_anchor - this->pos(); // 相对于左上角的位置

	QPointF point1, point2; // 气泡三角形与边框左右交点

	// establish the position of the anchor point in relation to m_rect
	bool above = anchor.y() <= m_rect.top();
	bool aboveCenter = anchor.y() > m_rect.top() && anchor.y() <= m_rect.center().y();
	bool belowCenter = anchor.y() > m_rect.center().y() && anchor.y() <= m_rect.bottom();
	bool below = anchor.y() > m_rect.bottom();

	bool onLeft = anchor.x() <= m_rect.left();
	bool leftOfCenter = anchor.x() > m_rect.left() && anchor.x() <= m_rect.center().x();
	bool rightOfCenter = anchor.x() > m_rect.center().x() && anchor.x() <= m_rect.right();
	bool onRight = anchor.x() > m_rect.right();

	// get the nearest m_rect corner.
	qreal x = (onRight + rightOfCenter) * m_rect.width();
	qreal y = (below + belowCenter) * m_rect.height();
	bool cornerCase = (above && onLeft) || (above && onRight) || (below && onLeft) || (below && onRight);
	bool vertical = qAbs(anchor.x() - x) > qAbs(anchor.y() - y);

	qreal x1 = x + leftOfCenter * 10 - rightOfCenter * 20 + cornerCase * !vertical * (onLeft * 10 - onRight * 20);
	qreal y1 = y + aboveCenter * 10 - belowCenter * 20 + cornerCase * vertical * (above * 10 - below * 20);
	point1.setX(x1);
	point1.setY(y1);

	qreal x2 = x + leftOfCenter * 20 - rightOfCenter * 10 + cornerCase * !vertical * (onLeft * 20 - onRight * 10);
	qreal y2 = y + aboveCenter * 20 - belowCenter * 10 + cornerCase * vertical * (above * 20 - below * 10);
	point2.setX(x2);
	point2.setY(y2);

	path.moveTo(point1);
	path.lineTo(anchor);
	path.lineTo(point2);
	path = path.simplified();
	
	painter.setBrush(QColor(255, 255, 255));
	painter.drawPath(path);
	painter.drawText(m_textRect, m_text);
}

void Tooltip::updateGeometry() {
	this->move(m_anchor.x() - AnchorToMarginLeft, m_anchor.y() - m_rect.height() - AnchorToMarginBottom);
	this->setFixedSize(m_rect.width() + 5, m_rect.height() + AnchorToMarginBottom + 2);
}

void Tooltip::mousePressEvent(QMouseEvent *)
{
	this->close();
}