#ifndef ROSRPCZCLIENT_H
#define ROSRPCZCLIENT_H

#include "rpcz/rpcz.hpp"

#include <boost/shared_ptr.hpp>
#include "BCI/requests/objectRecognitionStub.h"
#include "BCI/requests/cameraOriginStub.h"
#include "BCI/requests/graspReachabilityStub.h"
#include "BCI/requests/executeGraspStub.h"
#include "BCI/requests/optionSelectionStub.h"

using namespace graspit_rpcz;
class GraspPlanningState;
class RosRPCZClient
{

public:
    RosRPCZClient();
    ~RosRPCZClient();
    bool runObjectRetrieval(QObject * callbackReceiver = NULL, const char * slot = NULL);
    bool runObjectRecognition(QObject * callbackReceiver = NULL, const char * slot = NULL);
    bool getCameraOrigin(QObject * callbackReceiver = NULL, const char * slot = NULL);
    bool checkGraspReachability(const GraspPlanningState * gps, QObject * callbackReceiver = NULL, const char * slot = NULL);
    bool executeGrasp(const GraspPlanningState * gps, QObject * callbackReceiver = NULL, const char * slot = NULL);
    bool sendOptionChoices(std::vector<QImage*> & imageOptions, std::vector<QString> &imageDescriptions, const std::vector<float> & imageCosts,
                           float minimumConfidence,  QObject * callbackReceiver = NULL, const char * slot = NULL);
private:
    rpcz::application * _application;

    GraspReachabilityStub * graspReachabilityStub;
    ObjectRecognitionStub *objectRecognitionStub;
    ObjectRecognitionStub *objectRetrievalStub;
    CameraOriginStub *cameraOriginStub;
    ExecuteGraspStub *executeGraspStub;
    OptionSelectionStub *optionSelectionStub;
    bool _connected;
};

#endif // ROSRPCZCLIENT_H
