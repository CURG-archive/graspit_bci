#ifndef REQUEST_H
#define REQUEST_H
#include <QObject>
#include <QThread>
#include "rpcz/rpcz.hpp"
#include "debug.h"

class Request :public QObject
{
    Q_OBJECT
public:
    Request():_rpc(NULL){}

    bool sendRequest(QObject * callbackReceiver = NULL, const char * slot = NULL)
    {
        if(_rpc == NULL)
        {
            DBGA("Creating rpc connection");
            _rpc = new rpcz::rpc();
        }
        if(_rpc->get_status() !=  rpcz::status::INACTIVE)
        {
            DBGA("ERROR:Incorrect status");
            return false;
         }
        if(callbackReceiver)
        {
            //! This callback will be triggered by the rpcz thread, so it cannot be a direct connection
            connect(this, SIGNAL(requestComplete()), callbackReceiver, slot, Qt::QueuedConnection);

        }
           try
           {
               sendRequestImpl();
               std::cout << "REQUEST SENT" << std::endl;
               return true;
           }
           catch(rpcz::rpc_error &e)
           {
               std::cout << "RPCZ Error: " << e.what() << std::endl;
           }
           catch(const std::exception &e)
           {
               std::cout << "NON RPCZ Error: " << e.what() << std::endl;
           }
        return false;
    }

    void callback()
    {
        std::cout << "RESPONSE RECEIVED" << std::endl;
        callbackImpl();
        delete _rpc;
        _rpc = new(rpcz::rpc);
        if(this->thread() == this->thread()->currentThread())
            std::cout << "RESPONSE FROM WRONG THREAD" << std::endl;
        emit requestComplete();
    }

protected:

    virtual void sendRequestImpl() = 0;
    virtual void callbackImpl() = 0;

    rpcz::rpc *_rpc;

signals:

    void requestComplete();

};

#endif // REQUEST_H
