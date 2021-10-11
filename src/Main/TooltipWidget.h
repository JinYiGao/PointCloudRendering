/*
 * @Descripttion: tooltip
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-10-09 22:15:47
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-10-09 22:15:47
 */
#pragma once
#include <QWidget>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <iostream>

const int AnchorToMarginLeft = 12; // 气泡三角形距离左边界长度
const int AnchorToMarginBottom = 15; // 气泡三角形距离框底部长度

class Tooltip : public QWidget
{
	Q_OBJECT
public:
	Tooltip(QWidget *parent = nullptr);

	void setAnchor(QPointF point);
	void setText(const QString &text);
	void updateGeometry();

protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);

private:
	QString m_text;
	QRectF m_textRect;
	QRectF m_rect;
	QPointF m_anchor;
	QFont m_font;
};