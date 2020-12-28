#include "predictwindow.h"
#include "ui_predictwindow.h"
#include <QPainter>

QVector<double> gridHeat;
extern QVector<QStringList> dataTable;

PredictWindow::PredictWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PredictWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(),this->height());
    setWindowTitle("Order Prediction");
    setWindowIcon(QIcon(":/new/prefix1/FinalSources/logo256.jpg"));
    // UI setting
    {
        oriSpin=ui->oriSpin, desSpin=ui->desSpin;
        datetime=ui->datetime;
        pType0=ui->pType0, pType1=ui->pType1, pType2=ui->pType2;
        pCheck0=ui->pCheck0, pCheck1=ui->pCheck1;
        resultTime=ui->resultTime, resultFee=ui->resultFee, resultGrid=ui->resultGrid;
        predictInfo=ui->predictInfo;
        progressBar=ui->progress;
        progressBar->hide();
    }
    // Map paint
    {
        gridHeat.fill(0, 100);
        pmapView=new class pmapView(this);
        pmapView->update();
    }

}

PredictWindow::~PredictWindow()
{
    delete ui;
}

pmapView::pmapView(QWidget *parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background-image: url(:/new/prefix1/FinalSources/mapView800grid.png)");
    setParent(parent);
    resize(800,800);
    setGeometry(20,20,800,800);
}

void pmapView::paintEvent(QPaintEvent *)
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
            mapPainter.setBrush(QBrush(heatColor(gridHeat[currentGrid])));
            mapPainter.drawRect(43+72*j, 41+72*(9-i), 68, 68);
        }
    }
}

QColor pmapView::heatColor(double per)
{
    // From (128,255,0,20) to (255,64,0,128)
    // R +8 per cent, Alpha +4 per cent
    QColor color;
    if(per==0){
        color.setRgb(255, 255, 0, 0);
    }
    else if(per<0.16){
        color.setRgb(128+int(100*per*8), 255, 0, int(20+100*per*4));
    }
    else if(per<0.40){
        color.setRgb(255, 255-int(100*(per-0.16)*8), 0, int(84+100*(per-0.16)*4));
    }
    else{
        color.setRgb(255, 63, 0, 160);
    }
    return color;
}




void PredictWindow::on_pType0_toggled(bool checked)
{
    if(!checked)
        pTypeChanged();
}

void PredictWindow::on_pType1_toggled(bool checked)
{
    if(!checked)
        pTypeChanged();
}

void PredictWindow::on_pType2_toggled(bool checked)
{
    if(!checked)
        pTypeChanged();
}

void PredictWindow::on_pCheck0_stateChanged(int arg1)
{
    if(arg1==0){
        pCheck1->setCheckState(Qt::Unchecked);
        pCheck1->setEnabled(false);
        datetime->setEnabled(false);
    }
    else{
        pCheck1->setEnabled(true);
        pCheck1->setCheckState(Qt::Unchecked);
        datetime->setEnabled(true);
    }
}


