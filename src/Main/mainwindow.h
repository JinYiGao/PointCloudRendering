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
#include <QUndoStack>
#include <QAction>

#include <IO/lasio.h>

class DBRoot;
class PcdTreeWidget;
class SegmentationWidget;
class RenderWidget;
class EditPropertyWidget;
class ProfileWidget;

namespace Ui {
	class MainWindow;
}
 
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
	void openfile();
	void savefile();

	void isEDLChecked();
	void changeMSAA();
	void changeBudget();

	void start_segement();
	void deletePcd();
	void addPcdToOriginal();
	void pickPoint();
	void showEditWidget();
	void applyEditProperty(int classification);
	void quitEdit();

	void start_drawProfile();

	void enableUIItems();

private:
	Ui::MainWindow *ui;
	SegmentationWidget *segmentationWidget = nullptr;
	EditPropertyWidget *editPropertyWidget = nullptr;
	ProfileWidget *profileWidget = nullptr;

	RenderWidget *renderWidget = nullptr;
	PcdTreeWidget *pcdTreeWidget = nullptr;
	DBRoot *dbRoot = nullptr;
	QUndoStack *undoStack = nullptr;
	QAction *undoAction = nullptr;
	QAction *redoAction = nullptr;

public:
	static MainWindow *mainWindow; // 全局变量 获取MainWindow实例化指针
	QUndoStack* getUndoStack();
};

#endif // MAINWINDOW_H
