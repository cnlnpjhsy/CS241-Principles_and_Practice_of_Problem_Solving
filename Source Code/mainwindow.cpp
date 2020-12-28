#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "querywindow.h"
#include "predictwindow.h"
#include "qfiledialog.h"
#include <QMessageBox>
#include <qthread.h>
#include <QTextStream>
#include <QLabel>
#include <QPainter>
#include <QHeaderView>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QSplineSeries>

QVector<QStringList> dataTable;
int gridStatus[];
QMutex m_mutex;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(),this->height());
    setWindowTitle("Qt Ride-Hailing Order Analyzer");
    setWindowIcon(QIcon(":/new/prefix1/FinalSources/logo256.jpg"));
    // Status bar setting
    {
        ui->statusbar->addWidget(statusInfo);
        ui->statusbar->addWidget(revenueInfo);
        ui->statusbar->addWidget(progress);
        progress->hide();
        statusInfo->setText(" No files loaded. Please select a file. ");
        revenueInfo->setText("");
    }
    // Tableview setting
    {
        dataView=new QStandardItemModel();
        dataSelected=new QItemSelectionModel(dataView);
        connect(dataSelected,SIGNAL(currentChanged(QModelIndex,QModelIndex)), this,SLOT(on_currentChanged(QModelIndex,QModelIndex)));
        tableView->setParent(this);
        tableView->setGeometry(860, 900, 700, 410);
        tableView->setModel(dataView);
        tableView->setSelectionModel(dataSelected);
        tableViewHeaderSettings();

        connect(tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(on_clicked(const QModelIndex &)));
        table_pages=ui->tableSpinBox;
        tableSpinof=ui->tableSpinof;
    }
    // Mapview painting setting
    {
        for(int i=0; i<100; i++)
            gridStatus[i]=1;
        mapView=new class mapView(this);
        mapView->repaint();
    }
    // Filter: Overall setting
    {
        filter_grid_status=ui->filter_grid_status;
        filter_grid_status->setText("");
        filter_not_applied=false;
        connect(mapView, SIGNAL(filterChanged()), this, SLOT(filter_changed()));
        connect(this, SIGNAL(filterChanged()), this, SLOT(filter_changed()));

        filter_grid_selected=ui->filter_grid_selected;
        connect(mapView, SIGNAL(gridChanged()), this, SLOT(filter_grid_numbers()));
        connect(this, SIGNAL(gridChanged()), this, SLOT(filter_grid_numbers()));
    }
    // Filter: Time setting
    {
        filter_start_time=ui->filter_start_time;
        filter_end_time=ui->filter_end_time;
        applied_start_time.setDate(QDate(2016,11,1));
        applied_start_time.setTime(QTime(0, 0));
        applied_end_time.setDate(QDate(2016,11,15));
        applied_end_time.setTime(QTime(23,59));
    }
    // ChartView setting
    {
        spline=new QSplineSeries();
        bar=new QBarSeries(); pie=new QPieSeries(); barset=new QBarSet("");
        chart=new QChart();
        axis_X=new QValueAxis(), axis_Y=new QValueAxis(), axis_bar=new QBarCategoryAxis();
        chartView=ui->chartView;
        dataCombo=ui->dataCombo, graphCombo=ui->graphCombo;

        chartView->setChart(chart);
        chart->setTitleFont(QFont("微软雅黑"));
        chartView->setRenderHint(QPainter::Antialiasing);

        connect(this, SIGNAL(chartStarted(const int &)), this, SLOT(chart_start(const int &)));
        connect(this, SIGNAL(chartProgressed(const int &)), progress, SLOT(setValue(const int &)));
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}



