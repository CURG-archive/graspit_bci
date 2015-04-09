#ifndef ONLINEPLANNERCONTROLLER_H
#define ONLINEPLANNERCONTROLLER_H

#include <QObject>
#include "BCI/utils/uiTools.h"
#include "BCI/utils/plannerTools.h"
#include "BCI/utils/worldElementTools.h"
#include "robot.h"
#include "searchState.h"
#include "grasp.h"
#include "graspitGUI.h"
#include "ivmgr.h"
#include "onLinePlanner.h"
#include "DBase/DBPlanner/sql_database_manager.h"

#include "db_manager.h"
#include "database.h"
#include "DBase/graspit_db_model.h"
#include "DBase/graspit_db_grasp.h"
#include <QMutex>

//class OnLinePlanner * op;
class GraspableBody;

class GraspPlanningState;

namespace bci_experiment {


    class OnlinePlannerController : public QThread {
    Q_OBJECT

    public:

        static OnlinePlannerController *getInstance();

        bool analyzeApproachDir();

        bool hasRecognizedObjects();

        // Needs the align method to move here
        // Needs the align method broken in to the GUI part and the
        // actual moving the hand part

        void rotateHandLat();

        void rotateHandLong();

        void drawGuides();

        void alignHand();

        void destroyGuides();

        // Highlighting functionality should move to the view controller
        // The method here should only care about setting the next target
        // and possibly emitting a signal that the target has been emitted.
        void highlightAllBodies();

        void unhighlightAllBodies();

        void highlightNextBody();


        GraspableBody *getCurrentTarget();

        GraspableBody *incrementCurrentTarget();

        void setCurrentTarget(GraspableBody *gb);

        Hand *getHand();

        Hand *getRefHand();

        Hand *getGraspDemoHand();

        const GraspPlanningState *getGrasp(int index);

        unsigned int getNumGrasps();

        void incrementGraspIndex();

        void resetGraspIndex();

        const GraspPlanningState *getCurrentGrasp();

        const GraspPlanningState *getCurrentGraspIfExists();

        int getGraspIndexByGraspId(double id);

        const GraspPlanningState *getGraspByGraspId(double id);

        const GraspPlanningState *setCurrentGraspId(double id);

        bool timedUpdateRunning;

        bool stopTimedUpdate();

        bool startTimedUpdate();

        bool toggleTimedUpdate();

        void setSceneLocked(bool locked) {
            sceneLocked = locked;
        }

        bool getSceneLocked() {
            return sceneLocked;
        }
        //void connectToPlannerUpdateSignal();


        void initializeTarget();

        void showRobots(bool show);


        void sortGrasps();

        void connectPlannerUpdate(bool enableConnection);

        bool analysisIsBlocked() { return analysisBlocked; }

        void blockGraspAnalysis(bool block) { analysisBlocked = block; }

        virtual void run();

            bool renderPending;
            OnLinePlanner * currentPlanner;

	void generateGraspSimilarity(std::vector<vec3> & approachDirs,
	                             std::vector<float> & similarity);

    private:

        static OnlinePlannerController *onlinePlannerController;
        static QMutex createLock;


        OnlinePlannerController(QObject *parent = 0);

        void initializeDbInterface();

            db_planner::SqlDatabaseManager * mDbMgr;
            GraspableBody * currentTarget;
            unsigned int currentGraspIndex;
            double currentGraspId;
            Hand * graspDemonstrationHand;
            bool setAllowedPlanningCollisions();
            bool setPlannerTargets();
            bool sceneLocked;
            bool analysisBlocked;            
    signals:

        void render();


    private slots:

        // Perform any validation or processing that should update
        // the planner or it's visualizations periodically
        void plannerTimedUpdate();

    public slots:
            bool setPlannerToRunning();
            bool setPlannerToStopped();
            bool setPlannerToPaused();
            bool setPlannerToReady();
            void analyzeNextGrasp();
            void finishedAnalysis();

            void addToWorld(const QString model_filename, const QString object_name, const QString object_pose);
            void clearObjects();
            void targetRemoved();            
            void emitRender(){if(!renderPending){ emit render(); renderPending = true;}}
    };

}
#endif // ONLINEPLANNERCONTROLLER_H
