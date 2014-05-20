#include "Servers/RenderableProtoDrawer.h"
#include <Inventor/lists/SoPathList.h>

#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoCoordinate3.h>

#include "debug.h"


SoNodeList Renderer::getChildByName(SoSeparator * ivRoot, SbName & childName,
                                            SoType targetType, int maxResultsExpected)
{
    assert(ivRoot);
    SoNodeList resultList;

    SoSearchAction sa;
    sa.setSearchingAll(true);

    sa.setType(targetType, true);
    sa.setInterest( SoSearchAction::ALL);
    sa.setName(childName);
    sa.setFind(SoSearchAction::NAME);
    sa.apply(ivRoot);

    SoPathList &pathList = sa.getPaths();
    int numPaths = pathList.getLength();
    if (numPaths > maxResultsExpected)
    {
        DBGA(this->className() << "::getChildByName::Found too many children of node: "
             << ivRoot->getName().getString()  << " with name: "
             <<childName.getString() << " " );
        DBGA(this->className() << "::getChildByName:: Expected:" << maxResultsExpected
             << " Found:" << numPaths);
        resultList.append(static_cast<SoNode *>(NULL));
        return resultList;
    }

    for(int i = 0; i < numPaths; ++i)
    {
        resultList.append(pathList[i]->getTail());
    }
    return resultList;
}



 SoSeparator * Renderer::getOrAddSeparator(SoSeparator * ivRoot, SbName & childName)
{
    SoNodeList children = getChildByName(ivRoot, childName);
    if(children.getLength())
        return static_cast<SoSeparator *>(children[0]);
    else
    {
        SoSeparator * newChild = new SoSeparator();
        newChild->setName(childName);
        ivRoot->addChild(newChild);
        return newChild;
    }
}

SoSeparator *Renderer::getRenderRoot(SbName &childName, Renderable &renderable)
{
    SoSeparator * ivRoot = GraspItGUI::getInstance()->getIVmgr()->getWorld()->getIVRoot();

    if(renderable.has_renderableframe())
    {
        SbName frameName(renderable.renderableframe().c_str());
        ivRoot = getOrAddSeparator(ivRoot, frameName);
    }
    if(ivRoot && childName.getLength() > 0)
    {
        ivRoot = getOrAddSeparator(ivRoot, childName);
    }
    return ivRoot;
}

bool Renderer::render(Renderable &renderable, QString & renderableRootName)
{
    SbName renderableSeparatorName(renderableRootName.toStdString().c_str());
    SoSeparator * ivRoot = getRenderRoot(renderableSeparatorName, renderable);
    if(ivRoot)
        return renderImpl(ivRoot, renderable);
    else
        return false;

}




bool PointCloudRenderer::createNodes(SoSeparator *ivRoot, Renderable &renderable)
{

    SoTransform * coord_tran = new SoTransform();
    coord_tran->setName("PointCloudTransform");
    SoCoordinate3 * coord = new SoCoordinate3();
    coord->setName("PointCloudCoordinates");
    SoPointSet * pointSet = new SoPointSet();
    SoDrawStyle * drawStyle = new SoDrawStyle();

    SoMaterial * mat = new SoMaterial();
    mat->setName("PointCloudColorMaterial");
    SoMaterialBinding * matBinding = new SoMaterialBinding();
    matBinding->value = SoMaterialBinding::PER_PART;
    ivRoot->addChild(coord_tran);
    setScale(ivRoot, renderable);
    ivRoot->addChild(coord);
    ivRoot->addChild(mat);
    ivRoot->addChild(matBinding);

    drawStyle->pointSize = 3;
    ivRoot->addChild(drawStyle);

    ivRoot->addChild(pointSet);
    return true;
}

float PointCloudRenderer::getProtoScale(Renderable & renderable)
{
    float factor = 1.0;
    if(renderable.pointcloud().has_units())
    {
        factor = 1000.0/renderable.pointcloud().units();
    }
    return factor;
}

void PointCloudRenderer::setScale(SoSeparator *ivRoot, Renderable &renderable)
{
    float factor = 1.0;
    factor = getProtoScale(renderable);
    if(renderable.pointcloud().has_units())
    {
        factor = 1000.0/renderable.pointcloud().units();
    }


    if (ivRoot->getNumChildren() < 2 || ivRoot->getChild(1)->getTypeId() != SoScale::getClassTypeId())
    {
        ivRoot->insertChild(new SoScale(),1);
    }

    SoScale * cloudScale = static_cast<SoScale *>(ivRoot->getChild(1));
    cloudScale->setName("PointCloudScale");
    cloudScale->scaleFactor.setValue(factor, factor, factor);
}

