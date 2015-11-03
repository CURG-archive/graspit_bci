
#ifndef SPRITE_H
#define SPRITE_H


#include "qstring.h"
#include "qobject.h"
#include "QImage"

class SoAnnotation;
class SoImage;
class SoSFImage;
class SoTransform;


class Sprite: public QObject {

    Q_OBJECT ;

protected:

    double x;
    double y;
    double theta;

    QImage *qimage;
    SoImage *image;
    SoTransform * imageTran;
    QString filename;

    short renderAreaWidth;
    short renderAreaHeight;




    void convert(const QImage& p, SoSFImage& img) const;
    void convert(const SoSFImage& p, QImage& img) const;

public:
    Sprite(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_);
    virtual ~Sprite() {};
    virtual void update(int state, short renderAreaWidth, short renderAreaHeight)=0;
    bool intersects(QRectF *other_rect);


    bool valid;

    QRectF *bounding_rect;
    SoAnnotation *sprite_root;
};



class Target: public Sprite
{

    Q_OBJECT ;

protected:
    int steps_since_last_hit;

public:
    Target(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_, QString target_text);
    virtual ~Target();
    void update(int state, short renderAreaWidth, short renderAreaHeight);
    void setHit();

signals:
        void hit();
};



class Cursor: public Sprite
{
        Q_OBJECT ;
public:
    Cursor(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_);
    void setXYTheta(double x_, double y_, double theta_);
    void update(int state, short renderAreaWidth, short renderAreaHeight);

};

#endif