// Open some files.
void MainWindow::on_actionLoad_files_triggered()
{
    QStringList fileNames=QFileDialog::getOpenFileNames(this, "Select one or more files", "", "csv file(*.csv)");
    if(fileNames.size()>50){
        int result=
                QMessageBox::warning(this, "Warning", "You have selected more than 50 files.\n"
                                              "It will consume giga-size of memory, and it's "
                                              "recommended to check your free memory before you continue.\n"
                                              "Are you sure to continue loading?",
                             QMessageBox::Yes | QMessageBox::Default , QMessageBox::No | QMessageBox::Escape);
        if(result==0x00010000)
            return;
    }
    auto thread1=new FileThread(fileNames);    
    connect(thread1, SIGNAL(started(const int &)), this, SLOT(progress_start(const int &)));
    connect(thread1, SIGNAL(progressed(const int &)), progress, SLOT(setValue(const int &)));
    connect(thread1, SIGNAL(ended(const int &)), this, SLOT(filein_end(const int &)));
    // Logic changed: Process as "Apply" clicked
    // No longer used:
//    connect(thread1, SIGNAL(ended(int)), thread2, SLOT(loadin_start(int)));     // Not working
//    connect(thread2, SIGNAL(started(int)), this, SLOT(loadin_start(int)));
//    connect(thread2, SIGNAL(progressed(int)), progress, SLOT(setValue(int)));
//    connect(thread2, SIGNAL(ended(int)), this, SLOT(loadin_end(int)));
    thread1->start();

    on_filter_apply_clicked();
    // Filter apply will trigger Chart apply.
}
// Close all files.
void MainWindow::on_actionClose_All_Files_triggered()
{
    statusInfo->setText(" Closing... ");
    on_gridUnselect_clicked();
    dataTable.clear();
    while(dataView->rowCount())
        dataView->removeRows(0, dataView->rowCount());
    table_pages->setMaximum(1);
    tableSpinof->setText(" of 0");
    chart->removeAllSeries();
    chart->removeAxis(axis_X);
    chart->removeAxis(axis_Y);
    chart->removeAxis(axis_bar);
    chart->setTitle("");
    Closed();
}

// Confirm filter states.
int MainWindow::gridFilterState()
{
    if(ui->filter_grid_both->isChecked())
        return 0;
    if(ui->filter_grid_ori->isChecked())
        return 1;
    if(ui->filter_grid_des->isChecked())
        return 2;
}
int MainWindow::timeFilterState()
{
    if(ui->filter_time_both->isChecked())
        return 0;
    if(ui->filter_time_start->isChecked())
        return 1;
    if(ui->filter_time_end->isChecked())
        return 2;
}
// Tableview header settings.
void MainWindow::tableViewHeaderSettings()
{
    dataView->setColumnCount(12);
    //  0   1           2       3           4       5       6       7       8           9               10          11
    // ID, Dep time, End time, Ori lng, Ori lat, Des lng, Des lat, Fee, Dep time(sec), End time(sec), Ori grid, Des grid
    dataView->setHeaderData(0, Qt::Horizontal, "Order ID");
    dataView->setHeaderData(1, Qt::Horizontal, "Departure Time");
    dataView->setHeaderData(2, Qt::Horizontal, "End Time");
    dataView->setHeaderData(3, Qt::Horizontal, "Origin Lng.");
    dataView->setHeaderData(4, Qt::Horizontal, "Origin Lat.");
    dataView->setHeaderData(5, Qt::Horizontal, "Destination Lng.");
    dataView->setHeaderData(6, Qt::Horizontal, "Destination Lat.");
    dataView->setHeaderData(7, Qt::Horizontal, "Fee");
    tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->verticalHeader()->setDefaultSectionSize(15);
    tableView->setColumnWidth(0, 150);
    tableView->setColumnWidth(1, 250);
    tableView->setColumnWidth(2, 250);
    tableView->setColumnWidth(3, 250);
    tableView->setColumnWidth(4, 250);
    tableView->setColumnWidth(5, 250);
    tableView->setColumnWidth(6, 250);
    tableView->setColumnWidth(7, 100);
    for(int i=8; i<12; i++){
        tableView->setColumnHidden(i, true);
    }
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    //connect(tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(on_tableView_clicked(const QModelIndex &)));
}


