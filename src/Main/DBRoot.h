/*
 * @Descripttion: 点云数据及属性管理
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:20:28
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-22 19:04:26
 */
#pragma once

#include <iostream>
#include <vector>
#include <QTreeWidget>
#include <QAbstractItemView>
#include <Base/pcdManager.h>
#include <PointCloud/pointclouds.h>
#include <PointCloud/renderingWidget.h>
#include <PropertiesTreeDelegate.h>


class MainWindow;

class PcdTreeWidget :public QTreeWidget {
	Q_OBJECT
public:
	PcdTreeWidget(QWidget *parent);
	~PcdTreeWidget();
};

class DBRoot : public QObject {
	Q_OBJECT
public:
	DBRoot(PcdTreeWidget *pcdTreeWidget, QTreeView *propertiesTreeView, QObject *parent = nullptr);
	~DBRoot();

public:
	void add_pointcloud(PointCloud *pcd); // DBTree Add Pcd

	vector<PointCloud *> getSelectedPcd() const;

protected:
	PcdManager *pcdManager = nullptr; // 点云数据管理
	PcdTreeWidget *pcdTreeWidget = nullptr; // DBTree
	QTreeView *propertiesTreeView = nullptr; // properties数据视图
	PropertiesTreeDelegate *propDelegate = nullptr; // 数据视图更新编辑委托
	QStandardItemModel *propertiesModel = nullptr; // data model

protected:
	void showPropertiesView(PointCloud *pcd); // 显示点云属性
	void hidePropertiesView();// 不显示任何信息

signals:
	void selectionChanged();

private slots:
	void setVisible(QTreeWidgetItem*, int);
	void changeSelection();
};