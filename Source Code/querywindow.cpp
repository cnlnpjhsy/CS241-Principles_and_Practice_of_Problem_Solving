#include "querywindow.h"
#include "ui_querywindow.h"

extern QVector<QStringList> dataTable;

QueryWindow::QueryWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(),this->height());
    setWindowTitle("Order Query");
    setWindowIcon(QIcon(":/new/prefix1/FinalSources/logo256.jpg"));
    // queryTableView
    {
        showConditions=ui->showConditions;
        pageSpinBox=ui->pageSpinBox;
        pageMax=ui->pageMax;
        progressBar=ui->progressBar;
        progressBar->setVisible(false);
        queryTableView=ui->queryTableView;
        queryDataView=new QStandardItemModel();
        queryTableView->setModel(queryDataView);

        queryDataView->setColumnCount(11);
        queryDataView->setHeaderData(0, Qt::Horizontal, "Order ID");
        queryDataView->setHeaderData(1, Qt::Horizontal, "Departure Time");
        queryDataView->setHeaderData(2, Qt::Horizontal, "End Time");
        queryDataView->setHeaderData(3, Qt::Horizontal, "Travel Time");
        queryDataView->setHeaderData(4, Qt::Horizontal, "Origin Lng.");
        queryDataView->setHeaderData(5, Qt::Horizontal, "Origin Lat.");
        queryDataView->setHeaderData(6, Qt::Horizontal, "Grid #");
        queryDataView->setHeaderData(7, Qt::Horizontal, "Destination Lng.");
        queryDataView->setHeaderData(8, Qt::Horizontal, "Destination Lat.");
        queryDataView->setHeaderData(9, Qt::Horizontal, "Grid #");
        queryDataView->setHeaderData(10, Qt::Horizontal, "Fee");
        queryTableView->setColumnWidth(0, 480);
        queryTableView->setColumnWidth(1, 250);
        queryTableView->setColumnWidth(2, 250);
        queryTableView->setColumnWidth(3, 180);
        queryTableView->setColumnWidth(4, 280);
        queryTableView->setColumnWidth(5, 280);
        queryTableView->setColumnWidth(6, 100);
        queryTableView->setColumnWidth(7, 280);
        queryTableView->setColumnWidth(8, 280);
        queryTableView->setColumnWidth(9, 100);
        queryTableView->setColumnWidth(10, 100);
        queryTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        queryTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        queryTableView->verticalHeader()->setDefaultSectionSize(15);
        queryTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        queryTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    // Time, Fee Edit
    {
        selectTimeFrom=ui->selectTimeFrom;
        selectTimeTo=ui->selectTimeTo;
        selectFeeFrom=ui->selectFeeFrom;
        selectFeeTo=ui->selectFeeTo;
    }
    // Ori & Des Edit
    {
        selectOriNum=ui->selectOriNum, selectDesNum=ui->selectDesNum;
        oriView=new queryMapView, desView=new queryMapView;
        oriView->viewType=1, desView->viewType=2;
        oriSelected.fill(true, 100);
        desSelected.fill(true, 100);
        connect(this, SIGNAL(started1()), oriView, SLOT(selectstart()));
        connect(this, SIGNAL(started2()), desView, SLOT(selectstart()));

        selectOriNum=ui->selectOriNum; selectDesNum=ui->selectDesNum;
        set_ori_num(); set_des_num();
        connect(oriView, SIGNAL(end1()), this, SLOT(set_ori_num()));
        connect(desView, SIGNAL(end2()), this, SLOT(set_des_num()));
        connect(oriView, SIGNAL(rejected()), this, SLOT(set_ori_num()));
        connect(desView, SIGNAL(rejected()), this, SLOT(set_des_num()));
    }
    // Order ID
    {
        orderID=ui->orderID;
        IDbox=ui->groupBox_5;
    }

}

QueryWindow::~QueryWindow()
{
    delete ui;
}

void QueryWindow::on_selectOri_clicked()
{
    emit started1();
    oriView->setWindowTitle("Select Origin Grids");
    oriView->exec();
}

void QueryWindow::on_selectDes_clicked()
{
    emit started2();
    desView->setWindowTitle("Select Destination Grids");
    desView->exec();
}

void QueryWindow::set_ori_num()
{
    int cnt=0;
    for(int i=0; i<100; i++){
        if(oriSelected[i])
            cnt++;
    }
    selectOriNum->setText("("+QString::number(cnt)+")");
}

void QueryWindow::set_des_num()
{
    int cnt=0;
    for(int i=0; i<100; i++){
        if(desSelected[i])
            cnt++;
    }
    selectDesNum->setText("("+QString::number(cnt)+")");
}

void QueryWindow::on_selectTimeFrom_dateTimeChanged(const QDateTime &dateTime)
{
    selectTimeTo->setMinimumDateTime(dateTime);
}

void QueryWindow::on_selectTimeTo_dateTimeChanged(const QDateTime &dateTime)
{
    selectTimeFrom->setMaximumDateTime(dateTime);
}

void QueryWindow::on_selectFeeFrom_valueChanged(double arg1)
{
    selectFeeTo->setMinimum(arg1);
}

void QueryWindow::on_selectFeeTo_valueChanged(double arg1)
{
    selectFeeFrom->setMaximum(arg1);
}

