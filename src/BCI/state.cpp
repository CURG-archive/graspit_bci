#include "BCI/state.h"
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QColor>
#include "BCI/bciService.h"
#include "debug.h"


State::State( const QString& name, QState* parent )
    : QState( parent ),
      m_name( name ),
      m_prefix()
{
    this->addSelfTransition(this, SIGNAL(entered()), this, SLOT(sendOptionChoice()));
    this->addSelfTransition(this, SIGNAL(exited()), this, SLOT(disconnectOptionChoice()));
}


State::State( const QString& name, const QString& prefix, QState* parent )
    : QState( parent ),
      m_name( name ),
      m_prefix( prefix )
{
    this->addSelfTransition(this, SIGNAL(entered()), this, SLOT(sendOptionChoice()));
    this->addSelfTransition(this, SIGNAL(exited()), this, SLOT(disconnectOptionChoice()));
}


void State::onEntry( QEvent* e )
{
    Q_UNUSED( e );

    // Print out the state we are entering and it's parents
    QString state = m_name;
    State* parent = dynamic_cast<State*>( parentState() );
    while ( parent != 0 )
    {
        state = parent->name() + "->" + state;
        parent = dynamic_cast<State*>( parent->parentState() );
    }
    qDebug() << m_prefix << "Entering state:" << state;
}

void State::onExit( QEvent* e )
{
    Q_UNUSED( e );

    // Print out the state we are exiting and it's parents
    QString state = m_name;
    State* parent = dynamic_cast<State*>( parentState() );
    while ( parent != 0 )
    {
        state = parent->name() + "->" + state;
        parent = dynamic_cast<State*>( parent->parentState() );
    }
    qDebug() << m_prefix << "Exiting state:" << state;
}

void State::addSelfTransition(QObject *sender, const char * signal, const QObject *receiver, const char* slot  )
{
    QSignalTransition * newTransition =
            new QSignalTransition(sender, signal);

    addTransition(newTransition);

    connect(newTransition, SIGNAL(triggered()),
            receiver, slot);
}


void State::generateImageOptions(bool debug)
{
    unsigned int numImages = 5;
    for (int i = 0; i < imageOptions.size(); ++i)
            delete(imageOptions[i]);

    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();

    for (int i = 0; i < numImages; ++i)
    {
        QImage *img = new QImage(640,480, QImage::Format_RGB32);
        QString imgText = QString("Choice: ") + QString::number(i);
        setImageText(img, imgText,
                     Qt::black);


        imageOptions.push_back(img);
        imageDescriptions.push_back(imgText);
        imageCosts.push_back(0);
        if(debug)
            img->save(QString("img") + QString::number(i) + QString(".png"));
    }
}

void State::sendOptionChoice()
{
    generateImageOptions();
    float confidence = 1.0;

    BCIService::getInstance()->sendOptionChoices(imageOptions, imageDescriptions, imageCosts, confidence);
    connect(BCIService::getInstance(), SIGNAL(optionChoice(unsigned int, float, std::vector<float> & )),
                            this, SLOT(respondOptionChoice(unsigned int, float, std::vector<float> &)));

}



void State::respondOptionChoice(unsigned int option, float confidence, std::vector<float> & interestLevel)
{
    DBGA("Choice Recieved: " << option <<" " << confidence);
}



void State::setImageText(QImage *image, QString &text,
                    const QColor & fontColor)
{
    QPainter painter(image);
    QColor backgroundColor = Qt::white;
    int fontSize = 12;
    QFont font("Chicago", fontSize);
    backgroundColor.setAlphaF(1.0);
    painter.setBackgroundColor(backgroundColor);
    painter.setPen(fontColor); // The font color comes from user select on a QColorDialog
    painter.setFont(font); // The font size comes from user input
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawText(image->size(), Qt::AlignCenter, text);  // Draw a number on the image
}


void State::disconnectOptionChoice()
{
    this->addSelfTransition(BCIService::getInstance(), SIGNAL(optionChoice(unsigned int, float, std::vector<float> & )),
                            this, SLOT(respondOptionChoice(unsigned int, float, std::vector<float> &)));
}
