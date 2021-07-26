/*
 * @Descripttion: 点云属性信息树展示委托
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 20:41:52
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-22 19:05:34
 */
#pragma once

#include <PointCloud/pointclouds.h>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVariant>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <qstyleditemdelegate.h>

enum PROPERTY_ROLE {
	OBJECT_NO_PROPERTY = 0,
	OBJECT_NAME,
	OBJECT_VISIBILITY,
	OBJECT_COLOR,
	OBJECT_CLOUD_POINT_SIZE,
	TREE_VIEW_HEADER
};

class PropertiesTreeDelegate :  public QStyledItemDelegate {
	Q_OBJECT
public:
	PropertiesTreeDelegate(QStandardItemModel* _model, QAbstractItemView* _view, QObject *parent = nullptr);
	~PropertiesTreeDelegate();

	// 重写 父类函数
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;

	void fillModel(PointCloud *pcd); // 视图填充数据

	void unbind();

	PointCloud *getCurrentPcd();
private:
	PointCloud *currentPcd = nullptr;
	QStandardItemModel* m_model;
	QAbstractItemView* m_view;

	void addSeparator(const QString& title);
	void appendRow(QStandardItem* leftItem, QStandardItem* rightItem, bool openPersistentEditor = false);
	
	void fillWithPcd(PointCloud *pcd);

	bool isWideEditor(int itemData) const; // 根据role判断该item whether the editor is wide (i.e. spans on two columns) or not

public slots:
	void colorChanged(QString);
	void changePointSize(int);
	void updateItem(QStandardItem*);
};