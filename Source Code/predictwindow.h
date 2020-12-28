#ifndef PREDICTWINDOW_H
#define PREDICTWINDOW_H

#include <QDialog>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QAbstractButton>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>

namespace Ui {
class PredictWindow;
}

static const QStringList lat_p={"30.47911593", "30.52408195", "30.56904797", "30.61401398",
                              "30.65898", "30.70394602", "30.74891203", "30.79387805",
                              "30.83884407", "30.88381008"};
static const QStringList lng_p={"103.8038618", "103.8561346", "103.9084075", "103.9606803",
                              "104.0129532", "104.065226", "104.1174988", "104.1697717",
                              "104.2220445", "104.2743174"};

class pmapView;

class PredictWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PredictWindow(QWidget *parent = nullptr);
    ~PredictWindow();

private slots:
    void on_pType0_toggled(bool checked);

    void on_pType1_toggled(bool checked);

    void on_pType2_toggled(bool checked);

    void on_pCheck0_stateChanged(int arg1);

    void on_predictApply_clicked();

private:
    pmapView* pmapView;
    QSpinBox *oriSpin, *desSpin;
    QDateTimeEdit *datetime;
    QAbstractButton *pType0, *pType1, *pType2;
    QCheckBox *pCheck0, *pCheck1;
    QLabel *resultTime, *resultFee, *resultGrid, *predictInfo;
    QProgressBar *progressBar;

    int whichChecked(){
        if(pType0->isChecked())
            return 0;
        if(pType1->isChecked())
            return 1;
        if(pType2->isChecked())
            return 2;
    }

    void pTypeChanged(){
        switch(whichChecked()){
        case 0:
            oriSpin->setEnabled(true);
            desSpin->setEnabled(true);
            break;
        case 1:
            oriSpin->setEnabled(true);
            desSpin->setEnabled(false);
            break;
        case 2:
            oriSpin->setEnabled(false);
            desSpin->setEnabled(true);
            break;
        }
    }

    int isGrid_p(QString LNG, QString LAT){
            int i=0, j=0;
            for(; i<10; i++){
                if(LNG<lng_p[i])
                    break;
            }
            for(; j<10; j++){
                if(LAT<lat_p[j])
                    break;
            }
            return i*10+(j-1);
        }

    Ui::PredictWindow *ui;
};

#endif // PREDICTWINDOW_H

class pmapView:public QWidget{
    Q_OBJECT

public:
    pmapView(QWidget* parent);
    int viewType;
protected:
    void paintEvent(QPaintEvent *);
private:
    QColor heatColor(double per);

};