// Slots.
void MainWindow::on_clicked(const QModelIndex &current)
{
    if(!current.isValid()){
        return;
    }
    // Deselection (not work since the signal emits only when clicking different cells)
//    if(current.row()==previous.row()){
//        tableView->clearSelection();
//        gridStatus[dataView->item(current.row(), 10)->data(0).toInt()]-=10;
//        if(dataView->item(current.row(), 10)->data(0).toInt()!=dataView->item(current.row(), 11)->data(0).toInt())
//            gridStatus[dataView->item(current.row(), 11)->data(0).toInt()]-=20;
//        mapView->update();
//        return;
//    }
    // Order details...
        ui->details_start_time->setText(dataView->item(current.row(), 1)->data(0).toString());
        ui->details_end_time->setText(dataView->item(current.row(), 2)->data(0).toString());
        int spendtime=dataView->item(current.row(), 9)->data(0).toInt()
                     -dataView->item(current.row(), 8)->data(0).toInt();
        QTime time(0,0);
        time=time.addSecs(spendtime);
        ui->details_spend_time->setText(time.toString("hh:mm:ss"));
        if(dataView->item(current.row(), 10)->data(0).toInt()<0||dataView->item(current.row(), 10)->data(0).toInt()>99){
            ui->details_from_grid->setText("*Out of grids*");
        }
        else{
            ui->details_from_grid->setText(dataView->item(current.row(), 10)->data(0).toString());
        }
        if(dataView->item(current.row(), 11)->data(0).toInt()<0||dataView->item(current.row(), 10)->data(0).toInt()>99){
            ui->details_to_grid->setText("*Out of grids*");
        }
        else{
            ui->details_to_grid->setText(dataView->item(current.row(), 11)->data(0).toString());
        }
        ui->details_fee->setText(dataView->item(current.row(), 7)->data(0).toString());
    // Mapview repaint...
        for(int i=0; i<100; i++){
            gridStatus[i]%=10;
        }
        // Problem: Grid 44 (same), solved
        gridStatus[dataView->item(current.row(), 10)->data(0).toInt()]+=10;
        if(dataView->item(current.row(), 10)->data(0).toInt()!=dataView->item(current.row(), 11)->data(0).toInt())
            gridStatus[dataView->item(current.row(), 11)->data(0).toInt()]+=20;
        mapView->update();
}

void MainWindow::on_filter_start_time_dateTimeChanged(const QDateTime &dateTime)
{
    filter_end_time->setMinimumDateTime(dateTime);
    emit filter_changed();
}

void MainWindow::on_filter_end_time_dateTimeChanged(const QDateTime &dateTime)
{
    filter_start_time->setMaximumDateTime(dateTime);
    emit filter_changed();
}

void MainWindow::on_filter_grid_all_clicked()
{
    for(int i=0; i<100; i++){
        gridStatus[i]=(gridStatus[i]/10)*10+2;
    }
    mapView->update();
    emit filter_changed();
    emit gridChanged();
}

void MainWindow::on_filter_grid_clear_clicked()
{
    for(int i=0; i<100; i++){
        gridStatus[i]=(gridStatus[i]/10)*10+3;
    }
    mapView->update();
    emit filter_changed();
    emit gridChanged();
}

void MainWindow::on_tableSpinBox_valueChanged(int arg1)
{
    QStandardItemModel *Model=this->dataView;
    while(Model->rowCount()){
        Model->removeRows(0, Model->rowCount());
    }

    int count=0;
    for(int i=0; i<dataTable.size(); i++){
        if(count<(arg1-1)*100){
            if(dataTable[i][8]=="1"){
                count++;
            }
        }
        else if(count>=(arg1-1)*100&&count<arg1*100){
            if(dataTable[i][8]=="1"){
                count++;
                int i_count=Model->rowCount();
                for(int j=0; j<12; j++){
                    if(j==0||(j>=3&&j<=7)){
                        Model->setItem(i_count, j, new QStandardItem(dataTable[i][j]));
                    }
                    else if(j==1||j==2){
                        QDateTime time;
                        time.setSecsSinceEpoch(dataTable[i][j].toInt());
                        QString timeString;
                        timeString=time.toString("yyyy/MM/dd hh:mm:ss");
                        Model->setItem(i_count, j, new QStandardItem(timeString));
                    }
                    else if(j==8||j==9){
                        Model->setItem(i_count, j, new QStandardItem(dataTable[i][j-7]));
                    }
                    else if(j==10){
                        int oriGrid=isGrid(dataTable[i][3], dataTable[i][4]);
                        QString ori;
                        ori.setNum(oriGrid);
                        Model->setItem(i_count, j, new QStandardItem(ori));
                    }
                    else{
                        int desGrid=isGrid(dataTable[i][5], dataTable[i][6]);
                        QString des;
                        des.setNum(desGrid);
                        Model->setItem(i_count, j, new QStandardItem(des));
                    }
                }
            }
        }
        else
            break;
    }
}

