#ifndef BCISERVICE_H
#define BCISERVICE_H

#include <QObject>

#include "BCI/worldController.h"
#include "Servers/rosRPCZClient.h"

#include "BCI/rosclient.h"
#include<Inventor/Qt/SoQtRenderArea.h>

#include <QMutex>
#include <vector>
class QImage;
class QString;

class GraspableBody;
class DrawableFrame;
class GraspPlanningState;
class BCIControlWindow;
class ControllerSceneManager;


using namespace bci_experiment;

class BCIService:public QObject
{

    Q_OBJECT

public:
    ~BCIService(){}

    void emitGoToNextState1(){emit goToNextState1();}
    void emitGoToNextState2(){emit goToNextState2();}
    void emitGoToPreviousState(){emit goToPreviousState();}


    void emitExec(){emit exec();}
    void emitNext(){emit next();}
    void emitTargetBodyChanged(GraspableBody * b){emit targetBodyChanged(b);}
    void emitResetStateMachine(){resetStateMachine();}
    void emitCursorPosition(double x, double y){emit cursorPosition(x,y);}
    void emitDrawableFrame(DrawableFrame *drawableFrame){emit drawShapes(drawableFrame);}
    void emitRotLat(){emit rotLat();}
    void emitRotLong(){emit rotLong();}

    //void emitAnalyzeGrasp(const GraspPlanningState * gps) {emit analyzeGrasp(gps); }
    //void emitAnalyzeNextGrasp() {emit analyzeNextGrasp(); }
    void emitRunObjectRecognition(){}

    void emitAnalyzeApproachDir(GraspPlanningState * gs){emit analyzeApproachDir(gs);}



    //ros server calls
    bool runObjectRetreival(QObject *callbackReceiver, const char *slot);

    bool runObjectRecognition(QObject * callbackReceiver , const char * slot);

    bool getCameraOrigin(QObject * callbackReceiver, const char * slot);

    bool checkGraspReachability(const GraspPlanningState * state,
                                            QObject * callbackReceiver,
                                            const char * slot);


    bool executeGrasp(const GraspPlanningState * gps,
                                  QObject * callbackReceiver,
                                  const char * slot);


    static BCIService* getInstance();

    void init(BCIControlWindow *bciControlWindow);
    SoQtRenderArea *bciRenderArea;

public slots:
    //called when active planner is updated
    void onPlannerUpdated(){emit plannerUpdated();}

signals:

    //tell state machine to go to next state
    void goToNextState1();
    //tell state machine to go to second option for next state
    void goToNextState2();
    //tell state machine to return to previous state;
    void goToPreviousState();

    void plannerUpdated();

    void runObjectRecognitionSignal();
    void getCameraOriginSignal();
    void checkGraspReachabilitySignal();


    //! Signal to planner to execute
    void exec();

    //! Signal to planner to go to next state
    void next();

    //! Target body changed
    void targetBodyChanged(GraspableBody * b);

    //! Reset the current state machine
    void resetStateMachine();

    //! signal to give cursor position for binary message cursor
    void cursorPosition(double x, double y);

    //! signal to have something draw 2D shapes
    void drawShapes(DrawableFrame *drawableFrame);

    //! signal to rotate the hand longitudinally around the target object
    void rotLong();

    //! signal to rotate the hand latitudinally around the target object
    void rotLat();

    //! Signal that object recognition system should be rerun
    //void runObjectRecognition();
    void sendString(const QString & s);

    void analyzeApproachDir(GraspPlanningState * gps);


protected:
    virtual bool eventFilter(QObject *obj, QEvent *evt);

private:
        //singleton pattern, single static instance of the class
        static BCIService * bciServiceInstance;

        //this is singleton, so constructor must be private.
        BCIService();

        RosClient rosClient;

        static QMutex createLock;



        ControllerSceneManager * controllerSceneManager;


};

#endif // BCISERVICE_H
