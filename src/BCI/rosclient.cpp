#include "BCI/rosclient.h"
#include "get_camera_origin/GetCameraOrigin.h"
#include "graspitGUI.h"
#include "ivmgr.h"
#include "world.h"
#include "graspit_msgs/GetObjectInfo.h"
#include "std_msgs/Empty.h"
#include "std_srvs/Empty.h"
#include "debug.h"
#include "graspit_msgs/AnalyzePose.h"
#include "graspit_msgs/Grasp.h"
#include "moveit_trajectory_planner/LocationInfo.h"

#include <Inventor/nodes/SoCamera.h>
#include "graspit_msgs/Grasp.h"
#include "BCI/onlinePlannerController.h"


using bci_experiment::OnlinePlannerController;
using namespace moveit_trajectory_planner;

RosClient::RosClient()
     :recognizeObjectsActionClient("recognize_objections_action", true),
      analzeGraspReachabilityActionClient("analyze_grasp_action", true)
{
    n = new ros::NodeHandle("");

    grasp_execution_pubisher = n->advertise<graspit_msgs::Grasp>("/graspit/grasps", 5);

    get_camera_origin=n->serviceClient<get_camera_origin::GetCameraOrigin>("get_camera_origin");

    ROS_INFO("Ros Client waiting for ros action servers to come up...");
    if(recognizeObjectsActionClient.waitForServer(ros::Duration(5)))
    {
       ROS_INFO("Object Recognition Action Server is Running");
    }
    else
    {
       ROS_INFO("Object Recognition Action Server is Not Running");
    }

    if(analzeGraspReachabilityActionClient.waitForServer(ros::Duration(5)))
    {
       ROS_INFO("analzeGraspReachabilityAction Server is Running");
    }
    else
    {
       ROS_INFO("analzeGraspReachabilityAction Server is Not Running");
    }

    connect(
        this,
        SIGNAL(clearGB()),
        OnlinePlannerController::getInstance(),
        SLOT(clearObjects()));

    connect(
        this,
        SIGNAL(addToWorld(const QString, const QString, const QString )),
        OnlinePlannerController::getInstance(),
        SLOT(addToWorld(const QString , const QString, const QString )));

}


void RosClient::sendObjectRecognitionRequest()
{

    RunObjectRecognitionGoal goal;

    recognizeObjectsActionClient.sendGoal(goal,  boost::bind(&RosClient::objectRecognitionCallback, this, _1, _2),
                actionlib::SimpleActionClient<moveit_trajectory_planner::RunObjectRecognitionAction>::SimpleActiveCallback(),
                actionlib::SimpleActionClient<moveit_trajectory_planner::RunObjectRecognitionAction>::SimpleFeedbackCallback());

}

void RosClient::objectRecognitionCallback(const actionlib::SimpleClientGoalState& state,
                       const moveit_trajectory_planner::RunObjectRecognitionResultConstPtr& result)
{
    emit clearGB();
    while(graspItGUI->getIVmgr()->getWorld()->getNumGB())
    {
        usleep(10000);
    }

    std::for_each(result->object_info.begin(),
                  result->object_info.end(),
                  boost::bind(&RosClient::addObject, this, _1));

    ROS_INFO("Sucessfully Finished runObjectRecognition Request");
}


void RosClient::addObject(graspit_msgs::ObjectInfo object)
{

    QString  modelName(QString::fromStdString(object.model_name) + ".xml");
    QString objectName(QString::fromStdString(object.object_name));

    ROS_INFO("Adding Model %s" ,object.model_name.c_str());

    transf object_pose = transf(
                Quaternion(
                    object.object_pose.orientation.w,
                    object.object_pose.orientation.x,
                    object.object_pose.orientation.y,
                    object.object_pose.orientation.z),
                vec3(
                    object.object_pose.position.x*1000.0,
                    object.object_pose.position.y*1000.0,
                    object.object_pose.position.z*1000.0
                    ));

    std::stringstream s;
    s << object_pose;
    QString stringPose(QString::fromStdString(s.str()));

    emit addToWorld(modelName, objectName, stringPose);
}

