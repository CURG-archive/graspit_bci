#ifndef STATE_H
#define STATE_H

#include <QState>
#include <QSignalTransition>
#include <vector>

class BCIEvent;

class QImage;

class QColor;

class State : public QState {
Q_OBJECT

public:
    explicit State(const QString &name, QState *parent = 0);

    explicit State(const QString &name, const QString &prefix, QState *parent = 0);

    QString name() const {
        return m_name;
    }

    QString prefix() const {
        return m_prefix;
    }

    QSignalTransition *addSelfTransition(QObject *sender, const char *signal, const QObject *receiver, const char *slot);

    //Reimplement addTransition interface more safely -- explicitly disallow duplicates.
    QSignalTransition *addTransition(QObject *sender, const char *signal, QAbstractState *target);

    void addTransition(QAbstractTransition *transition);

    QAbstractTransition *addTransition(QAbstractState *target);

public slots:

    void setName(const QString &name) {
        m_name = name;
    }

    void setPrefix(const QString &prefix) {
        m_prefix = prefix;
    }

    virtual void sendOptionChoice();

    virtual void respondOptionChoice(unsigned int option, float confidence, std::vector<float> &interestLevel);

    void disconnectOptionChoice();

    void connectOptionChoice();


protected:
    virtual void onEntry(QEvent *e);

    virtual void onExit(QEvent *e);

    virtual void setImageText(QImage *image, QString &text,
            const QColor &fontColor);

    virtual void generateImageOptions(bool debug = true);

    virtual void generateStringImageOptions(bool debug = true);

    QAbstractTransition *checkForDuplicateTransitions(QAbstractTransition *transition);

protected:
    QString m_name;
    QString m_prefix;
    std::vector<QImage *> imageOptions;
    std::vector<QString> imageDescriptions;

    std::vector<QString> stringOptions;
    std::vector<float> defaultCosts;
    std::vector<float> imageCosts;
};


#endif
