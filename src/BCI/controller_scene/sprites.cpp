#include "BCI/controller_scene/sprites.h"
#include "BCI/controller_scene/controller_scene_mgr.h"

#include <Inventor/nodes/SoImage.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoMaterial.h>
#include "BCI/bciService.h"

#include "qpainter.h"
#include <iostream>

#define PI 3.14159265

#define MIN_X  -1.4
#define MAX_X  1.25
#define MIN_Y  -1.015
#define MAX_Y  0.85

Cursor::Cursor(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_)
    : Sprite(control_scene_separator, filename, x_, y_, theta_)
{
    valid=true;
}

void Cursor::setXYTheta(double x_, double y_, double theta_)
{
    if (x_ < MIN_X)
    {
        x_ = MIN_X;
    }
    else if (x_ > MAX_X)
    {
        x_ = MAX_X;
    }

    if (y_ < MIN_Y)
    {
        y_ = MIN_Y;
    }
    else if (y_ > MAX_Y)
    {
        y_ = MAX_Y;
    }

    x = x_;
    y = y_;
    theta = theta_;

    QImage rotated_img;
    QTransform rotating;
    rotating.rotate(theta);

//    image->horAlignment = SoImage::CENTER;
//    image->vertAlignment = SoImage::CENTER;

    int orig_height = qimage->height();
    int orig_width = qimage->width();

    QImage scaled_img;
    scaled_img = qimage->scaled(int(1.0/20 *renderAreaHeight), int(1.0/20*renderAreaWidth), Qt::KeepAspectRatio);
    rotated_img = scaled_img.transformed(rotating);

    int height = scaled_img.height();
    int width = scaled_img.width();

    convert(rotated_img.copy(QRect((rotated_img.width()-width)/2,
                                   (rotated_img.height()-height)/2,
                                   width,
                                   height)), image->image);

    bounding_rect = new QRectF(x,y,  orig_height/500.0, orig_width/500.0);
    imageTran->translation.setValue(SbVec3f(x,y,0));

}


void Cursor::update(int state, short renderAreaWidth_, short renderAreaHeight_)
{
    if (!valid)
    {
        return;
    }

    double new_x = x;
    double new_y = y;
    double new_theta = theta;

    renderAreaHeight = renderAreaHeight_;
    renderAreaWidth = renderAreaWidth_;

    if(state == CursorState::MOVING_SLOW)
    {
        new_x = x + 0.001 * cos(theta * PI / 180.0 );
        new_y = y -= 0.001 * sin(theta * PI / 180.0 );
    }
    else if(state == CursorState::MOVING_FAST )//move fast
    {
        new_x += 0.01 * cos(theta * PI / 180.0 );
        new_y -= 0.01 * sin(theta * PI / 180.0 );
    }
    else //rotate
    {
        new_theta = theta + 4;
        if (new_theta > 360)
        {
            new_theta = 0;
        }
    }

    setXYTheta(new_x, new_y, new_theta);

}

Target::Target(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_, QString target_text)
    : Sprite(control_scene_separator, filename, x_, y_, theta_),
      steps_since_last_hit(500)
{    
    QPainter p(qimage);
    p.setPen(QPen(Qt::lightGray));
    p.setFont(QFont("Times", 28, QFont::Bold));
    p.drawText(qimage->rect(), Qt::AlignCenter, target_text.toStdString().c_str());
    convert(*qimage, image->image);
}

Target::~Target()
{
    delete qimage;
    std::cout << "Deleting Target" << std::endl;
}

void Target::setHit()
{
    if (steps_since_last_hit > 5)
    {
        steps_since_last_hit = 0;
    }
}

void Target::update(int state, short renderAreaWidth_, short renderAreaHeight_)
{
    if(!valid)
    {
        return;
    }
    renderAreaHeight = renderAreaHeight_;
    renderAreaWidth = renderAreaWidth_;

    if(steps_since_last_hit == 5)
    {
        steps_since_last_hit += 1;
        emit(hit());
        return;
    }
    else if(steps_since_last_hit < 5)
    {
        steps_since_last_hit += 1;
        image->filename = "sprites/target_hit.png";
    }

    int orig_height = qimage->height();
    int orig_width = qimage->width();

    int scaled_height = int(1.0/10 *renderAreaHeight);
    int scaled_width = int(1.0/10*renderAreaWidth);
    scaled_height = std::max(scaled_height, 5);
    scaled_width = std::max(scaled_width, 5);
    QSize scaled_size = QSize(scaled_height, scaled_width);
    QImage scaled_img = qimage->scaled(scaled_size, Qt::KeepAspectRatio);

    int height = scaled_img.height();
    int width = scaled_img.width();

    convert(scaled_img.copy(QRect((scaled_img.width()-width)/2,
                                   (scaled_img.height()-height)/2,
                                   width,
                                   height)), image->image);

    bounding_rect = new QRectF(x,y,  orig_height/500.0, orig_width/500.0);
}