void RosClient::executeGrasp(const GraspPlanningState * gps)
{
    graspit_msgs::Grasp grasp;
    grasp.object_name = gps->getObject()->getName().toStdString().c_str();
    grasp.epsilon_quality=gps->getEpsilonQuality();
    grasp.volume_quality=gps->getVolume();
    grasp.grasp_id=gps->getAttribute("graspId");

    for(int i = 0; i < gps->getQualityMeasures()->size(); i++)
    {
        grasp.secondary_qualities.push_back(gps->getQualityMeasures()->at(i));
    }

    double dof[gps->getHand()->getNumDOF()];
    const_cast<GraspPlanningState *>(gps)->getPosture()->getHandDOF(dof);
    for(int i = 0; i < gps->getHand()->getNumDOF(); ++i)
    {
       grasp.final_grasp_dof.push_back(dof[i]);
       grasp.pre_grasp_dof.push_back(dof[i]);
    }

    transf finalHandTransform = gps->readPosition()->getCoreTran();

    float tx = finalHandTransform.translation().x() / 1000;
    float ty = finalHandTransform.translation().y() / 1000;
    float tz = finalHandTransform.translation().z() / 1000;
    float rw = finalHandTransform.rotation().w;
    float rx = finalHandTransform.rotation().x;
    float ry = finalHandTransform.rotation().y;
    float rz = finalHandTransform.rotation().z;

    grasp.final_grasp_pose.position.x=tx ;
    grasp.final_grasp_pose.position.y=ty;
    grasp.final_grasp_pose.position.z=tz;
    grasp.final_grasp_pose.orientation.w=rw;
    grasp.final_grasp_pose.orientation.x=rx;
    grasp.final_grasp_pose.orientation.y=ry;
    grasp.final_grasp_pose.orientation.z=rz;

    double moveDist = -50.0;
    transf pregraspHandTransform = (translate_transf(vec3(0,0,moveDist) * gps->getHand()->getApproachTran()) * gps->readPosition()->getCoreTran());
    tx = pregraspHandTransform.translation().x() / 1000;
    ty = pregraspHandTransform.translation().y() / 1000;
    tz = pregraspHandTransform.translation().z() / 1000;
    rw = pregraspHandTransform.rotation().w;
    rx = pregraspHandTransform.rotation().x;
    ry = pregraspHandTransform.rotation().y;
    rz = pregraspHandTransform.rotation().z;

    grasp.pre_grasp_pose.position.x=tx;
    grasp.pre_grasp_pose.position.y=ty;
    grasp.pre_grasp_pose.position.z=tz;
    grasp.pre_grasp_pose.orientation.w=rw;
    grasp.pre_grasp_pose.orientation.x=rx;
    grasp.pre_grasp_pose.orientation.y=ry;
    grasp.pre_grasp_pose.orientation.z=rz;

    grasp_execution_pubisher.publish(grasp);
}


