#include "BCI/controller_scene/sprites.h"
#include "BCI/controller_scene/controller_scene_mgr.h"

#include <Inventor/nodes/SoImage.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoMaterial.h>
#include "BCI/bciService.h"

#include <iostream>

#define PI 3.14159265

#define MIN_X  -1.4
#define MAX_X  1.25
#define MIN_Y  -1.015
#define MAX_Y  0.85

Cursor::Cursor(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_)
    : Sprite(control_scene_separator, filename, x_, y_, theta_)
{

}

void Cursor::update(int state, float pixels_per_inch, short renderAreaWidth, short renderAreaHeight)
{
    if(state == CursorState::MOVING_SLOW)
    {
        x += 0.001 * cos(theta * PI / 180.0 );
        y -= 0.001 * sin(theta * PI / 180.0 );
    }
    else if(state == CursorState::MOVING_FAST )//move fast
    {
        x += 0.01 * cos(theta * PI / 180.0 );
        y -= 0.01 * sin(theta * PI / 180.0 );
    }
    else //rotate
    {
        theta += 4;
        if (theta > 360)
        {
            theta = 0;
        }
    }

    if (x < MIN_X)
    {
        x = MIN_X;
    }
    else if (x > MAX_X)
    {
        x = MAX_X;
    }

    if (y < MIN_Y)
    {
        y = MIN_Y;
    }
    else if (y > MAX_Y)
    {
        y = MAX_Y;
    }
    QImage rotated_img;
    QTransform rotating;
    rotating.rotate(theta);
    int orig_height = qimage->height();
    int orig_width = qimage->width();

    //rotated_img = rotated_img.scaled(width/renderAreaWidth, height/ renderAreaHeight, Qt::KeepAspectRatio);


    std::cout << "Pixels Per Inch" << std::endl;
    std::cout << pixels_per_inch << std::endl;
    std::cout << "Render Width" << std::endl;
    std::cout << renderAreaWidth << std::endl;
    std::cout << "Render Height" << std::endl;
    std::cout << renderAreaHeight << std::endl;

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
    //bounding_rect = new QRectF(x,y,  height/500.0, width/500.0);
    //bounding_rect = new QRectF(x,y,  height/(1.0*orig_height)*orig_height*0.00348, width/(1.0*orig_width)*orig_width*0.00348);
    imageTran->translation.setValue(SbVec3f(x,y,0));

//    std::cout << "x: ";
//    std::cout << x;
//    std::cout << "y: ";
//    std::cout << y << std::endl;

}

Target::Target(SoAnnotation * control_scene_separator, QString filename, double x_, double y_, double theta_)
    : Sprite(control_scene_separator, filename, x_, y_, theta_),
      steps_since_last_hit(500)
{

}

void Target::setHit()
{
    if (steps_since_last_hit > 5)
    {
        steps_since_last_hit = 0;
    }
}

