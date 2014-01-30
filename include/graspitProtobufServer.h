#ifndef GRASPITPROTOBUFSERVER_H
#define GRASPITPROTOBUFSERVER_H
#include <QtNetwork>
class GraspitProtobufMessage;

class GraspitProtobufConnection : public QObject
{
    Q_OBJECT
public:
    GraspitProtobufConnection(QObject * parent, QTcpSocket * socket,
                              unsigned int maximum_len = 5000);
    ~GraspitProtobufConnection();
private:
  QTcpSocket * sock;
  unsigned int max_len;
  GraspitProtobufMessage * msg;

private slots:
    //! Read the message and act on it.
    void parseMessage();

    //! Read the message from the wire and try to parse it.
    bool readMessage();
};

class GraspitProtobufServer : public QTcpServer
{
    Q_OBJECT
public:
    GraspitProtobufServer(QObject * parent, unsigned int port_num);
private slots:
    void onConnection();
};

#endif // GRASPITPROTOBUFSERVER_H
