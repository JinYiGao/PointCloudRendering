/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:20:34
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-31 23:37:46
 */
#include <DBRoot.h>
#include <PropertiesTreeDelegate.h>

static const int propViewLeftColumnWidth = 115;

DBRoot::DBRoot(PcdTreeWidget *pcdTreeWidget, QTreeView *propertiesTreeView, RenderWidget *glWidget, QObject *parent){
	this->pcdManager = PcdManager::GetInstance();

	this->pcdTreeWidget = pcdTreeWidget;
	this->propertiesTreeView = propertiesTreeView;
	this->glWidget = glWidget;

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

void DBRoot::add_pointcloud(std::shared_ptr<PointCloud> &pcd) {
	// 设置Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->pcdTreeWidget);
	QString name = pcd->name;
	int i = 1;
	// exist pcdname ?
	while (pcdManager->existPcd(name)) {
		name = pcd->name + QString::number(i);
		i++;
	}
	pcd->name = name;
	item->setText(0, name);
	item->setCheckState(0, Qt::Checked);
	this->pcdTreeWidget->addTopLevelItem(item);
	// 添加到点云管理
	pcdManager->addPcdToManager(pcd);

	// glWidget add Pcd
	glWidget->addPointCloud(pcd);
	glWidget->setMouseTracking(true);
}

void DBRoot::setVisible(QTreeWidgetItem* item, int index) {
	QString name = item->text(0);
	auto pcd = pcdManager->getPointCloud(name);
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
		std::shared_ptr<PointCloud> pcd = this->pcdManager->getPointCloud(name);

		showPropertiesView(pcd);
	}
	// 选中个数不为一个则不显示任何信息
	else {
		hidePropertiesView();
	}

	emit selectionChanged();
}

vector<std::shared_ptr<PointCloud>> DBRoot::getSelectedPcd() const {
	vector<std::shared_ptr<PointCloud>> pcdlist;
	auto selectlist = this->pcdTreeWidget->selectedItems();
	for (int i = 0; i < selectlist.size(); i++) {
		auto item = selectlist.at(i);
		QString name = item->text(0);
		std::shared_ptr<PointCloud> pcd = this->pcdManager->getPointCloud(name);
		pcdlist.emplace_back(pcd);
	}
	return pcdlist;
}

vector<QString> DBRoot::getSelectedPcdName() const {
	vector<QString> namelist;
	auto selectlist = this->pcdTreeWidget->selectedItems();
	for (int i = 0; i < selectlist.size(); i++) {
		auto item = selectlist.at(i);
		QString name = item->text(0);
		namelist.emplace_back(name);
	}
	return namelist;
}

// 统一删除所有点云
void DBRoot::removeSelectedPcd() {
	auto selectlist = this->pcdTreeWidget->selectedItems();
	if (selectlist.size() == 0) {
		return;
	}
	for (int i = 0; i < selectlist.size(); i++) {
		auto item = selectlist.at(i);
		QString name = item->text(0);
		delete selectlist[i]; // dbtree 删除
		this->pcdManager->remove_pointcloud(name); // pcdManager 删除
		this->glWidget->removePointCloud(name); // glWidget 删除
	}
}

void DBRoot::showPropertiesView(std::shared_ptr<PointCloud> &pcd) {
	this->propDelegate->fillModel(pcd);

	propertiesTreeView->setEnabled(true);
	propertiesTreeView->setColumnWidth(0, propViewLeftColumnWidth);
	propertiesTreeView->setColumnWidth(1, propViewLeftColumnWidth);
}

void DBRoot::hidePropertiesView() {
	propDelegate->unbind();
	propertiesModel->clear();
	propertiesTreeView->setEnabled(false);
}

PcdTreeWidget::PcdTreeWidget(QWidget *parent) : QTreeWidget(parent) {

}

PcdTreeWidget::~PcdTreeWidget() {

}