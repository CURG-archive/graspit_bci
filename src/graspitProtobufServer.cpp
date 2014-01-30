#include "graspitProtobufServer.h"
#include "Drawable.pb.h"
#include "GraspitMessage.pb.h"
#include "debug.h"

GraspitProtobufConnection::GraspitProtobufConnection(QObject *parent, QTcpSocket *socket,
                                                     unsigned int maximum_len) :
    QObject(parent),
    sock(socket),
    max_len(maximum_len)
{
    connect(this,SLOT(parseMessage()), sock, SIGNAL(readyRead()));
    msg = new GraspitProtobufMessage;
}

GraspitProtobufConnection::~GraspitProtobufConnection()
{
    sock->deleteLater();
    delete msg;
}

void GraspitProtobufConnection::parseMessage()
{
  if (!readMessage()){
      DBGP("GraspitProtobufConnection::parseMessage::Failed to parse message")
      return;
  }

}

bool GraspitProtobufConnection::readMessage()
{

   QByteArray inputByteArray = sock->readAll();

   return msg->ParseFromArray(inputByteArray.data(), inputByteArray.size());
}


GraspitProtobufServer::GraspitProtobufServer(QObject * parent, unsigned int port_num) :
    QTcpServer(parent)
{
    connect(this, SIGNAL(newConnection()), this, SLOT(onConnection()));
}

void GraspitProtobufServer::onConnection()
{

    QTcpSocket *clientConnection = nextPendingConnection();

    GraspitProtobufConnection *newConnection = new GraspitProtobufConnection(NULL, clientConnection);
    connect(clientConnection, SIGNAL(disconnected()),
            newConnection, SLOT(deleteLater()));
}