void Target::update(int state, float pixels_per_inch, short renderAreaWidth, short renderAreaHeight)
{
    if(steps_since_last_hit == 5)
    {
        steps_since_last_hit += 1;
        emit(hit());
    }
    else if(steps_since_last_hit < 5)
    {
        steps_since_last_hit += 1;
        image->filename = "sprites/target_hit.png";
    }
    int orig_height = qimage->height();
    int orig_width = qimage->width();

    QImage scaled_img;
    scaled_img = qimage->scaled(int(1.0/10 *renderAreaHeight), int(1.0/10*renderAreaWidth), Qt::KeepAspectRatio);

    int height = scaled_img.height();
    int width = scaled_img.width();

    std::cout << orig_width << std::endl;
    std::cout << width << std::endl;
    convert(scaled_img.copy(QRect((scaled_img.width()-width)/2,
                                   (scaled_img.height()-height)/2,
                                   width,
                                   height)), image->image);


    //bounding_rect = new QRectF(x,y,  height/500.0, width/500.0);
    bounding_rect = new QRectF(x,y,  orig_height/500.0, orig_width/500.0);
//    else if(steps_since_last_hit == 6)
//    {
//        image->filename = filename.toStdString().c_str();
//    }
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
    sprite_root = new SoAnnotation;
    control_scene_separator->addChild(sprite_root);

    qimage = new QImage;
    image = new SoImage;
    image->filename = filename_.toStdString().c_str();

    convert(image->image, *qimage);

    float pixels_per_inch = 0.1;// BCIService::getInstance()->bciRenderArea->getViewportRegion().getPixelsPerInch();

    bounding_rect = new QRectF(x,y,qimage->height()/pixels_per_inch, qimage->width()/pixels_per_inch);

    imageTran = new SoTransform;
    imageTran->translation.setValue(SbVec3f(x,y,0));

    SoMaterial * imagematerial = new SoMaterial;
    imagematerial->transparency = 0.5;

    sprite_root->addChild((imageTran));
    sprite_root->addChild(imagematerial);
    sprite_root->addChild(image);

}


Sprite::~Sprite()
{
    //delete qimage;
    //imageTran->unref();
    //image->unref();

}

bool Sprite::intersects(QRectF *other_rect)
{

    double other_center_x = other_rect->x() + (other_rect->width())/2.0 ;
    double other_center_y = other_rect->y() + (other_rect->width())/2.0 ;
    double other_radius = (other_rect->width())/2.0;

    double my_center_x = bounding_rect->x() + (bounding_rect->width())/2.0 ;
    double my_center_y = bounding_rect->y() + (bounding_rect->width())/2.0 ;
    double my_radius = (bounding_rect->width())/2.0;

    double dist = sqrt(pow(my_center_x-other_center_x,2) + pow(my_center_y-other_center_y,2));

    std::cout << "Checking intersections" << std::endl;
    std::cout << "RECT 1" << std::endl;
    std::cout << "x: ";
    std::cout << other_rect->x();
    std::cout << " y: ";
    std::cout << other_rect->y();
    std::cout << " h: ";
    std::cout << other_rect->height();
    std::cout << " w: ";
    std::cout << other_rect->width();
    std::cout << " center_x: ";
    std::cout << other_center_x;
    std::cout << " center_y: ";
    std::cout << other_center_y<<std::endl;

    std::cout << "RECT 2" << std::endl;
    std::cout << "x: ";
    std::cout << bounding_rect->x();
    std::cout << " y: ";
    std::cout << bounding_rect->y();
    std::cout << " h: ";
    std::cout << bounding_rect->height();
    std::cout << " w:";
    std::cout << bounding_rect->width();
    std::cout << " center_x: ";
    std::cout << my_center_x;
    std::cout << " center_y: ";
    std::cout << my_center_y<<std::endl;

    std::cout << "Distance Between Target and Cursor: ";
    std::cout << dist;

    //double min_dist_for_collision = .261;//.076
    double min_dist_for_collision = other_radius + my_radius;
    //double min_dist_for_collision = .0002* BCIService::getInstance()->bciRenderArea->getSize()[0];
    std::cout << "Min Distance For collision: ";
    std::cout << min_dist_for_collision << std::endl;
    //return this->bounding_rect->intersects(*other_rect);
    //return (other_radius + my_radius) > dist;
    if ((min_dist_for_collision) > dist)
    {
        std::cout << "We have a collision" << std::endl;
        return true;
    }
    return false;
}

void Sprite::update(int state)
{
//    image->filename = "show_image2.png";

//    QImage qimg;
//    convert(image->image, qimg);
//    QTransform rotating;
//    //rotating.rotate(rand()%360);
//    rotating.rotate(180);
//    qimg = qimg.transformed(rotating);
//    convert(qimg, image->image);
}


void Sprite::convert(const QImage& p, SoSFImage& img) const
{
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
