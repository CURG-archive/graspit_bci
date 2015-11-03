#ifndef STOPPEDEXECUTIONVIEW_H
#define STOPPEDEXECUTIONVIEW_H

#include <QWidget>

namespace Ui {
class StoppedExecutionView;
}

class StoppedExecutionView : public QWidget
{
    Q_OBJECT
    
public:
    explicit StoppedExecutionView(QWidget *parent = 0);
    ~StoppedExecutionView();
    
private:
    Ui::StoppedExecutionView *ui;
};

#endif // STOPPEDEXECUTIONVIEW_H
