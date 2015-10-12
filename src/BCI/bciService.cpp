#include "BCI/bciService.h"
#include "BCI/BCIStateMachine.h"

#include <cstdlib>
#include "graspit_msgs/GetObjectInfo.h"
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
    //rosRPCZServer.runObjectRecognition(callbackReceiver, slot);
    rosClient.runObjectRecognition();
}

void BCIService::getCameraOrigin(QObject * callbackReceiver, const char * slot)
{
    rosRPCZServer.getCameraOrigin(callbackReceiver, slot);
}


void BCIService::checkGraspReachability(const GraspPlanningState * state,
                                        QObject * callbackReceiver,
                                        const char * slot)
{
    rosRPCZServer.checkGraspReachability(state, callbackReceiver, slot);
}
 
void BCIService::executeGrasp(const GraspPlanningState * gps, 
			      QObject * callbackReceiver,
			      const char * slot)
{
    rosRPCZServer.executeGrasp(gps);
}


