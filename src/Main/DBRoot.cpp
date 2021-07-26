/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:20:34
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-21 14:20:34
 */
#include <DBRoot.h>

static const int propViewLeftColumnWidth = 115;

DBRoot::DBRoot(PcdTreeWidget *pcdTreeWidget, QTreeView *propertiesTreeView, QObject *parent){
	this->pcdManager = PcdManager::GetInstance();

	this->pcdTreeWidget = pcdTreeWidget;
	this->propertiesTreeView = propertiesTreeView;

	this->pcdTreeWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	// DBTree
	connect(this->pcdTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(setVisible(QTreeWidgetItem*, int)));
	connect(this->pcdTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(changeSelection()));

	// PropertiesDelegate
	propertiesModel = new QStandardItemModel(0, 2, parent);
	propDelegate = new PropertiesTreeDelegate(propertiesModel, propertiesTreeView);
	this->propertiesTreeView->setItemDelegate(propDelegate); // set Delegate
	this->propertiesTreeView->setModel(propertiesModel);
	this->propertiesTreeView->header()->setSectionResizeMode(QHeaderView::Interactive);
	this->propertiesTreeView->setEnabled(false);
}

DBRoot::~DBRoot() {

}

void DBRoot::add_pointcloud(PointCloud *pcd) {
	// 设置Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->pcdTreeWidget);
	item->setText(0, QString::fromStdString(pcd->name));
	item->setCheckState(0, Qt::Checked);
	this->pcdTreeWidget->addTopLevelItem(item);
	// 添加到点云管理
	pcdManager->add_pointcloud(pcd);
}

void DBRoot::setVisible(QTreeWidgetItem* item, int index) {
	QString name = item->text(0);
	auto pcd = pcdManager->getPointCloud(name.toStdString());
	if (pcd == nullptr) {
		return;
	}
	if (item->checkState(0) == Qt::Checked) {
		pcd->setVisible(true);
	}
	else {
		pcd->setVisible(false);
	}
}

void DBRoot::changeSelection() {
	auto selectlist = this->pcdTreeWidget->selectedItems();
	if (selectlist.size() == 1) {
		auto item = selectlist.at(0);
		QString name = item->text(0);
		qDebug() << name;
		PointCloud *pcd = this->pcdManager->getPointCloud(name.toStdString());

		showPropertiesView(pcd);
	}
	// 选中个数不为一个则不显示任何信息
	else {
		hidePropertiesView();
	}

	emit selectionChanged();
}

vector<PointCloud *> DBRoot::getSelectedPcd() const {
	vector<PointCloud*> pcdlist;
	auto selectlist = this->pcdTreeWidget->selectedItems();
	for (int i = 0; i < selectlist.size(); i++) {
		auto item = selectlist.at(i);
		QString name = item->text(0);
		PointCloud *pcd = this->pcdManager->getPointCloud(name.toStdString());
		pcdlist.emplace_back(pcd);
	}
	return pcdlist;
}

void DBRoot::showPropertiesView(PointCloud *pcd) {
	this->propDelegate->fillModel(pcd);

	propertiesTreeView->setEnabled(true);
	propertiesTreeView->setColumnWidth(0, propViewLeftColumnWidth);
	propertiesTreeView->setColumnWidth(1, propViewLeftColumnWidth);
}

void DBRoot::hidePropertiesView() {
	std::cout << "hide" << std::endl;

	propDelegate->unbind();
	propertiesModel->clear();
	propertiesTreeView->setEnabled(false);
}

PcdTreeWidget::PcdTreeWidget(QWidget *parent) : QTreeWidget(parent) {

}

PcdTreeWidget::~PcdTreeWidget() {

}