void MainWindow::on_gridUnselect_clicked()
{
    for(int i=0; i<100; i++){
        gridStatus[i]%=10;
    }
    mapView->update();
    tableView->clearSelection();
    ui->details_start_time->setText("");
    ui->details_end_time->setText("");
    ui->details_spend_time->setText("");
    ui->details_from_grid->setText("");
    ui->details_to_grid->setText("");
    ui->details_fee->setText("");
}

void MainWindow::on_filter_grid_both_toggled(bool checked)
{
    if(!checked)
        emit filterChanged();
}
void MainWindow::on_filter_grid_ori_toggled(bool checked)
{
    if(!checked)
        emit filterChanged();
}
void MainWindow::on_filter_grid_des_toggled(bool checked)
{
    if(!checked)
        emit filterChanged();
}
void MainWindow::on_filter_time_both_toggled(bool checked)
{
    if(!checked)
        emit filterChanged();
}
void MainWindow::on_filter_time_start_toggled(bool checked)
{
    if(!checked)
        emit filterChanged();
}
void MainWindow::on_filter_time_end_toggled(bool checked)
{
    if(!checked)
        emit filterChanged();
}
// Filter Apply clicked.
void MainWindow::on_filter_apply_clicked()
{
    // Grid filter
    {
        for(int i=0; i<100; i++){
            switch (gridStatus[i]%10) {
            case 2:
                gridStatus[i]=1; break;
            case 3:
                gridStatus[i]=0; break;
            default:
                gridStatus[i]%=10; break;
            }
        }
    }
    // Time filter
    int start_sec=filter_start_time->dateTime().toSecsSinceEpoch(),
        end_sec=filter_end_time->dateTime().toSecsSinceEpoch();
    // Data re-load
    auto thread3=new LoadInTableThread(dataView, start_sec, end_sec,
                                       gridFilterState(), timeFilterState());
    connect(thread3, SIGNAL(started(const int &)), this, SLOT(loadin_start(const int &)));
    connect(thread3, SIGNAL(progressed(const int &)), progress, SLOT(setValue(const int &)));
    connect(thread3, SIGNAL(ended(const int &)), this, SLOT(loadin_end(const int &)));
    connect(thread3, SIGNAL(cleared(const int &)), this, SLOT(cleartable_start(const int &)));
    connect(thread3, SIGNAL(feed(const double &)), this, SLOT(fee_end(const double &)));
    thread3->start();
    //thread3->wait();
    // Initialize
    tableView->clearSelection();
    filter_not_applied=false;
    filter_grid_status->setText("");
    mapView->update();
    ui->details_start_time->setText("");
    ui->details_end_time->setText("");
    ui->details_from_grid->setText("");
    ui->details_to_grid->setText("");
    ui->details_spend_time->setText("");
    ui->details_fee->setText("");
    applied_start_time=filter_start_time->dateTime();
    applied_end_time=filter_end_time->dateTime();
}


