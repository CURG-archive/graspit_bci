#ifndef ROSCLIENT_H
#define ROSCLIENT_H

#include <QObject>
#include "graspit_msgs/GetObjectInfo.h"
#include "graspit_msgs/ObjectInfo.h"
#include "moveit_trajectory_planner/LocationInfo.h"
#include "moveit_trajectory_planner/RunObjectRecognitionAction.h"
#include "moveit_trajectory_planner/CheckGraspReachabilityAction.h"
#include <actionlib/client/simple_action_client.h>

#include "ros/ros.h"
class GraspPlanningState;
class RosClient:public QObject
{

    Q_OBJECT

public:
    RosClient();

    void sendObjectRecognitionRequest();
    void executeGrasp(const GraspPlanningState * gps);
    void sendCheckGraspReachabilityRequest(const GraspPlanningState * state);
    void sendGetCameraOriginRequest();

signals:
    void addToWorld(const QString model_filename, const QString object_name, const QString object_pose);
    void clearGB();

private:

    ros::NodeHandle *n;

    ros::Publisher grasp_execution_pubisher;

    ros::ServiceClient get_camera_origin;

    actionlib::SimpleActionClient<moveit_trajectory_planner::RunObjectRecognitionAction> recognizeObjectsActionClient;
    actionlib::SimpleActionClient<moveit_trajectory_planner::CheckGraspReachabilityAction> analzeGraspReachabilityActionClient;

    void addObject(graspit_msgs::ObjectInfo object);
    void buildCheckReachabilityRequest(const GraspPlanningState * gps, moveit_trajectory_planner::CheckGraspReachabilityGoal &goal);
    void objectRecognitionCallback(const actionlib::SimpleClientGoalState& state, const moveit_trajectory_planner::RunObjectRecognitionResultConstPtr& result);
    void checkGraspReachabilityCallback(const actionlib::SimpleClientGoalState& state,  const moveit_trajectory_planner::CheckGraspReachabilityResultConstPtr& result);
};

#endif // ROSCLIENT_H
