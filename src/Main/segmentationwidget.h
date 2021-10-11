#ifndef SEGMENTATIONWIDGET_H
#define SEGMENTATIONWIDGET_H

#include <QDialog>
#include <memory>

#include <PointCloud/renderingWidget.h>
#include <Main/DBRoot.h>

namespace Ui {
class SegmentationWidget;
}

class SegmentationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit SegmentationWidget(QWidget *parent, RenderWidget *glWidget, DBRoot *dbRoot);
    ~SegmentationWidget();

private:
    Ui::SegmentationWidget *ui;

	RenderWidget *glWidget = nullptr;
	DBRoot *dbRoot = nullptr;

private slots:
	void cancel_segment();
	void inPolygon_segment();
	void outPolygon_segment();
	void check_segment();
};

#endif // SEGMENTATIONWIDGET_H