// Chart Apply clicked.
void MainWindow::on_chartApplied_clicked()
{
    chart=new QChart();
    chartView->setChart(chart);
    spline=new QSplineSeries();
    bar=new QBarSeries(); pie=new QPieSeries(); barset=new QBarSet("");
    axis_X=new QValueAxis(), axis_Y=new QValueAxis(), axis_bar=new QBarCategoryAxis();

    chart->setTitleFont(QFont("微软雅黑"));
    chartView->setRenderHint(QPainter::Antialiasing);

    // DataCombo:       0               1               2
    //          Orders-TravelTime, Orders-Depatures, Orders-Fees
    // GraphCombo:  0           1       2
    //          LineChart, Histogram, PieChart
    switch (dataCombo->currentIndex()) { //All switch
    case 0:{ //DataCombo
        emit chartStarted(dataTable.size());
        QVector<int> OrdersPerMin(61, 0);
        for(int i=0; i<dataTable.size(); i++){
            if(i%100==0)
                emit chartProgressed(i);
            if(dataTable[i][8]=="0")
                continue;
            int traveltime=dataTable[i][2].toInt()-dataTable[i][1].toInt();
            if(traveltime>60*60)
                OrdersPerMin[60]++;
            else
                OrdersPerMin[traveltime/60]++;
        }
        chart_end();
        switch (graphCombo->currentIndex()) {
        case 0:{ //GraphCombo
            spline=new QSplineSeries();
            chart->addSeries(spline);

            for(int i=0; i<60; i++)
                spline->append(i, OrdersPerMin[i]);
            int Ymax=0;
            for(int i=0; i<60; i++){
                if(spline->pointsVector()[i].y()>Ymax)
                    Ymax=spline->pointsVector()[i].y();
            }
            axis_X->setRange(0, 60);
            axis_Y->setRange(0, Ymax);
            axis_X->setTitleText("Travel Time (min)");
            axis_Y->setTitleText("Orders");
            axis_X->setLabelFormat("%d");
            axis_Y->setLabelFormat("%d");
            axis_Y->applyNiceNumbers();

            chart->setAxisX(axis_X, spline);
            chart->setAxisY(axis_Y, spline);
            chart->legend()->hide();
            chart->update();
            break;
        }
        case 1:{
            bar=new QBarSeries();
            barset=new QBarSet("");
            chart->addSeries(bar);

            for(int i=0; i<6; i++){
                int sum=0;
                for(int j=i*10; j<i*10+10; j++){
                    sum+=OrdersPerMin[j];
                }
                barset->append(sum);
            }
            barset->append(OrdersPerMin[60]);
            int Ymax=0;
            for(int i=0; i<7; i++){
                if(barset->at(i)>Ymax)
                    Ymax=barset->at(i);
            }

            bar->append(barset);
            axis_bar->append("0-10");axis_bar->append("10-20");axis_bar->append("20-30");
            axis_bar->append("30-40");axis_bar->append("40-50");axis_bar->append("50-60");
            axis_bar->append(">60");
            axis_bar->setTitleText("Travel Time (min)");
            axis_Y->setRange(0, Ymax);
            axis_Y->applyNiceNumbers();
            axis_Y->setTitleText("Orders");
            axis_Y->setLabelFormat("%d");

            chart->setAxisX(axis_bar, bar);
            chart->setAxisY(axis_Y, bar);
            chart->legend()->hide();
            chart->update();
            break;
        }
        case 2:{
            pie=new QPieSeries();
            chart->addSeries(pie);

            int sum[7]={0}, total=0;
            for(int i=0; i<6; i++){
                for(int j=i*10; j<i*10+10; j++){
                    sum[i]+=OrdersPerMin[j];
                    total+=OrdersPerMin[j];
                }
            }
            sum[6]=OrdersPerMin[60];
            total+=OrdersPerMin[60];

            QStringList percent;
            for(int i=0; i<7; i++){
                double per=100*double(sum[i])/total;
                QString str=QString::number(per, 'f', 2)+"%";
                percent.append(str);
            }

            pie->append("0-10\n"+percent[0], sum[0]); pie->append("10-20\n"+percent[1], sum[1]); pie->append("20-30\n"+percent[2], sum[2]);
            pie->append("30-40\n"+percent[3], sum[3]); pie->append("40-50\n"+percent[4], sum[4]); pie->append("50-60\n"+percent[5], sum[5]);
            pie->append(">60\n"+percent[6], sum[6]);
            pie->setLabelsVisible();

            chart->setTitle("Travel Time (min)");
            chart->legend()->hide();
            chart->update();
            break;
        }
        } //GraphCombo end
        break;
    } //DataCombo end
    case 1:{ //DataCombo
        emit chartStarted(dataTable.size());
        QVector<int> OrdersPer2Hrs(180, 0);
        for(int i=0; i<dataTable.size(); i++){
            if(i%100==0)
                emit chartProgressed(i);
            if(dataTable[i][8]=="0")
                continue;
            //1477929600 is 2016.11.1 0:00
            OrdersPer2Hrs[(dataTable[i][1].toInt()-1477929600)/7200]++;
        }
        chart_end();
        switch(graphCombo->currentIndex()){
        case 0:{ //GraphCombo
            spline=new QSplineSeries();
            chart->addSeries(spline);

            for(int i=0; i<180; i++){
                QDateTime time=QDateTime::fromSecsSinceEpoch(1477929600+7200*i);
                spline->append(time.toMSecsSinceEpoch(), OrdersPer2Hrs[i]);
            }
            int Ymax=0;
            for(int i=0; i<120; i++){
                if(spline->pointsVector()[i].y()>Ymax)
                    Ymax=spline->pointsVector()[i].y();
            }

            QDateTimeAxis *axis_time = new QDateTimeAxis;
            axis_time->setFormat("MM-dd hh:mm");
            axis_time->setRange(applied_start_time, applied_end_time);
            axis_Y->setRange(0, Ymax);
            axis_time->setTitleText("Depature Time");
            axis_Y->setTitleText("Orders");
            axis_Y->setLabelFormat("%d");
            axis_Y->applyNiceNumbers();

            chart->setAxisX(axis_time, spline);
            chart->setAxisY(axis_Y, spline);
            chart->legend()->hide();
            chart->update();
            break;
        }
        case 1:{
            bar=new QBarSeries();
            QBarSet *barset0=new QBarSet("Morning");
            QBarSet *barset1=new QBarSet("Noon");
            QBarSet *barset2=new QBarSet("Afternoon");
            QBarSet *barset3=new QBarSet("Evening");
            QBarSet *barset4=new QBarSet("Midnight");
            QBarSet *barset5=new QBarSet("Before Dawn");
            chart->addSeries(bar);

            int Ymax=0;
            for(int i=0; i<15; i++) {barset5->append(OrdersPer2Hrs[12*i+1]+OrdersPer2Hrs[12*i+2]);  if(barset5->at(i)>Ymax) Ymax=barset5->at(i);}
            for(int i=0; i<15; i++) {barset0->append(OrdersPer2Hrs[12*i+3]+OrdersPer2Hrs[12*i+4]);  if(barset0->at(i)>Ymax) Ymax=barset0->at(i);}
            for(int i=0; i<15; i++) {barset1->append(OrdersPer2Hrs[12*i+5]+OrdersPer2Hrs[12*i+6]);  if(barset1->at(i)>Ymax) Ymax=barset1->at(i);}
            for(int i=0; i<15; i++) {barset2->append(OrdersPer2Hrs[12*i+7]+OrdersPer2Hrs[12*i+8]);  if(barset2->at(i)>Ymax) Ymax=barset2->at(i);}
            for(int i=0; i<15; i++) {barset3->append(OrdersPer2Hrs[12*i+9]+OrdersPer2Hrs[12*i+10]); if(barset3->at(i)>Ymax) Ymax=barset3->at(i);}
            barset4->append(OrdersPer2Hrs[0]);for(int i=1; i<14; i++) barset4->append(OrdersPer2Hrs[12*i+11]+OrdersPer2Hrs[12*i+12]);barset4->append(OrdersPer2Hrs[179]);
            for(int i=0; i<15; i++) {                                                               if(barset4->at(i)>Ymax) Ymax=barset4->at(i);}
            bar->append(barset5); bar->append(barset0); bar->append(barset1);
            bar->append(barset2); bar->append(barset3); bar->append(barset4);
            QStringList dateAxis={"11-01", "11-02", "11-03", "11-04", "11-05",
                                 "11-06", "11-07", "11-08", "11-09", "11-10",
                                 "11-11", "11-12", "11-13", "11-14", "11-15"};
            axis_bar->setCategories(dateAxis);
            axis_bar->setRange(applied_start_time.date().toString("MM-dd"),
                               applied_end_time.date().toString("MM-dd"));
            axis_bar->setTitleText("Depature Time");
            axis_bar->setLabelsAngle(75);
            axis_Y->setRange(0, Ymax);
            axis_Y->applyNiceNumbers();
            axis_Y->setTitleText("Orders");
            axis_Y->setLabelFormat("%d");

            chart->setAxisX(axis_bar, bar);
            chart->setAxisY(axis_Y, bar);
            chart->legend()->setVisible(true);
            chart->update();
            break;
        }
        case 2:{
            pie=new QPieSeries();
            chart->addSeries(pie);

            int sum[6]={0}, total=0;
            for(int i=0; i<5; i++){
                for(int j=0; j<15; j++){
                    sum[i]+=(OrdersPer2Hrs[i*2+1+12*j]+OrdersPer2Hrs[i*2+2+12*j]);
                }
            }
            sum[5]+=OrdersPer2Hrs[0]; for(int i=1; i<14; i++) sum[5]+=(OrdersPer2Hrs[12*i+11]+OrdersPer2Hrs[12*i+12]); sum[5]+=OrdersPer2Hrs[179];
            for(int i=0; i<180; i++) total+=OrdersPer2Hrs[i];

            QStringList percent;
            for(int i=0; i<6; i++){
                double per=100*double(sum[i])/total;
                QString str=QString::number(per, 'f', 2)+"%";
                percent.append(str);
            }

            pie->append("Before Dawn\n"+percent[0], sum[0]); pie->append("Morning\n"+percent[1], sum[1]); pie->append("Noon\n"+percent[2], sum[2]);
            pie->append("Afternoon\n"+percent[3], sum[3]); pie->append("Evening\n"+percent[4], sum[4]); pie->append("Midnight\n"+percent[5], sum[5]);
            pie->setLabelsVisible();

            chart->setTitle("Depature Time");
            chart->legend()->hide();
            chart->update();
            break;
        }
        } //GraphCombo end
        break;
    } //DataCombo end
    case 2:{ //DataCombo
        emit chartStarted(dataTable.size());
        QVector<double> OrdersPerFee(201, 0); //0.0, 0.1, 0.2, ..., 19.9, 20.0, >20
        double total=0;
        for(int i=0; i<dataTable.size(); i++){
            if(i%100==0)
                emit chartProgressed(i);
            if(dataTable[i][8]=="0")
                continue;
            double fee=dataTable[i][7].toDouble();
            if(fee<20){
                OrdersPerFee[int(fee*10)]+=fee;
            }
            else
                OrdersPerFee[200]+=fee;
            total+=fee;
        }
        chart_end();
        switch (graphCombo->currentIndex()) {
        case 0:{ //GraphCombo
            spline=new QSplineSeries();
            chart->addSeries(spline);

            for(double i=0; i<200; i++){
                spline->append(i/10, OrdersPerFee[i]);
            }
            double Ymax=0;
            for(int i=0; i<200; i++){
                if(spline->pointsVector()[i].y()>Ymax)
                    Ymax=spline->pointsVector()[i].y();
            }

            axis_X->setRange(0, 20);
            axis_Y->setRange(0, Ymax);
            axis_X->setTitleText("Fee");
            axis_Y->setTitleText("Orders");
            axis_X->setLabelFormat("%.1f");
            axis_Y->setLabelFormat("%d");
            axis_Y->applyNiceNumbers();

            chart->setAxisX(axis_X, spline);
            chart->setAxisY(axis_Y, spline);
            chart->legend()->hide();
            chart->update();
            break;
        }
        case 1:{
            bar=new QBarSeries();
            barset=new QBarSet("");
            chart->addSeries(bar);

            for(int i=0; i<6; i++){
                double sum=0;
                for(int j=i*30; j<i*30+30; j++){
                    sum+=OrdersPerFee[j];
                }
                barset->append(sum);
            }
            double more18=0;
            for(int i=180; i<201; i++){
                more18+=OrdersPerFee[i];
            }
            barset->append(more18);
            double Ymax=0;
            for(int i=0; i<7; i++){
                if(barset->at(i)>Ymax)
                    Ymax=barset->at(i);
            }

            bar->append(barset);
            axis_bar->append("0.0-3.0");axis_bar->append("3.0-6.0");axis_bar->append("6.0-9.0");
            axis_bar->append("9.0-12");axis_bar->append("12-15");axis_bar->append("15-18");
            axis_bar->append(">18");
            axis_bar->setTitleText("Fee");
            axis_Y->setRange(0, Ymax);
            axis_Y->applyNiceNumbers();
            axis_Y->setTitleText("Orders");
            axis_Y->setLabelFormat("%d");

            chart->setAxisX(axis_bar, bar);
            chart->setAxisY(axis_Y, bar);
            chart->legend()->hide();
            chart->update();
            break;
        }
        case 2:{
            pie=new QPieSeries();
            chart->addSeries(pie);

            double sum[7]={0};
            for(int i=0; i<6; i++){
                for(int j=i*30; j<i*30+30; j++){
                    sum[i]+=OrdersPerFee[j];
                }
            }
            double more18=0;
            for(int i=180; i<201; i++){
                more18+=OrdersPerFee[i];
            }
            sum[6]=more18;

            QStringList percent;
            for(int i=0; i<7; i++){
                double per=100*(sum[i])/total;
                QString str=QString::number(per, 'f', 2)+"%";
                percent.append(str);
            }

            pie->append("0.0-3.0\n"+percent[0], sum[0]); pie->append("3.0-6.0\n"+percent[1], sum[1]); pie->append("6.0-9.0\n"+percent[2], sum[2]);
            pie->append("9.0-12\n"+percent[3], sum[3]); pie->append("12-15\n"+percent[4], sum[4]); pie->append("15-18\n"+percent[5], sum[5]);
            pie->append(">18\n"+percent[6], sum[6]);
            pie->setLabelsVisible();

            chart->setTitle("Fee");
            chart->legend()->hide();
            chart->update();
            break;
        }
        } //GraphCombo end
        break;
    } //DataCombo end
    } //All switch end
}