void PredictWindow::on_predictApply_clicked()
{
    progressBar->setMaximum(dataTable.size());
    progressBar->setValue(0);
    progressBar->setVisible(true);

    double r_fee=-1;
    int r_time=-1;
    int r_grid=-1;

    switch (whichChecked()) {
    case 0:{
        int count=0;
        double t_fee=0;
        int t_time=0;

        QDateTime p_datetime=datetime->dateTime();
        int p_datetimeEpoch=p_datetime.toSecsSinceEpoch();
        if(pCheck0->isChecked()&&pCheck1->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt(),
                    endTime=dataTable[i][2].toInt();
                int week=datetime->dateTime().date().dayOfWeek();
                if(oriGrid==oriSpin->value() && desGrid==desSpin->value()
                        && p_datetimeEpoch%86400-1800<=startTime%86400
                        && p_datetimeEpoch%86400+1800>=startTime%86400){
                    QDateTime tmp;
                    tmp=tmp.fromSecsSinceEpoch(startTime);
                    QDate date=tmp.date();
                    if(date.dayOfWeek()==week){
                        count++;
                        t_fee+=dataTable[i][7].toDouble();
                        t_time+=(endTime-startTime);
                    }
                }
                else
                    continue;
            }
            if(count){
                r_time=t_time/count; r_fee=t_fee/count;
            }
        }
        else if(pCheck0->isChecked()&&!pCheck1->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt(),
                    endTime=dataTable[i][2].toInt();
                if(oriGrid==oriSpin->value() && desGrid==desSpin->value()
                        && p_datetimeEpoch%86400-1800<=startTime%86400
                        && p_datetimeEpoch%86400+1800>=startTime%86400){
                    count++;
                    t_fee+=dataTable[i][7].toDouble();
                    t_time+=(endTime-startTime);
                }
                else
                    continue;
            }
            if(count){
                r_time=t_time/count; r_fee=t_fee/count;
            }
        }
        else if(!pCheck0->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt(),
                    endTime=dataTable[i][2].toInt();
                if(oriGrid==oriSpin->value() && desGrid==desSpin->value()){
                    count++;
                    t_fee+=dataTable[i][7].toDouble();
                    t_time+=(endTime-startTime);
                }
                else
                    continue;
            }
            if(count){
                r_time=t_time/count; r_fee=t_fee/count;
            }
        }

        progressBar->setVisible(false);
        if(count){
            QTime resTime(0,0);
            resTime=resTime.addSecs(r_time);
            resultTime->setText("Travel Time: "+resTime.toString("hh:mm:ss"));
            resultFee->setText("Fee: "+QString::number(r_fee, 'f', 2));
            resultGrid->setText("Grid: -");
        }
        else{
            resultTime->setText("Travel Time: -");
            resultFee->setText("Fee: -");
            resultGrid->setText("Grid: -");
        }
        predictInfo->setText("Travel Time & Fee prediction based on "+QString::number(count)
                             +" records of data.");
        break;
    }
    case 1:{
        int count=0;
        gridHeat.fill(0, 100);

        QDateTime p_datetime=datetime->dateTime();
        int p_datetimeEpoch=p_datetime.toSecsSinceEpoch();
        if(pCheck0->isChecked()&&pCheck1->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt();
                int week=datetime->dateTime().date().dayOfWeek();
                if(oriGrid==oriSpin->value()&&desGrid>=0&&desGrid<100
                        && p_datetimeEpoch%86400-1800<=startTime%86400
                        && p_datetimeEpoch%86400+1800>=startTime%86400){
                    QDateTime tmp;
                    tmp=tmp.fromSecsSinceEpoch(startTime);
                    QDate date=tmp.date();
                    if(date.dayOfWeek()==week){
                        count++;
                        gridHeat[desGrid]++;
                    }
                }
                else
                    continue;
            }
        }
        else if(pCheck0->isChecked()&&!pCheck1->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt();
                if(oriGrid==oriSpin->value()&&desGrid>=0&&desGrid<100
                        && p_datetimeEpoch%86400-1800<=startTime%86400
                        && p_datetimeEpoch%86400+1800>=startTime%86400){
                    count++;
                    gridHeat[desGrid]++;
                }
                else
                    continue;
            }
        }
        else if(!pCheck0->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                if(oriGrid==oriSpin->value()&&desGrid>=0&&desGrid<100){
                    count++;
                    gridHeat[desGrid]++;
                }
                else
                    continue;
            }
        }

        progressBar->setVisible(false);
        r_grid=0;
        for(double i=0; i<100; i++){
            gridHeat[i]/=count;
            if(gridHeat[i]>gridHeat[r_grid])
                r_grid=i;
        }
        if(count){
            pmapView->update();
            resultTime->setText("Travel Time: -");
            resultFee->setText("Fee: -");
            resultGrid->setText("Grid: "+QString::number(r_grid)+"("
                                +QString::number(gridHeat[r_grid]*100, 'f', 2)+"%)");
        }
        else{
            resultTime->setText("Travel Time: -");
            resultFee->setText("Fee: -");
            resultGrid->setText("Grid: -");
        }
        predictInfo->setText("Destination prediction based on "+QString::number(count)
                             +" records of data.");
        break;
    }
    case 2:{
        int count=0;
        gridHeat.fill(0, 100);

        QDateTime p_datetime=datetime->dateTime();
        int p_datetimeEpoch=p_datetime.toSecsSinceEpoch();
        if(pCheck0->isChecked()&&pCheck1->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt();
                int week=datetime->dateTime().date().dayOfWeek();
                if(desGrid==desSpin->value()&&oriGrid>=0&&oriGrid<100
                        && p_datetimeEpoch%86400-1800<=startTime%86400
                        && p_datetimeEpoch%86400+1800>=startTime%86400){
                    QDateTime tmp;
                    tmp=tmp.fromSecsSinceEpoch(startTime);
                    QDate date=tmp.date();
                    if(date.dayOfWeek()==week){
                        count++;
                        gridHeat[oriGrid]++;
                    }
                }
                else
                    continue;
            }
        }
        else if(pCheck0->isChecked()&&!pCheck1->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                int startTime=dataTable[i][1].toInt();
                if(desGrid==desSpin->value()&&oriGrid>=0&&oriGrid<100
                        && p_datetimeEpoch%86400-1800<=startTime%86400
                        && p_datetimeEpoch%86400+1800>=startTime%86400){
                    count++;
                    gridHeat[oriGrid]++;
                }
                else
                    continue;
            }
        }
        else if(!pCheck0->isChecked()){
            for(int i=0; i<dataTable.size(); i++){
                if(i%100==0){
                    progressBar->setValue(i);
                }
                int oriGrid=isGrid_p(dataTable[i][3], dataTable[i][4]),
                    desGrid=isGrid_p(dataTable[i][5], dataTable[i][6]);
                if(desGrid==desSpin->value()&&oriGrid>=0&&oriGrid<100){
                    count++;
                    gridHeat[oriGrid]++;
                }
                else
                    continue;
            }
        }

        progressBar->setVisible(false);
        r_grid=0;
        for(double i=0; i<100; i++){
            gridHeat[i]/=count;
            if(gridHeat[i]>gridHeat[r_grid])
                r_grid=i;
        }
        if(count){
            pmapView->update();
            resultTime->setText("Travel Time: -");
            resultFee->setText("Fee: -");
            resultGrid->setText("Grid: "+QString::number(r_grid)+"("
                                +QString::number(gridHeat[r_grid]*100, 'f', 2)+"%)");
        }
        else{
            resultTime->setText("Travel Time: -");
            resultFee->setText("Fee: -");
            resultGrid->setText("Grid: -");
        }
        predictInfo->setText("Origin prediction based on "+QString::number(count)
                             +" records of data.");
        break;
    }
    }

}
