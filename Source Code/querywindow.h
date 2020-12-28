#ifndef QUERYWINDOW_H
#define QUERYWINDOW_H

#include <QDialog>
#include <QStandardItemModel>
#include <QTableView>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QLineEdit>
#include <QGroupBox>

#include "querymapview.h"

namespace Ui {
class QueryWindow;
}

extern QVector<bool> oriSelected, desSelected;
static const QStringList lat_q={"30.47911593", "30.52408195", "30.56904797", "30.61401398",
                              "30.65898", "30.70394602", "30.74891203", "30.79387805",
                              "30.83884407", "30.88381008"};
static const QStringList lng_q={"103.8038618", "103.8561346", "103.9084075", "103.9606803",
                              "104.0129532", "104.065226", "104.1174988", "104.1697717",
                              "104.2220445", "104.2743174"};

class QueryWindow : public QDialog
{
    Q_OBJECT

public:
    explicit QueryWindow(QWidget *parent = nullptr);
    ~QueryWindow();

    QStandardItemModel *queryDataView;
    QTableView *queryTableView;
    QDateTimeEdit *selectTimeFrom, *selectTimeTo;
    QSpinBox *pageSpinBox;
    QDoubleSpinBox *selectFeeFrom, *selectFeeTo;
    QLabel *selectOriNum, *selectDesNum, *showConditions, *pageMax;
    QProgressBar *progressBar;
    QLineEdit *orderID;
    QGroupBox *IDbox;

    queryMapView *oriView, *desView;

signals:
    void started1();
    void started2();
private slots:
    void on_selectOri_clicked();

    void on_selectDes_clicked();

    void set_ori_num();

    void set_des_num();

    void on_selectTimeFrom_dateTimeChanged(const QDateTime &dateTime);

    void on_selectTimeTo_dateTimeChanged(const QDateTime &dateTime);

    void on_selectFeeFrom_valueChanged(double arg1);

    void on_selectFeeTo_valueChanged(double arg1);

    void on_queryApplied_clicked();

    void on_pageSpinBox_valueChanged(int arg1);

private:
    QVector<int> queryIndex;
    int isGrid_q(QString LNG, QString LAT){
            int i=0, j=0;
            for(; i<10; i++){
                if(LNG<lng_q[i])
                    break;
            }
            for(; j<10; j++){
                if(LAT<lat_q[j])
                    break;
            }
            return i*10+(j-1);
        }
    Ui::QueryWindow *ui;
};



#endif // QUERYWINDOW_H

