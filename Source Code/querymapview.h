#ifndef QUERYMAPVIEW_H
#define QUERYMAPVIEW_H

#include <QDialog>
#include <QMouseEvent>
#include <QPainter>

namespace Ui {
class queryMapView;
}

//QVector<bool> oriSelected, desSelected;

class qmapView;

class queryMapView : public QDialog
{
    Q_OBJECT

public:
    explicit queryMapView(QWidget *parent = nullptr);
    ~queryMapView();

    int viewType;
public slots:
    void selectstart();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_OK_clicked();

private:
    qmapView* qmapView;

    Ui::queryMapView *ui;
signals:
    void end1();
    void end2();
};

#endif // QUERYMAPVIEW_H


class qmapView:public QWidget{
    Q_OBJECT

public:
    qmapView(QWidget* parent);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    int viewType;
protected:
    void paintEvent(QPaintEvent *);
private:
    QPoint pointStart;
    QPoint pointEnd;
    void gridUpdate();
};
