#include "BCI/bciService.h"
#include "BCI/BCIStateMachine.h"
#include "BCI/controller_scene/controller_scene_mgr.h"

#include "debug.h"

#include <cstdlib>
#include "graspit_msgs/GetObjectInfo.h"
#include <QKeyEvent>
#include <QMutex>


BCIService * BCIService::bciServiceInstance = NULL;
QMutex BCIService::createLock;

BCIService* BCIService::getInstance()
{
    if(createLock.locked())
    {
        assert(0);
        return NULL;
    }
    QMutexLocker lock(&createLock);
    if(!bciServiceInstance)
    {
        DBGA("Building a BCIService")
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
    bciRenderArea = bciControlWindow->bciWorldView->renderArea;
    //builds and starts a qtStateMachine
    BCIStateMachine *bciStateMachine = new BCIStateMachine(bciControlWindow,this);    
    connect(this, SIGNAL(plannerUpdated()), bciControlWindow, SLOT(redraw()));
    connect(OnlinePlannerController::getInstance(), SIGNAL(render()), bciControlWindow, SLOT(redraw()));
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

bool BCIService::runObjectRetreival(QObject * callbackReceiver,
                                    const char * slot)
{
    DBGA("BCIService::runObjectRetreival");
    rosClient.sendObjectRecognitionRequest();
    return true;
}

bool BCIService::runObjectRecognition(QObject * callbackReceiver ,
                                      const char * slot)
{
    DBGA("BCIService::runObjectRecognition");
    rosClient.sendObjectRecognitionRequest();
    return true;
}


bool BCIService::getCameraOrigin(QObject * callbackReceiver, const char * slot)
{
    rosClient.sendGetCameraOriginRequest();
    return true;
}


bool BCIService::checkGraspReachability(const GraspPlanningState * state,
                                        QObject * callbackReceiver,
                                        const char * slot)
{
    rosClient.sendCheckGraspReachabilityRequest(state);
    return true;
}
 
bool BCIService::executeGrasp(const GraspPlanningState * gps,
			      QObject * callbackReceiver,
			      const char * slot)
{
    rosClient.executeGrasp(gps);
    return true;
}

