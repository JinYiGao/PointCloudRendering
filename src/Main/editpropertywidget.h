#ifndef EDITPROPERTYWIDGET_H
#define EDITPROPERTYWIDGET_H

#include <QWidget>

namespace Ui {
class EditPropertyWidget;
}

class EditPropertyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditPropertyWidget(QWidget *parent = nullptr);
    ~EditPropertyWidget();

private:
    Ui::EditPropertyWidget *ui;

signals:
	void quitEdit();
signals:
	void applyEdit(int classificationValue);
	
public slots:
	void click_cancel();
	void click_apply();
};

#endif // EDITPROPERTYWIDGET_H
