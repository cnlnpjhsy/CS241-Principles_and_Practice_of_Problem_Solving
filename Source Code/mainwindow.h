#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QThread>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QProgressBar>
#include <QString>
#include <QPixmap>
#include <QStandardItemModel>
#include <QDateTime>
#include <QPainter>
#include <QTableView>
#include <QMouseEvent>
#include <QDateTimeEdit>
#include <QMetaType>
#include <QSpinBox>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QSplineSeries>
#include <QMutex>
#include <QMutexLocker>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// After fileThread, dataTable has
// StringLists, and each list includes the fields.
extern QVector<QStringList> dataTable;
// Grid Status & Geo
// 0=inactive         1=active       2=about to active          3=about to inactive
// 10=ori,inactive    11=ori,active  12=ori,about to active     13=ori,about to inactive
// 20=des,inactive    21=des,active  22=des,about to active     23=des,about to inactive
extern int gridStatus[100];
extern QMutex m_mutex;

class mapView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLabel *statusInfo=new QLabel; QString before;
    QLabel *revenueInfo=new QLabel;
    QProgressBar *progress=new QProgressBar;
    QStandardItemModel *dataView;
    QItemSelectionModel *dataSelected;
    //QAbstractItemModel *dataView;
    QTableView *tableView=new QTableView();
    mapView* mapView;
    QDateTimeEdit *filter_start_time, *filter_end_time;
    QLabel *filter_grid_status, *filter_grid_selected, *tableSpinof;
    QSpinBox *table_pages;

    QSplineSeries *spline;
    QBarSeries *bar; QBarSet *barset; QPieSeries *pie;
    QChart *chart;
    QValueAxis *axis_X;
    QValueAxis *axis_Y;
    QBarCategoryAxis *axis_bar;
    QChartView *chartView;
    QComboBox *dataCombo, *graphCombo;

    void paintAll();    // No use
    int gridFilterState();
    int timeFilterState();
    void tableViewHeaderSettings();

public slots:
    void progress_start(const int &num){
        progress->show();
        progress->setValue(0);
        progress->setMaximum(num*32767);
        statusInfo->setText(" Loading files... ");
        revenueInfo->hide();
    }
    void filein_end(const int &num){
        QString info;
        info=info.number(num);
        info+=" records loaded. ";
        statusInfo->setText(info);
        progress->hide();
    }
    void loadin_start(const int &num){
        progress->show();
        progress->setValue(0);
        progress->setMaximum(num);
        statusInfo->setText(" Loading data... ");
        table_pages->setValue(1);
        table_pages->setMaximum(1);
        revenueInfo->hide();
    }
    void loadin_end(const int &num){
        QString info;
        info=info.number(num);
        info+=" records loaded.   ";
        statusInfo->setText(info);
        before=statusInfo->text();
        progress->hide();
        if(num){
            table_pages->setMaximum(1+(num-1)/100);
            tableSpinof->setText(" of "+QString::number(table_pages->maximum()));
        }
        on_chartApplied_clicked();
    }
    void fee_end(const double &num){
        revenueInfo->setHidden(false);
        revenueInfo->setText(QString::number(num, 'f', 2)+" total revenue under this filter.   ");
    }
    void cleartable_start(const int &num){
        progress->show();
        progress->setValue(0);
        progress->setMaximum(num);
        statusInfo->setText(" Clearing data... ");
    }
    void filter_changed(){
        if(filter_not_applied==false){
            filter_not_applied=true;
            filter_grid_status->setText("Changes not applied.");
        }
    }
    void filter_grid_numbers(){
        int numbers=0;
        for(int i=0; i<100; i++){
            if(gridStatus[i]%10==1||gridStatus[i]%10==2)
                numbers++;
        }
        filter_grid_selected->setText(" "+QString::number(numbers)+" grids selected.");
    }
    void chart_start(const int &num){
        progress->show();
        progress->setValue(0);
        progress->setMaximum(num);
        statusInfo->setText(" Repainting charts... ");
    }
    void chart_end(){
        progress->hide();
        statusInfo->setText(before);
    }
private slots:
    void on_actionLoad_files_triggered();

    void on_actionClose_All_Files_triggered();

    void Closed(){
        statusInfo->setText(" No files loaded. Please select a file.   ");
        revenueInfo->setText("");
    }

    void on_clicked(const QModelIndex &current);

    void on_filter_start_time_dateTimeChanged(const QDateTime &dateTime);

    void on_filter_end_time_dateTimeChanged(const QDateTime &dateTime);

    void on_filter_grid_all_clicked();

    void on_filter_grid_clear_clicked();

    void on_filter_apply_clicked();

    void on_filter_grid_both_toggled(bool checked);

    void on_filter_grid_ori_toggled(bool checked);

    void on_filter_grid_des_toggled(bool checked);

    void on_filter_time_both_toggled(bool checked);

    void on_filter_time_start_toggled(bool checked);

    void on_filter_time_end_toggled(bool checked);

    void on_tableSpinBox_valueChanged(int arg1);

    void on_gridUnselect_clicked();

    void on_chartApplied_clicked();

    void on_actionQuery_triggered();

    void on_actionPrediction_triggered();

