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
#include <Main/DBRoot.h>
#include <Main/segmentationwidget.h>
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
	void changeMSAA();

	void start_segement();

	void enableUIItems();

private:
	QToolButton *toolbtn_segment = new QToolButton(this);

public:
	Ui::MainWindow *ui;
	SegmentationWidget *segmentationWidget = nullptr;

	RenderWidget *renderWidget = nullptr;
	PcdTreeWidget *pcdTreeWidget = nullptr;
	DBRoot *dbRoot = nullptr;
};

#endif // MAINWINDOW_H
