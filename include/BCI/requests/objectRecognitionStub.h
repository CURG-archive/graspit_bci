#include "BCI/requests/requestStub.h"
#include "run_recognition.rpcz.h"
#include "run_recognition.pb.h"
#include "rpcz/rpc_channel.hpp"
#include "rpcz/rpc_channel_impl.hpp"
#include <QString>

using namespace graspit_rpcz;
class Body;

class ObjectRecognitionStub: public Request
{
public:
    ObjectRecognitionStub(rpcz::rpc_channel * channel);

protected:
    virtual void sendRequestImpl();
    virtual void callback();

private:
    graspit_rpcz::ObjectRecognitionService_Stub objectRecognition_stub;
    graspit_rpcz::ObjectRecognitionRequest request;
    graspit_rpcz::ObjectRecognitionResponse response;

    void addObject(GraspableObject object);
    Body * addToWorld(const QString & model_filename);


};