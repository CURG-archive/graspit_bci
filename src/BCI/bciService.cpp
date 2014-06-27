#include "BCI/bciService.h"
#include "BCI/BCIStateMachine.h"

#include <QKeyEvent>

BCIService * BCIService::bciServiceInstance = NULL;

BCIService* BCIService::getInstance()
{
    if(!bciServiceInstance)
    {
        bciServiceInstance = new BCIService();
        QApplication::instance()->installEventFilter(bciServiceInstance);
    }

    return bciServiceInstance;
}

BCIService::BCIService()
{

}

void BCIService::init(BCIControlWindow *bciControlWindow)
{
    //builds and starts a qtStateMachine
    BCIStateMachine *bciStateMachine = new BCIStateMachine(bciControlWindow,this);
    bciStateMachine->start();
}

bool BCIService::eventFilter(QObject * obj, QEvent* evt)
{
    if(evt->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(evt);
        if(keyEvent->key() == Qt::Key::Key_Z)
            runObjectRecognition(NULL, NULL);

        if(keyEvent->key() == Qt::Key::Key_N)
        {
            if(OnlinePlannerController::getInstance() && world_element_tools::getWorld()->getCurrentPlanner())
            {


                OnlinePlannerController::getInstance()->incrementGraspIndex();
                OnlinePlannerController::getInstance()->getCurrentGrasp()->execute(
                            OnlinePlannerController::getInstance()->getGraspDemoHand());
                DBGA("set next graspfrom key press");
                return true;
            }
        }    
        if(keyEvent->key() == Qt::Key::Key_C)
        {
            if(OnlinePlannerController::getInstance())
            {
                if(OnlinePlannerController::getInstance()->getCurrentGrasp()){
                    DBGA("emitted check grasp from key press");
                    checkGraspReachability(OnlinePlannerController::getInstance()->getCurrentGrasp(), NULL, NULL);
                }
                else
                {
                    DBGA("couldn't find grasp to check");
                }
            }

        }
        if(keyEvent->key() == Qt::Key::Key_T)
        {
            if(OnlinePlannerController::getInstance())
            {
                OnlinePlannerController::getInstance()->toggleTimedUpdate();
                DBGA("Planner timed update running: " << OnlinePlannerController::getInstance()->timedUpdateRunning);
            }
        }
        return true;
    }
    else
    {
            return QObject::eventFilter(obj, evt);
    }
}



void BCIService::runObjectRecognition(QObject * callbackReceiver ,
                                      const char * slot)
{
    rosServer.runObjectRecognition(callbackReceiver, slot);
}

void BCIService::getCameraOrigin(QObject * callbackReceiver, const char * slot)
{
    rosServer.getCameraOrigin(callbackReceiver, slot);
}


void BCIService::checkGraspReachability(const GraspPlanningState * state,
                                        QObject * callbackReceiver,
                                        const char * slot)
{
    rosServer.checkGraspReachability(state, callbackReceiver, slot);
}
 
void BCIService::executeGrasp(const GraspPlanningState * gps, 
			      QObject * callbackReceiver,
			      const char * slot)
{
    rosServer.executeGrasp(gps);
}