bool PointCloudRenderer::renderImpl(SoSeparator *ivRoot, Renderable &renderable)
{
    //Being here without a point cloud is a bug.
    assert(renderable.has_pointcloud() || renderable.has_pointcloud2());

    //Create the node subtree of the renderable if necessary.
    if(!ivRoot->getNumChildren())
    {
        createNodes(ivRoot, renderable);
    }
    setScale(ivRoot, renderable);
    //Get the nodes to insert the point data into.
    SoMaterial * mat = static_cast<SoMaterial *>(ivRoot->getChild(3));
    SoCoordinate3 * coord =
            static_cast<SoCoordinate3 *>(ivRoot->getChild(2));

    //Read the point data into Inventor structures.
    std::vector<SbVec3f> points;
    std::vector<SbColor> colors;
    int point_size = fillPointList(renderable, points,colors);
    //If points were parsed or the input meant to send no points,
    //set the new point cloud points and colors.
    if(points.size() || !point_size)
    {
        //Insert the inventor structures in to the
        coord->point.setValues(0,points.size(), &points[0]);
        mat->diffuseColor.setValues(0,colors.size(), &colors[0]);
    }
    return points.size() > 0;
}

int PointCloudRenderer::fillPointList(Renderable & renderable,
                                       std::vector<SbVec3f> &points,
                                       std::vector<SbColor> &colors)
{
    //Unpack the pointcloud just for future code berevity
    const PointCloudXYZRGB & pc(renderable.pointcloud());
    int pointNum = pc.points_size();
    points.reserve(pointNum);
    colors.reserve(pointNum);

    for(int i = 0; i < pointNum; ++i)
    {
        //Unpack the data from the nested data structures
        const PointXYZRGB & pc_colored_point(pc.points(i));
        const PointXYZ & pc_point(pc_colored_point.point());
        const ColorRGB & pc_color(pc_colored_point.color());
        if (std::isnan(pc_point.x()))
            continue;
        //Put the data in the data structures
        points.push_back(SbVec3f(pc_point.x(), pc_point.y(), pc_point.z()));
        colors.push_back(SbColor(pc_color.red(), pc_color.green(), pc_color.blue()));
    }
    return pointNum;

}

float PointCloud2Renderer::getProtoScale(Renderable &renderable)
{
    float factor = 1.0;
    if(renderable.pointcloud2().has_units())
    {
        factor = 1000.0/renderable.pointcloud2().units();
    }
    return factor;

}

int PointCloud2Renderer::fillPointList(Renderable & renderable,
                                        std::vector<SbVec3f> &points,
                                       std::vector<SbColor> &colors)
{
    const PointCloud2 & pc(renderable.pointcloud2());

    int pointNum = pc.height()*pc.width();
    points.reserve(pointNum);
    colors.reserve(pointNum);
    QByteArray dataArray;
    char * rawData = const_cast<char *>(pc.data().c_str());

    for(int i = 0; i < pointNum; ++i)
    {

        dataArray.fromRawData(rawData,4);
        float x = *reinterpret_cast<float *> (rawData);

        rawData += 4;

        float y = *reinterpret_cast<float *> (rawData);

        rawData += 4;

        float z = *reinterpret_cast<float *> (rawData);
        rawData += 8;
        float blue = *reinterpret_cast<unsigned char *>(rawData)/255.0;
        rawData += 1;
        float green = *reinterpret_cast<unsigned char *>(rawData)/255.0;
        rawData += 1;
        float red = *reinterpret_cast<unsigned char *>(rawData)/255.0;
        rawData += 14;
        if(!std::isnan(x))
        {
            points.push_back(SbVec3f(x, y, z));
            colors.push_back(SbColor(red, green, blue));
        }
    }
    return pointNum;
}


bool FrameRenderer::renderImpl(SoSeparator * ivRoot, Renderable &renderable)
{
    SoSeparator * worldIVRoot = GraspItGUI::getInstance()->getIVmgr()->getWorld()->getIVRoot();
    SbName pc2Name("PointCloudRoot2");
    SoNodeList pointCloud2 = getChildByName(worldIVRoot, pc2Name);

    assert(renderable.has_renderableframe());
    SbName frameName(renderable.renderableframe().c_str());
    ivRoot = getOrAddSeparator(worldIVRoot, frameName);
    SoTransform * tran;

    if(ivRoot->getNumChildren() && ivRoot->getChild(0)->getTypeId() == SoTransform::getClassTypeId())
    {
        tran = static_cast<SoTransform *>(ivRoot->getChild(0));
        DBGA("IVRoot child number: " << ivRoot->getNumChildren());
    }
    else
    {
        tran = new SoTransform();
        ivRoot->insertChild(tran, 0);
    }
    Quaternion q(renderable.frame().orientation().w(),renderable.frame().orientation().x(),
                 renderable.frame().orientation().y(),renderable.frame().orientation().z());
    tran->rotation.setValue(q.toSbRotation());
    float units = 1.0;

    if(renderable.frame().has_units())
        units = 1000.0/renderable.frame().units();

    tran->translation.setValue(renderable.frame().translation().x() * units,
                               renderable.frame().translation().y() * units,
                               renderable.frame().translation().z() * units);
    return true;
}


void RenderableProtoDrawer::renderMessage(Renderable &renderable)
{
    if(renderable.has_pointcloud())
    {
        PointCloudRenderer renderer;
        QString ivRootName("PointCloudRoot");
        renderer.render(renderable,ivRootName);
    }
    if(renderable.has_pointcloud2())
    {
        PointCloud2Renderer renderer;
        QString ivRootName("PointCloudRoot2");
        renderer.render(renderable,ivRootName);
    }
    if(renderable.has_frame())
    {
        FrameRenderer renderer;
        QString emptyName = "";
        renderer.render(renderable, emptyName);
    }
}