void RosClient::buildCheckReachabilityRequest(const GraspPlanningState * gps, moveit_trajectory_planner::CheckGraspReachabilityGoal &goal)
{
    goal.grasp.object_name = gps->getObject()->getName().toStdString().c_str();
    goal.grasp.epsilon_quality=gps->getEpsilonQuality();
    goal.grasp.volume_quality=gps->getVolume();
    goal.grasp.grasp_id=gps->getAttribute("graspId");

    for(int i = 0; i < gps->getQualityMeasures()->size(); i++)
    {
        goal.grasp.secondary_qualities.push_back(gps->getQualityMeasures()->at(i));
    }

    double dof[gps->getHand()->getNumDOF()];
    const_cast<GraspPlanningState *>(gps)->getPosture()->getHandDOF(dof);
    for(int i = 0; i < gps->getHand()->getNumDOF(); ++i)
    {
       goal.grasp.final_grasp_dof.push_back(dof[i]);
       goal.grasp.pre_grasp_dof.push_back(dof[i]);
    }

    transf finalHandTransform = gps->readPosition()->getCoreTran();

    float tx = finalHandTransform.translation().x() / 1000;
    float ty = finalHandTransform.translation().y() / 1000;
    float tz = finalHandTransform.translation().z() / 1000;
    float rw = finalHandTransform.rotation().w;
    float rx = finalHandTransform.rotation().x;
    float ry = finalHandTransform.rotation().y;
    float rz = finalHandTransform.rotation().z;

    goal.grasp.final_grasp_pose.position.x=tx ;
    goal.grasp.final_grasp_pose.position.y=ty;
    goal.grasp.final_grasp_pose.position.z=tz;
    goal.grasp.final_grasp_pose.orientation.w=rw;
    goal.grasp.final_grasp_pose.orientation.x=rx;
    goal.grasp.final_grasp_pose.orientation.y=ry;
    goal.grasp.final_grasp_pose.orientation.z=rz;

    double moveDist = -50.0;
    transf pregraspHandTransform = (translate_transf(vec3(0,0,moveDist) * gps->getHand()->getApproachTran()) * gps->readPosition()->getCoreTran());
    tx = pregraspHandTransform.translation().x()/ 1000;
    ty = pregraspHandTransform.translation().y()/ 1000;
    tz = pregraspHandTransform.translation().z()/ 1000;
    rw = pregraspHandTransform.rotation().w;
    rx = pregraspHandTransform.rotation().x;
    ry = pregraspHandTransform.rotation().y;
    rz = pregraspHandTransform.rotation().z;

    goal.grasp.pre_grasp_pose.position.x=tx;
    goal.grasp.pre_grasp_pose.position.y=ty;
    goal.grasp.pre_grasp_pose.position.z=tz;
    goal.grasp.pre_grasp_pose.orientation.w=rw;
    goal.grasp.pre_grasp_pose.orientation.x=rx;
    goal.grasp.pre_grasp_pose.orientation.y=ry;
    goal.grasp.pre_grasp_pose.orientation.z=rz;
}


void RosClient::sendCheckGraspReachabilityRequest(const GraspPlanningState * gps)
{

    CheckGraspReachabilityGoal goal;
    buildCheckReachabilityRequest(gps, goal);

    analzeGraspReachabilityActionClient.sendGoal(goal,  boost::bind(&RosClient::checkGraspReachabilityCallback, this, _1, _2),
                actionlib::SimpleActionClient<moveit_trajectory_planner::CheckGraspReachabilityAction>::SimpleActiveCallback(),
                actionlib::SimpleActionClient<moveit_trajectory_planner::CheckGraspReachabilityAction>::SimpleFeedbackCallback());

}

void RosClient::checkGraspReachabilityCallback(const actionlib::SimpleClientGoalState& state,
                                               const moveit_trajectory_planner::CheckGraspReachabilityResultConstPtr& result)
{

    if(bci_experiment::OnlinePlannerController::getInstance()->analysisIsBlocked())
        return;

    QString attribute = QString("testResult");

    EGPlanner* currentWorldPlanner = graspItGUI->getIVmgr()->getWorld()->getCurrentPlanner();
    QMutexLocker lock(&currentWorldPlanner->mListAttributeMutex);
    for(int i = 0; i < currentWorldPlanner->getListSize(); i++ )
    {
        const GraspPlanningState * gps = currentWorldPlanner->getGrasp(i);
        if (gps->getAttribute("graspId") == result->grasp_id)
        {

            int reachabilityScore = 0;
            if(result->isPossible)
            {
                reachabilityScore = 1;
            }
            else
            {
               reachabilityScore = -1;
            }

            currentWorldPlanner->setGraspAttribute(i, attribute, reachabilityScore);
            std::cout << "SetGraspAttribute graspId " << result->grasp_id << " attributeString " << reachabilityScore << "\n";
            break;
        }
    }
}

void RosClient::sendGetCameraOriginRequest()
{
    get_camera_origin::GetCameraOrigin srv;
    if ( get_camera_origin.call(srv))
    {
       ROS_INFO("Successfully received Camera Origin.");
       float x = srv.response.camera_pose.position.x;
       float y = srv.response.camera_pose.position.y;
       float z = srv.response.camera_pose.position.z;
       transf t(mat3::IDENTITY, 1.5*vec3(x,y,z));
       graspItGUI->getIVmgr()->setCameraTransf(t);
       graspItGUI->getIVmgr()->getViewer()->getCamera()->pointAt(SbVec3f(0,0,0), SbVec3f(0,0,1));
    }
    else
    {
      ROS_ERROR("Failed to get Camera Origin.");
    }
}
