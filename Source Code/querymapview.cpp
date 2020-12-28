#include "querymapview.h"
#include "ui_querymapview.h"

QVector<bool> oriSelected, desSelected;

queryMapView::queryMapView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::queryMapView)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(),this->height());
    setWindowIcon(QIcon(":/new/prefix1/FinalSources/logo256.jpg"));
    qmapView=new class qmapView(this);
    qmapView->update();
}

queryMapView::~queryMapView()
{
    delete ui;
}

void queryMapView::selectstart()
{
    qmapView->viewType=viewType;
    qmapView->update();
}

qmapView::qmapView(QWidget *parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background-image: url(:/new/prefix1/FinalSources/mapView800grid.png)");
    setParent(parent);
    resize(800,800);
    setGeometry(20,20,800,800);
}

void qmapView::paintEvent(QPaintEvent *)
{
    QPainter mapPainter(this);
    // Initialize the whole map
    mapPainter.setPen(QPen(QColor(0,0,0,0)));
    mapPainter.setBrush(QBrush(QColor(195,195,195,128)));
    mapPainter.drawRect(0, 0, 800, 35);
    mapPainter.drawRect(0, 35, 37, 728);
    mapPainter.drawRect(764, 35, 36, 728);
    mapPainter.drawRect(0, 763, 800, 37);
    // Then paint all
    for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
            int currentGrid=i*10+j;
            switch(viewType){
            case 1:{
                if(oriSelected[currentGrid])
                    mapPainter.setBrush(QBrush(QColor(128,255,255,128)));
                else
                    mapPainter.setBrush(QBrush(QColor(195,195,195,0)));
                break;
            }
            case 2:{
                if(desSelected[currentGrid])
                    mapPainter.setBrush(QBrush(QColor(255,128,64,128)));
                else
                    mapPainter.setBrush(QBrush(QColor(195,195,195,0)));
                break;
            }
            }
            mapPainter.drawRect(43+72*j, 41+72*(9-i), 68, 68);
        }
    }
}

void qmapView::gridUpdate()
{
    int x1=pointStart.x(), x2=pointEnd.x();
    int y1=pointStart.y(), y2=pointEnd.y();
    int x_topleft, y_topleft, x_botright, y_botright;
    if(x1<x2){
        x_topleft=x1, x_botright=x2;
    }
    else{
        x_topleft=x2, x_botright=x1;
    }
    if(y1<y2){
        y_topleft=y1, y_botright=y2;
    }
    else{
        y_topleft=y2, y_botright=y1;
    }

    int grid_topleft_x=(x_topleft-40)/72;
    int grid_botright_x=(x_botright-40)/72;
    int grid_topleft_y=9-(y_topleft-37)/72;
    int grid_botright_y=9-(y_botright-37)/72;
    if(grid_botright_x>9)
        grid_botright_x=9;
    if(grid_botright_y<0)
        grid_botright_y=0;

    if(viewType==1){
        for(int i=grid_topleft_y; i>=grid_botright_y; i--){
            for(int j=grid_topleft_x; j<=grid_botright_x; j++){
                (oriSelected[i*10+j])? oriSelected[i*10+j]=false : oriSelected[i*10+j]=true;
            }
        }
    }
    else if(viewType==2){
        for(int i=grid_topleft_y; i>=grid_botright_y; i--){
            for(int j=grid_topleft_x; j<=grid_botright_x; j++){
                (desSelected[i*10+j])? desSelected[i*10+j]=false : desSelected[i*10+j]=true;
            }
        }
    }
    update();
}

void qmapView::mousePressEvent(QMouseEvent *event)
{
    pointStart=event->pos();
}

void qmapView::mouseReleaseEvent(QMouseEvent *event)
{
    pointEnd = event->pos();
    gridUpdate();
}

void queryMapView::on_pushButton_clicked()
{
    switch (viewType) {
    case 1:
        oriSelected.fill(true, 100);
        qmapView->update();
        break;
    case 2:
        desSelected.fill(true, 100);
        qmapView->update();
        break;
    }
}

void queryMapView::on_pushButton_2_clicked()
{
    switch (viewType) {
    case 1:
        oriSelected.fill(false, 100);
        qmapView->update();
        break;
    case 2:
        desSelected.fill(false, 100);
        qmapView->update();
        break;
    }
}

void queryMapView::on_OK_clicked()
{
    switch (viewType) {
    case 1:
        emit end1();
        break;
    case 2:
        emit end2();
        break;
    }
    close();
}