// mapview.cpp (Supposed to be there)
//
// Painter initialize & settings.
void mapView::paintEvent(QPaintEvent *){
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
            switch(gridStatus[currentGrid]){
            case 0:
                mapPainter.setBrush(QBrush(QColor(195,195,195,128))); break;
            case 1:
                mapPainter.setBrush(QBrush(QColor(195,195,195,0))); break;
            case 2:
                mapPainter.setBrush(QBrush(QColor(128,255,128,128))); break;
            case 3:
                mapPainter.setBrush(QBrush(QColor(255,128,128,128))); break;
            case 10: case 11: case 12: case 13:
                mapPainter.setBrush(QBrush(QColor(128,255,255,128))); break;
            case 20: case 21: case 22: case 23:
                mapPainter.setBrush(QBrush(QColor(255,128,64,128))); break;
            }
            mapPainter.drawRect(43+72*j, 41+72*(9-i), 68, 68);
        }
    }
}
// Map initialize.
mapView::mapView(QWidget* parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background-image: url(:/new/prefix1/FinalSources/mapView800grid.png)");
    setParent(parent);
    resize(800,800);
    setGeometry(30,60,800,800);
}

void mapView::mousePressEvent(QMouseEvent *event)
{
    pointStart=event->pos();
}

void mapView::mouseReleaseEvent(QMouseEvent *event)
{
    pointEnd = event->pos();
    gridUpdate();
}
// Selecting grids.
void mapView::gridUpdate()
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

    for(int i=grid_topleft_y; i>=grid_botright_y; i--){
        for(int j=grid_topleft_x; j<=grid_botright_x; j++){
            switch (gridStatus[i*10+j]) {
            case 0: case 3: case 10: case 13: case 20: case 23:
                gridStatus[i*10+j]=(gridStatus[i*10+j]/10)*10+2; break;
            case 1: case 2: case 11: case 12: case 21: case 22:
                gridStatus[i*10+j]=(gridStatus[i*10+j]/10)*10+3; break;
            default:
                break;
            }
        }
    }
    update();
    emit filterChanged();
    emit gridChanged();
}


void MainWindow::on_actionQuery_triggered()
{
    QueryWindow query;
    query.exec();
}

void MainWindow::on_actionPrediction_triggered()
{
    PredictWindow predict;
    predict.exec();
}
