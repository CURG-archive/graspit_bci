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
    this->addSelfTransition(this, SIGNAL(exited()), this, SLOT(disconnectOptionChoice()));
}


State::State( const QString& name, const QString& prefix, QState* parent )
    : QState( parent ),
      m_name( name ),
      m_prefix( prefix )
{
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
    stringOptions.clear();
    stringOptions.push_back(QString("Choice 1"));
    stringOptions.push_back(QString("Choice 2"));
    stringOptions.push_back(QString("Choice 3"));
    stringOptions.push_back(QString("Choice 4"));

    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();

    generateStringImageOptions(debug);
    for (unsigned int i = 0; i < stringOptions.size(); ++i)
    {
        imageDescriptions.push_back(stringOptions[i]);
        imageCosts.push_back(0);
     }
}


void State::generateStringImageOptions(bool debug)
{
    for (unsigned int i = 0; i < stringOptions.size(); ++i)
    {
        QImage *img = new QImage(640,480, QImage::Format_ARGB32_Premultiplied);
        QColor textColor = Qt::white;
        img->fill(qRgba(0,0,0,255));
        textColor.setAlpha(255);
        setImageText(img, stringOptions[i],
                     textColor);


        imageOptions.push_back(img);        
        if(debug)
            img->save(QString("img") + QString::number(i) + QString(".png"));
    }
}

void State::sendOptionChoice()
{
    generateImageOptions();
    float confidence = 1.0;
    if(imageOptions.size() != imageDescriptions.size())
        DBGA("imageOptions size != imageDescriptions size");

    if(imageOptions.size() != imageCosts.size())
        DBGA("imageOptions size != imageCosts size");

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
    QColor backgroundColor = Qt::black;
    int fontSize = 12;
    QFont font("Chicago", fontSize);
    painter.setPen(fontColor);
    painter.setOpacity(1.0);
    painter.setFont(font);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawText(image->rect(), Qt::AlignCenter, text);  // Draw a number on the image
}


void State::disconnectOptionChoice()
{
    this->addSelfTransition(BCIService::getInstance(), SIGNAL(optionChoice(unsigned int, float, std::vector<float> & )),
                            this, SLOT(respondOptionChoice(unsigned int, float, std::vector<float> &)));
}
