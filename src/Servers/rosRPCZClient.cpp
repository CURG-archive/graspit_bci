#include "rosRPCZClient.h"

#include "BCI/bciService.h"
#include "EGPlanner/searchState.h"


RosRPCZClient::RosRPCZClient():
    _application(),
    graspReachabilityStub(_application.create_rpc_channel("tcp://localhost:5561")),
    objectRecognitionStub(_application.create_rpc_channel("tcp://localhost:5561")),
    cameraOriginStub(_application.create_rpc_channel("tcp://localhost:5561")),
    executeGraspStub(_application.create_rpc_channel("tcp://localhost:5561"))
{

}


bool RosRPCZClient::runObjectRecognition(QObject * callbackReceiver, const char * slot)
{
    return objectRecognitionStub.sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::getCameraOrigin(QObject * callbackReceiver, const char *slot)
{
    return cameraOriginStub.sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::checkGraspReachability(const GraspPlanningState * gps, QObject * callbackReceiver, const char * slot)
{    
    graspReachabilityStub.buildRequest(gps);
    return graspReachabilityStub.sendRequest(callbackReceiver, slot);
}


bool RosRPCZClient::executeGrasp(const GraspPlanningState * gps, QObject * callbackReceiver, const char * slot)
{
  executeGraspStub.buildRequest(gps);
  return executeGraspStub.sendRequest(callbackReceiver, slot);
}




