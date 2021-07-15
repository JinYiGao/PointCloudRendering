/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-01-30 22:47:22
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-05-29 11:53:07
 */
#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <thread>
#include <qglobal.h>
#include <QMainWindow>
#include <QMessageBox>
#include <QButtonGroup>
#include <QToolButton>
#include <QIcon>

#include <IO/lasio.h>
#include <PointCloud/renderingWidget.h>

namespace Ui {
class MainWindow;
}
 
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
	void openfile();
	void isEDLChecked();
	void notShowColor();
	void changeMSAA();
	void showColorFromLabel();
	void showColorFromIdensity();
	void showColorFromRGB();
	void changePointSize(int value);
	void start_segement();
	void cancel_segement();
	void inPolygon_segement();
	void outPolygon_segement();
	void check_segement();

private:
    Ui::MainWindow *ui;
	QButtonGroup *checkBoxGroup = nullptr;

	QToolButton *toolbtn_segement = new QToolButton(this);
	QToolButton *toolbtn_inPolygon = new QToolButton(this);
	QToolButton *toolbtn_outPolygon = new QToolButton(this);
	QToolButton *toolbtn_cancel = new QToolButton(this);
	QToolButton *toolbtn_check = new QToolButton(this);

private:
	Eigen::MatrixXf getColorStrip(); // 生成色带

public:
	PointCloud *pcd = new PointCloud();
	RenderWidget *renderWidget = nullptr;
};

#endif // MAINWINDOW_H
