/*
 * @Descripttion: 点云数据及属性管理
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:20:28
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-08-01 00:03:17
 */
#pragma once

#include <iostream>
#include <vector>
#include <QTreeWidget>
#include <QAbstractItemView>
#include <Base/pcdManager.h>
#include <PointCloud/pointclouds.h>
#include <PointCloud/renderingWidget.h>


class PropertiesTreeDelegate;
class QStandardItemModel;

class PcdTreeWidget :public QTreeWidget {
	Q_OBJECT
public:
	PcdTreeWidget(QWidget *parent);
	~PcdTreeWidget();
};

class DBRoot : public QObject {
	Q_OBJECT
public:
	DBRoot(PcdTreeWidget *pcdTreeWidget, QTreeView *propertiesTreeView, RenderWidget *glWidget, QObject *parent = nullptr);
	~DBRoot();

public:
	void add_pointcloud(std::shared_ptr<PointCloud> &pcd); // DBTree Add Pcd

	vector<std::shared_ptr<PointCloud>> getSelectedPcd() const; // 选择当前dbTree下选择的点云
	vector<QString> getSelectedPcdName() const; // 选择当前dbTree下选择的点云名称

	void removeSelectedPcd(); // 删除选中的点云

protected:
	RenderWidget *glWidget = nullptr; // glWidget
	PcdManager *pcdManager = nullptr; // 点云数据管理
	PcdTreeWidget *pcdTreeWidget = nullptr; // DBTree
	QTreeView *propertiesTreeView = nullptr; // properties数据视图
	PropertiesTreeDelegate *propDelegate = nullptr; // 数据视图更新编辑委托
	QStandardItemModel *propertiesModel = nullptr; // data model

protected:
	void showPropertiesView(std::shared_ptr<PointCloud> &pcd); // 显示点云属性
	void hidePropertiesView();// 不显示任何信息

signals:
	void selectionChanged();

private slots:
	void setVisible(QTreeWidgetItem*, int);
	void changeSelection();
};