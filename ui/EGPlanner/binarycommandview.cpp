#include "binarycommandview.h"
#include "Drawable.pb.h"
#include <algorithm>
#include <functional>
#include <QBrush>
#include <QPen>

BinaryCommandView::BinaryCommandView(QWidget * parent) :QWidget(parent)
{
    currentFrame = new DrawableFrame;
}


void BinaryCommandView::paintEvent(QPaintEvent *ev)
{

   QPainter painter(this);
   painter.setBrush(Qt::blue);
   float vwidth = width();//painter.viewport().width();
   float vheight= height();//painter.viewport().height();
   double cursorRadius = .3/7.5*vwidth;

   painter.drawEllipse(cursorX*vwidth + cursorRadius, vheight - (cursorY*vheight+cursorRadius), cursorRadius, cursorRadius);

   double rectWidth = 1.0/7.5*width();
   double rect1XOffset = 2.5/7.5*width();
   double rect2XOffset = 4.5/7.5*width();
   double rect3XOffset = 6.5/7.5*width();

   painter.drawRect(QRect(rect1XOffset,0,rectWidth,height()));
   painter.drawRect(QRect(rect2XOffset,0,rectWidth,height()));
   painter.drawRect(QRect(rect3XOffset,0,rectWidth,height()));

   QPointF boundaryPoint1 = QPointF(0,height() - 2/7.5*height());
   QPointF boundaryPoint2 = QPointF(width(),height() - 2/7.5*height());

   painter.drawLine(boundaryPoint1, boundaryPoint2);
}

void BinaryCommandView::updateCursor(double x, double y)
{
    cursorX = x/7.5;
    cursorY = y/7.5;
    update();
}

Qt::BrushStyle shapeStyleToBrushStyle(const ShapeDrawable & shape)
{
    switch(shape.fill_pattern())
    {
    case ShapeDrawable::DiagonalLines:
        return Qt::BDiagPattern;
        break;
    case ShapeDrawable::FDiagonalLines:
        return Qt::FDiagPattern;
        break;
    case ShapeDrawable::HorizontalLines:
        return Qt::HorPattern;
        break;

    case ShapeDrawable::VerticalLines:
        return Qt::VerPattern;
        break;

    case ShapeDrawable::CrossedLines:
        return Qt::CrossPattern;
        break;
    case ShapeDrawable::XCrossedLines:
        return Qt::DiagCrossPattern;
        break;
    case ShapeDrawable::SolidFill:
        return Qt::SolidPattern;
        break;
    case ShapeDrawable::NoFill:
        return Qt::NoBrush;
        break;
    }
    return Qt::SolidPattern;
}


Qt::PenStyle shapeStyleToPenStyle(const ShapeDrawable & shape)
{
    switch(shape.line_style())
    {
    case ShapeDrawable::SolidLine:
        return Qt::SolidLine;
        break;
    case ShapeDrawable::Dotted:
        return Qt::DotLine;
        break;
    case ShapeDrawable::Dashed:
        return Qt::DashLine;
        break;
    }
    return Qt::SolidLine;
}


void BinaryCommandView::drawShape(const ShapeDrawable & shape, QPainter & painter)
{
    Qt::BrushStyle fillStyle(shapeStyleToBrushStyle(shape));
    Qt::PenStyle outlineStyle(shapeStyleToPenStyle(shape));
    painter.setBrush(fillStyle);
    painter.setPen(outlineStyle);
    QPointF origin(shape.x(), shape.y());

    switch(shape.shape_type())
    {
    case ShapeDrawable::Circle:
    {
        painter.drawEllipse(origin,shape.width(), shape.height());
        break;
    }

    case ShapeDrawable::Rectangle:
    {
        QRectF rect(origin, QSizeF(shape.width(), shape.height()));
        painter.drawRect(rect);
        break;
    }
    case ShapeDrawable::Line:
    {
        QPointF lineEnding(shape.x() + shape.width(), shape.y() + shape.width());
        painter.drawLine(origin, lineEnding);
        break;
    }
    case ShapeDrawable::Text:
    {
        QFont font;
        font = painter.font();
        font.setPixelSizeFloat(shape.height());

        painter.setFont(font);
        painter.drawText(origin,QString(shape.text().c_str()));
	break;
    }
       
    }
}





void BinaryCommandView::addToObjectMap(ShapeDrawable & shape)
{
    drawable_object_map[shape.name()] = &shape;
}

void BinaryCommandView::updateDrawableObjects(ShapeDrawable & shape)
{
    std::string shape_name = shape.name();
    std::map<std::string, ShapeDrawable*>::iterator lb = drawable_object_map.lower_bound(shape.name());
    if(lb != drawable_object_map.end() && !drawable_object_map.key_comp()(shape_name, lb->first))
    {
        lb->second->CopyFrom(shape);
    }
    else
    {
        ShapeDrawable * newShape = currentFrame->add_shapes();
        newShape->CopyFrom(shape);
        drawable_object_map[shape.name()] = newShape;
    }
}


void BinaryCommandView::rebuildDrawableObjectMap()
{
    drawable_object_map.clear();
    std::for_each(currentFrame->mutable_shapes()->begin(),
                  currentFrame->mutable_shapes()->end(),
                  std::bind(&BinaryCommandView::addToObjectMap, this, std::placeholders::_1));
}



 void BinaryCommandView::updateFrame(DrawableFrame & drawing)
 {
     if(drawing.cleardrawables())
     {
         currentFrame->CopyFrom(drawing);
         rebuildDrawableObjectMap();
     }
     else
     {
       std::for_each(drawing.mutable_shapes()->begin(), drawing.mutable_shapes()->end(),
                     std::bind(&BinaryCommandView::updateDrawableObjects, this, std::placeholders::_1));
     }
 }


