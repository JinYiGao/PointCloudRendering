/*
 * @Descripttion: ����߻��ƹ���
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-28 21:55:17
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-28 21:55:18
 */
#pragma once 

#include <vector>
#include <Base/tool.h>
#include <Eigen/Eigen>

using std::vector;

class RenderWidget;

class ToolDrawPolyline :public Tool {
	Q_OBJECT
public:
	ToolDrawPolyline();
	~ToolDrawPolyline();

protected:
	vector<Point> polyline; // ���������
	bool isActivate = false;  // �Ƿ����ù���

	bool visible = true; // ����߿ɼ���
	bool drawing = false; // �Ƿ��ڻ���״̬
	bool is_translate = false; // �Ƿ���ƽ��״̬

	int type = DrawPolylineTool;
	int width;
	int height;

	Eigen::Vector2i startPos;
	Eigen::Vector2i endPos;

public:
	// ��д�����麯��
	void activate() override; // ���ù���
	void deactivate() override; // �˳�����
	void suspend() override; // ��ͣ����
	void resume() override; // �ָ�����

	int getToolType() override;

	void reset() override; // ����

	void draw(QPainter *painter) override;
	void gl_draw() override;

	void setVisible(bool visible);
	vector<Point> getPolyline() const;

	// �������
	void mousePress(QMouseEvent *e) override;
	void mouseRelease(QMouseEvent *e) override;
	void mouseMove(QMouseEvent *e) override;
	void mouseDoubleClick(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;
	// ���̲���
	void keyPress(QKeyEvent *e) override;

signals:
	void endDraw(vector<Point> polyline);
	void exit();
};