void QueryWindow::on_queryApplied_clicked()
{
    int start_sec=selectTimeFrom->dateTime().toSecsSinceEpoch(),
        end_sec=selectTimeTo->dateTime().toSecsSinceEpoch();
    // Start
    while(queryDataView->rowCount()){
        queryDataView->removeRows(0, queryDataView->rowCount());
    }
    queryIndex.clear();
    int count=0;
    showConditions->setVisible(false);
    progressBar->setMaximum(dataTable.size());
    progressBar->setValue(0);
    progressBar->setVisible(true);
    pageSpinBox->setValue(1);
    pageSpinBox->setMaximum(1);

    for(int i=0; i<dataTable.size(); i++){
        if(i%100==0){
            progressBar->setValue(i);
        }
        if(IDbox->isChecked()){
            if(!dataTable[i][0].contains(orderID->text()))
                continue;
        }
        int oriGrid=isGrid_q(dataTable[i][3], dataTable[i][4]),
            desGrid=isGrid_q(dataTable[i][5], dataTable[i][6]);
        int startTime=dataTable[i][1].toInt(),
            endTime=dataTable[i][2].toInt();
        double fee=dataTable[i][7].toDouble();

        if(oriGrid>=0 && oriGrid<=99 && desGrid>=0 && desGrid<=99
           && oriSelected[oriGrid] && desSelected[desGrid]
           && start_sec<=startTime && end_sec>=endTime
           && fee>=selectFeeFrom->value() && fee<=selectFeeTo->value()){
            queryIndex.push_back(i);
            count++;
        }
        else{
            continue;
        }
    }
    // Load 100 records
    for(int i=0; i<queryIndex.size(); i++){
        int i_count=queryDataView->rowCount();
        int index=queryIndex[i];
        for(int j=0; j<11; j++){
            if(j==0){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j]));
            }
            else if(j==1||j==2){
                QDateTime time;
                time.setSecsSinceEpoch(dataTable[index][j].toInt());
                QString timeString;
                timeString=time.toString("yyyy/MM/dd hh:mm:ss");
                queryDataView->setItem(i_count, j, new QStandardItem(timeString));
            }
            else if(j==3){
                QTime time(0,0);
                time=time.addSecs(dataTable[index][2].toInt()-dataTable[index][1].toInt());
                QString timeString;
                timeString=time.toString("hh:mm:ss");
                queryDataView->setItem(i_count, j, new QStandardItem(timeString));
            }
            else if(j==4||j==5){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j-1]));
            }
            else if(j==6){
                int grid=isGrid_q(dataTable[index][3], dataTable[index][4]);
                queryDataView->setItem(i_count, j, new QStandardItem(QString::number(grid)));
            }
            else if(j==7||j==8){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j-2]));
            }
            else if(j==9){
                int grid=isGrid_q(dataTable[index][5], dataTable[index][6]);
                queryDataView->setItem(i_count, j, new QStandardItem(QString::number(grid)));
            }
            else if(j==10){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j-3]));
            }
        }
        if(queryDataView->rowCount()==100)
            break;
    }

    progressBar->setVisible(false);
    if(IDbox->isChecked()){
        showConditions->setText(QString::number(count)+" records found under condition: Order ID, Origins"
                                +selectOriNum->text()+", Destinations"+selectDesNum->text()
                                +", "+selectTimeFrom->dateTime().toString("yyyy/MM/dd HH:mm:ss")
                                +" - "+selectTimeTo->dateTime().toString("yyyy/MM/dd HH:mm:ss")
                                +", Fee "+QString::number(selectFeeFrom->value())
                                +" - "+QString::number(selectFeeTo->value()));
    }
    else{
        showConditions->setText(QString::number(count)+" records found under condition: Origins"
                                +selectOriNum->text()+", Destinations"+selectDesNum->text()
                                +", "+selectTimeFrom->dateTime().toString("yyyy/MM/dd HH:mm:ss")
                                +" - "+selectTimeTo->dateTime().toString("yyyy/MM/dd HH:mm:ss")
                                +", Fee "+QString::number(selectFeeFrom->value())
                                +" - "+QString::number(selectFeeTo->value()));
    }
    showConditions->setVisible(true);

    if(count){
        pageSpinBox->setMaximum(1+(count-1)/100);
        pageMax->setText(" of "+QString::number(pageSpinBox->maximum()));
    }
}

void QueryWindow::on_pageSpinBox_valueChanged(int arg1)
{
    while(queryDataView->rowCount()){
        queryDataView->removeRows(0, queryDataView->rowCount());
    }
    for(int i=(arg1-1)*100; i<queryIndex.size(); i++){
        int i_count=queryDataView->rowCount();
        int index=queryIndex[i];
        for(int j=0; j<11; j++){
            if(j==0){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j]));
            }
            else if(j==1||j==2){
                QDateTime time;
                time.setSecsSinceEpoch(dataTable[index][j].toInt());
                QString timeString;
                timeString=time.toString("yyyy/MM/dd hh:mm:ss");
                queryDataView->setItem(i_count, j, new QStandardItem(timeString));
            }
            else if(j==3){
                QTime time(0,0);
                time=time.addSecs(dataTable[index][2].toInt()-dataTable[index][1].toInt());
                QString timeString;
                timeString=time.toString("hh:mm:ss");
                queryDataView->setItem(i_count, j, new QStandardItem(timeString));
            }
            else if(j==4||j==5){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j-1]));
            }
            else if(j==6){
                int grid=isGrid_q(dataTable[index][3], dataTable[index][4]);
                queryDataView->setItem(i_count, j, new QStandardItem(QString::number(grid)));
            }
            else if(j==7||j==8){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j-2]));
            }
            else if(j==9){
                int grid=isGrid_q(dataTable[index][5], dataTable[index][6]);
                queryDataView->setItem(i_count, j, new QStandardItem(QString::number(grid)));
            }
            else if(j==10){
                queryDataView->setItem(i_count, j, new QStandardItem(dataTable[index][j-3]));
            }
        }
        if(queryDataView->rowCount()==100)
            break;
    }
}
