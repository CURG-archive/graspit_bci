
#ifndef BINARYCOMMANDVIEW_H
#define BINARYCOMMANDVIEW_H

#include <QPainter>
#include<QWidget>
#include <map>
#include <string>
class DrawableFrame;
class ShapeDrawable;

class BinaryCommandView: public QWidget
{
    Q_OBJECT
public:
    BinaryCommandView(QWidget * parent);

    virtual void updateCursor(double, double);
public slots:
    virtual void updateFrame(DrawableFrame & drawing);

protected:
    void updateDrawableObjects(ShapeDrawable & shape);
    void rebuildDrawableObjectMap();
    void addToObjectMap(ShapeDrawable & shape);
    void drawShape(const ShapeDrawable & shape, QPainter & painter);
    double cursorX;
    double cursorY;
    DrawableFrame * currentFrame;
    std::map<std::string, ShapeDrawable *> drawable_object_map;
    void paintEvent(QPaintEvent*);
signals:
};

#endif // BINARYCOMMANDVIEW_H

