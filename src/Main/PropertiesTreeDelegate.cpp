/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 20:42:38
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-31 23:32:27
 */
#include <PropertiesTreeDelegate.h>
#include <PointCloud/pointclouds.h>

 // Default separator colors
constexpr const char* SEPARATOR_STYLESHEET = "QLabel { background-color : darkGray; color : white; }";

// 
static QStandardItem* ITEM(const QString& name,
	Qt::ItemFlag additionalFlags = Qt::NoItemFlags,
	PROPERTY_ROLE role = OBJECT_NO_PROPERTY)
{
	QStandardItem* item = new QStandardItem(name);
	//flags
	item->setFlags(Qt::ItemIsEnabled | additionalFlags);
	//role (if any)
	if (role != OBJECT_NO_PROPERTY)
	{
		item->setData(role);
	}

	return item;
}

// 
static QStandardItem* CHECKABLE_ITEM(bool checkState, PROPERTY_ROLE role)
{
	QStandardItem* item = ITEM("", Qt::ItemIsUserCheckable, role);
	//check state
	item->setCheckState(checkState ? Qt::Checked : Qt::Unchecked);

	return item;
}

static QStandardItem* PERSISTENT_EDITOR(PROPERTY_ROLE role)
{
	return ITEM(QString(), Qt::ItemIsEditable, role);
}

PropertiesTreeDelegate::PropertiesTreeDelegate(QStandardItemModel* _model, QAbstractItemView* _view, QObject *parent) : QStyledItemDelegate(parent) {
	this->m_model = _model;
	this->m_view = _view;
}

PropertiesTreeDelegate::~PropertiesTreeDelegate() {
	unbind();
}

void PropertiesTreeDelegate::unbind() {
	this->currentPcd = nullptr;
	if (m_model) {
		m_model->disconnect(this);
	}
}

std::shared_ptr<PointCloud> PropertiesTreeDelegate::getCurrentPcd() {
	return this->currentPcd;
}

void PropertiesTreeDelegate::fillModel(std::shared_ptr<PointCloud> &pcd) {
	if (pcd == nullptr) {
		return;
	}

	this->currentPcd = pcd;
	if (m_model) {
		m_model->removeRows(0, m_model->rowCount());
		m_model->setColumnCount(2);
		m_model->setHeaderData(0, Qt::Horizontal, tr("Property"));
		m_model->setHeaderData(1, Qt::Horizontal, tr("State/Value"));
	}

	fillWithPcd(pcd);
}

void PropertiesTreeDelegate::addSeparator(const QString& title) {
	if (m_model) {
		QStandardItem* leftItem = new QStandardItem(/*title*/);
		QStandardItem* rightItem = new QStandardItem(/*title*/);
		leftItem->setData(TREE_VIEW_HEADER);
		leftItem->setAccessibleDescription(title);
		rightItem->setData(TREE_VIEW_HEADER);
		QList<QStandardItem*> rowItems{ leftItem, rightItem };
		m_model->appendRow(rowItems);

		if (m_view != nullptr)
		{
			m_view->openPersistentEditor(m_model->index(m_model->rowCount() - 1, 0));
			m_view->openPersistentEditor(m_model->index(m_model->rowCount() - 1, 1));
		}
	}
}

void PropertiesTreeDelegate::appendRow(QStandardItem* leftItem, QStandardItem* rightItem, bool openPersistentEditor/*=false*/) {
	if (m_model)
	{
		//append row
		QList<QStandardItem*> rowItems{ leftItem, rightItem };

		m_model->appendRow(rowItems);

		//the persistent editor (if any) is always the right one!
		if (openPersistentEditor && (m_view != nullptr))
		{
			m_view->openPersistentEditor(m_model->index(m_model->rowCount() - 1, 1));
		}
	}
}

void PropertiesTreeDelegate::fillWithPcd(std::shared_ptr<PointCloud> &pcd) {
	if (!pcd || !m_model) {
		return;
	}

	addSeparator(tr("Point Cloud"));

	// name
	appendRow(ITEM(tr("Name")), ITEM(pcd->name, Qt::ItemIsEditable, OBJECT_NAME));
	// points
	appendRow(ITEM(tr("PointsNum")), ITEM(QString::number(pcd->points_num)));
	// visibility
	appendRow(ITEM(tr("Visible")), CHECKABLE_ITEM(pcd->getVisible(), OBJECT_VISIBILITY));
	// colors
	appendRow(ITEM(tr("Colors")), PERSISTENT_EDITOR(OBJECT_COLOR), true);
	// point size
	appendRow(ITEM(tr("Point Size")), PERSISTENT_EDITOR(OBJECT_CLOUD_POINT_SIZE), true);

	connect(m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateItem(QStandardItem*)));
}

bool PropertiesTreeDelegate::isWideEditor(int itemData) const {
	switch (itemData) {
	case TREE_VIEW_HEADER:
		return true;
	default:
		break;
	}
	return false;
}

