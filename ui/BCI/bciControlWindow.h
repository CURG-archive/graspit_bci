#ifndef BCICONTROLWINDOW_H
#define BCICONTROLWINDOW_H




#include <QDialog>
#include "ui_BCIControlWindowBase.h"
#include "BCI/bciService.h"
#include "debug.h"
#include "BCI/onlinePlannerController.h"
class BCIControlWindow: public QDialog, public Ui::BCIControlWindowBase
{

    Q_OBJECT

public:


    BCIControlWindow(QWidget *parent = 0 )
        :QDialog(parent)
    {
        BCIService::getInstance();
        if(OnlinePlannerController::getInstance()->thread() != this->thread())
            DBGA("OnlinePlannerController not in same thread as BCIControlWindow");


        setupUi(this);
        //this->setBackgroundColor(QColor("1F2A36"));
        //this->setBackgroundColor(QColor::fromRgb(97, 126, 158));
        this->setBackgroundColor(QColor::fromRgb(112,128,144));


    }
public slots:
    void redraw()
    {
        this->bciWorldView->redraw();
    }
};


#endif // BCICONTROLWINDOW_H
