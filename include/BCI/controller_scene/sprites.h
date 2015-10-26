
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



    void convert(const QImage& p, SoSFImage& img) const;
    void convert(const SoSFImage& p, QImage& img) const;

public:
    Sprite(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_);
    virtual ~Sprite();
    virtual void update(int state);
    bool intersects(QRectF *other_rect);

    QRectF *bounding_rect;
    SoAnnotation *sprite_root;
};



class Target: public Sprite
{

    Q_OBJECT ;

protected:
    int steps_since_last_hit;

public:
    Target(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_);
    void setHit();
    void update(int state);

signals:
        void hit();
};



class Cursor: public Sprite
{
        Q_OBJECT ;
public:
    Cursor(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_);
    void update(int state);
};

#endif