// 创建控件
QWidget* PropertiesTreeDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &option,
	const QModelIndex &index) const {

	if (!m_model || !currentPcd) {
		return nullptr;
	}

	QStandardItem *item = m_model->itemFromIndex(index);

	if (!item || !item->data().isValid()) {
		return nullptr;
	}

	int itemData = item->data().toInt(); // 
	if (item->column() == 0 && !isWideEditor(itemData)) {
		//on the first column, only editors spanning on 2 columns are allowed
		return nullptr;
	}
	
	QWidget *outputWidget = nullptr;
	//
	switch (itemData) 
	{
		case OBJECT_COLOR: {
			QComboBox *comboBox = new QComboBox(parent);
			comboBox->blockSignals(true);
			comboBox->addItem(tr("None"));
			if (currentPcd) {
				comboBox->addItem(tr("RGB"));
				comboBox->addItem(tr("Label"));
				comboBox->addItem(tr("Idensity"));
			}
			comboBox->blockSignals(false);

			connect(comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(colorChanged(QString)));

			outputWidget = comboBox;
		}
		break;
		case OBJECT_CLOUD_POINT_SIZE: {
			QSpinBox *spinBox = new QSpinBox(parent);
			spinBox->setRange(1, 10);
			spinBox->setSingleStep(1);
			spinBox->setMinimumHeight(25);

			connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(changePointSize(int)));

			outputWidget = spinBox;
		}
		break;
		case TREE_VIEW_HEADER:{
			QLabel *headerLabel = new QLabel(parent);
			headerLabel->setStyleSheet(SEPARATOR_STYLESHEET); // 
			outputWidget = headerLabel;
		}
		break;
		default:
			return QStyledItemDelegate::createEditor(parent, option, index);
	}

	if (outputWidget)
	{
		outputWidget->setFocusPolicy(Qt::StrongFocus); //Qt doc: << The returned editor widget should have Qt::StrongFocus >>
	}

	return outputWidget;
}

void SetSpinBoxValue(QWidget *editor, int value, bool keyboardTracking = false)
{
	QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
	if (!spinBox)
	{
		assert(false);
		return;
	}
	spinBox->setKeyboardTracking(keyboardTracking);
	spinBox->setValue(value);
}

void SetComboBoxIndex(QWidget *editor, int index) {
	QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
	if (!comboBox)
	{
		assert(false);
		return;
	}
	assert(index < 0 || index < comboBox->maxCount());
	comboBox->setCurrentIndex(index);
}

// 设置数据
void PropertiesTreeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	if (!m_model || !currentPcd) {
		return;
	}
	QStandardItem* item = m_model->itemFromIndex(index);

	if (!item || !item->data().isValid() || (item->column() == 0 && !isWideEditor(item->data().toInt())))
		return;

	switch (item->data().toInt()) {
	case TREE_VIEW_HEADER:
	{
		QLabel* label = qobject_cast<QLabel*>(editor);
		if (label)
		{
			label->setText(item->accessibleDescription());
		}
		break;
	}
	case OBJECT_CLOUD_POINT_SIZE: 
	{
		SetSpinBoxValue(editor, currentPcd->getPointSize());
		break;
	}
	case OBJECT_COLOR:
	{
		switch (currentPcd->getAttributeMode()) {
		case NO_COLOR:
		{
			SetComboBoxIndex(editor, 0);
			break;
		}
		case FROM_RBG:
		{
			SetComboBoxIndex(editor, 1);
			break;
		}
		case FROM_Label:
		{
			SetComboBoxIndex(editor, 2);
			break;
		}
		case FROM_INTENSITY:
		{
			SetComboBoxIndex(editor, 3);
			break;
		}
		default:
			break;
		}
	}
	default:
		QStyledItemDelegate::setEditorData(editor, index);
		break;
	}
}

// slots
void PropertiesTreeDelegate::colorChanged(QString type) {
	std::cout << "color changed" << std::endl;
	if (type == "None") {
		currentPcd->setAttributeMode(NO_COLOR);
	}
	else if (type == "RGB") {
		currentPcd->setAttributeMode(FROM_RBG);
	}
	else if (type == "Label") {
		currentPcd->setAttributeMode(FROM_Label);
	}
	else if (type == "Idensity") {
		currentPcd->setAttributeMode(FROM_INTENSITY);
	}
}

void PropertiesTreeDelegate::changePointSize(int size) {
	currentPcd->setPointSize(size);
}

//
void PropertiesTreeDelegate::updateItem(QStandardItem *item) {
	if (!currentPcd || item->column() == 0 || !item->data().isValid()) {
		return;
	}
	switch (item->data().toInt()) {
	case OBJECT_VISIBILITY:
	{
		currentPcd->setVisible(item->checkState() == Qt::Checked);
		break;
	}
	default:
		break;
	}
}