private:
    Ui::MainWindow *ui;
    bool filter_not_applied;
    QDateTime applied_start_time, applied_end_time;
signals:
    void chartStarted(const int &);
    void chartProgressed(const int &);
    void filterChanged();
    void gridChanged();
};
#endif // MAINWINDOW_H

class mapView:public QWidget{
    Q_OBJECT

public:
    mapView(QWidget* parent);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
protected:
    void paintEvent(QPaintEvent *);
private:
    QPoint pointStart;
    QPoint pointEnd;
    void gridUpdate();
signals:
    void filterChanged();
    void gridChanged();
};

static const QStringList lat={"30.47911593", "30.52408195", "30.56904797", "30.61401398",
                              "30.65898", "30.70394602", "30.74891203", "30.79387805",
                              "30.83884407", "30.88381008"};
static const QStringList lng={"103.8038618", "103.8561346", "103.9084075", "103.9606803",
                              "104.0129532", "104.065226", "104.1174988", "104.1697717",
                              "104.2220445", "104.2743174"};

static int isGrid(QString LNG, QString LAT){
        int i=0, j=0;
        for(; i<10; i++){
            if(LNG<lng[i])
                break;
        }
        for(; j<10; j++){
            if(LAT<lat[j])
                break;
        }
        return i*10+(j-1);
    }


class FileThread:public QThread{
    Q_OBJECT

public:
    QStringList fileNames;
    FileThread(QStringList fileNames): fileNames(fileNames){}
signals:
    void ended(const int &);
    void started(const int &);
    void progressed(const int &);
private:
    int record_count=0;
    void run() override{
        QMutexLocker lock(&m_mutex);
        dataTable.clear();
        record_count=0;
        emit started(fileNames.size());
        for(int i=0; i<fileNames.size(); i++){
            QFile file(fileNames[i]);
            if(!file.open(QIODevice::ReadOnly | QFile::Text))
                {
                    //QMessageBox::warning(this, "Warning", "Cannot open file!\nError:"+file.errorString());
                    return;
                }
            QTextStream in(&file);
            QStringList records=in.readAll().split("\n");

            for(int j=1; j<records.size(); j++){
                if(records[j]=="")
                    continue;
                QStringList cols=records[j].split(",");
                // Col 8
                cols.append("0");
                dataTable.push_back(cols);                  // Already separated.
                record_count++;
                if(record_count%100==0){
                    emit progressed(record_count);
                }
            }
            file.close();
        }
        emit ended(record_count);
    }
};

class LoadInTableThread:public QThread{
    Q_OBJECT

public:
    QStandardItemModel *Model;
    int count=0;                // Not working
    int start_sec, end_sec, gridMode, timeMode;
    LoadInTableThread(QStandardItemModel* model, int start, int end, int grid_m, int time_m):
        Model(model), start_sec(start), end_sec(end), gridMode(grid_m), timeMode(time_m) {}
public slots:
    void loadin_start(const int &num){ // Not working
        count=num;
    }
signals:
    void started(const int &);
    void progressed(const int &);
    void ended(const int &);
    void cleared(const int &);
    void feed(const double &);
private:
    void run() override{
        QMutexLocker lock(&m_mutex);
//        int clearcount=Model->rowCount();
//        emit cleared(clearcount);
        while(Model->rowCount()){
            Model->removeRows(0, Model->rowCount());
//            if(Model->rowCount()%100==0)
//                emit progressed(clearcount-Model->rowCount());
        }

        // Filter working...
        emit started(dataTable.size());
        int filter_count=0;
        double fee=0;
        for(int i=0; i<dataTable.size(); i++){
            if(i%100==0)
                emit progressed(i);
            int oriGrid=isGrid(dataTable[i][3], dataTable[i][4]),
                desGrid=isGrid(dataTable[i][5], dataTable[i][6]);
            int startTime=dataTable[i][1].toInt(),
                endTime=dataTable[i][2].toInt();
            QString ori, des;
            ori.setNum(oriGrid), des.setNum(desGrid);

            dataTable[i][8]="0";
            switch (gridMode) {
            case 0:
                if(oriGrid>99||desGrid>99||gridStatus[oriGrid]==0||gridStatus[desGrid]==0) continue; else break;
            case 1:
                if(oriGrid>99||gridStatus[oriGrid]==0) continue; else break;
            case 2:
                if(desGrid>99||gridStatus[desGrid]==0) continue; else break;
            }
            switch (timeMode) {
            case 0:
                if(startTime>=start_sec&&endTime<=end_sec) break; else continue;
            case 1:
                if(startTime>=start_sec&&startTime<=end_sec) break; else continue;
            case 2:
                if(endTime>=start_sec&&endTime<=end_sec) break; else continue;
            }
            dataTable[i][8]="1";
            filter_count++;
            fee+=dataTable[i][7].toDouble();
        }
        emit feed(fee);
        // Load 100 records at most into tableView...
        for(int i=0; i<dataTable.size(); i++){
            if(dataTable[i][8]=="0")
                continue;
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
            if(Model->rowCount()==100)
                break;
        }
        emit ended(filter_count);
    }
};