Sprite::Sprite(SoAnnotation *control_scene_separator,
               QString filename_,
               double x_,
               double y_,
               double theta_)
    : x(x_),
      y(y_),
      theta(theta_),
      filename(filename_)
{
    valid=true;
    sprite_root = new SoAnnotation;
    control_scene_separator->addChild(sprite_root);

    qimage = new QImage(filename);
    image = new SoImage;
    image->filename = filename_.toStdString().c_str();

    bounding_rect = new QRectF(x,y,qimage->height()/500.0, qimage->width()/500.0);

    imageTran = new SoTransform;
    imageTran->translation.setValue(SbVec3f(x,y,0));

    SoMaterial * imagematerial = new SoMaterial;
    imagematerial->transparency = 0.5;

    sprite_root->addChild((imageTran));
    sprite_root->addChild(imagematerial);
    sprite_root->addChild(image);

    renderAreaWidth= 100;
    renderAreaHeight = 100;

}


bool Sprite::intersects(QRectF *other_rect)
{

    double other_center_x = other_rect->x() + (other_rect->width())/2.0 ;
    double other_center_y = other_rect->y() + (other_rect->width())/2.0 ;
    double other_radius = (other_rect->width())/2.0;

    if (bounding_rect->isNull() || !this->valid)
    {
        return false;
    }

    double my_center_x = bounding_rect->x() + (bounding_rect->width())/2.0 ;
    double my_center_y = bounding_rect->y() + (bounding_rect->width())/2.0 ;
    double my_radius = (bounding_rect->width())/2.0;

    double dist = sqrt(pow(my_center_x-other_center_x,2) + pow(my_center_y-other_center_y,2));

    double min_dist_for_collision = other_radius + my_radius;

    if ((min_dist_for_collision) > dist)
    {
        return true;
    }
    return false;
}


void Sprite::convert(const QImage& p, SoSFImage& img) const
{
    SoDB::writelock();
    SbVec2s size;
    size[0] = p.width();
    size[1] = p.height();

    int buffersize = p.numBytes();
    int numcomponents = 0;
    QVector<QRgb> table = p.colorTable();
    if (!table.isEmpty()) {
        if (p.hasAlphaChannel()) {
            if (p.allGray())
                numcomponents = 2;
            else
                numcomponents = 4;
        }
        else {
            if (p.allGray())
                numcomponents = 1;
            else
                numcomponents = 3;
        }
    }
    else {
        numcomponents = buffersize / (size[0] * size[1]);
    }
    // allocate image data
    img.setValue(size, numcomponents, NULL);

    unsigned char * bytes = img.startEditing(size, numcomponents);

    int width  = (int)size[0];
    int height = (int)size[1];

    for (int y = 0; y < height; y++)
    {
        unsigned char * line = &bytes[width*numcomponents*(height-(y+1))];
        for (int x = 0; x < width; x++)
        {
            QRgb rgb = p.pixel(x,y);
            switch (numcomponents)
            {
            default:
                break;
            case 1:
                line[0] = qGray( rgb );
                break;
            case 2:
                line[0] = qGray( rgb );
                line[1] = qAlpha( rgb );
                break;
            case 3:
                line[0] = qRed( rgb );
                line[1] = qGreen( rgb );
                line[2] = qBlue( rgb );
                break;
            case 4:
                line[0] = qRed( rgb );
                line[1] = qGreen( rgb );
                line[2] = qBlue( rgb );
                line[3] = qAlpha( rgb );
                break;
            }

            line += numcomponents;
        }
    }

    img.finishEditing();
    SoDB::writeunlock();
}

void Sprite::convert(const SoSFImage& p, QImage& img) const
{
    SbVec2s size;
    int numcomponents;

    const unsigned char * bytes = p.getValue(size, numcomponents);

    int width  = (int)size[0];
    int height = (int)size[1];

    img = QImage(width, height, QImage::Format_RGB32);
    QRgb * bits = (QRgb*) img.bits();

    for (int y = 0; y < height; y++)
    {
        const unsigned char * line = &bytes[width*numcomponents*(height-(y+1))];
        for (int x = 0; x < width; x++)
        {
            switch (numcomponents)
            {
            default:
            case 1:
                *bits++ = qRgb(line[0], line[0], line[0]);
                break;
            case 2:
                *bits++ = qRgba(line[0], line[0], line[0], line[1]);
                break;
            case 3:
                *bits++ = qRgb(line[0], line[1], line[2]);
                break;
            case 4:
                *bits++ = qRgba(line[0], line[1], line[2], line[3]);
                break;
            }

            line += numcomponents;
        }
    }
}
