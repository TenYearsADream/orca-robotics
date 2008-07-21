#include "osgutil.h"
#include <iostream>
#include <assert.h>
#include <osg/PolygonMode>
#include <osg/BlendFunc>
#include <osg/ShapeDrawable>
#include <osg/Geode>

using namespace std;

namespace orcaqgui3d {

osg::ref_ptr<osg::PositionAttitudeTransform>
getPositionAttitudeTransform( double x,
                              double y,
                              double z,
                              double roll,
                              double pitch,
                              double yaw )
{
    osg::ref_ptr<osg::PositionAttitudeTransform> pos = new osg::PositionAttitudeTransform;

    pos->setPosition( osg::Vec3(x,y,z) );

    // Not sure this order is right...
    osg::Quat rot;
    rot.makeRotate( roll,  osg::Vec3(1,0,0) );
    rot.makeRotate( pitch, osg::Vec3(0,1,0) );
    rot.makeRotate( yaw,   osg::Vec3(0,0,1) );

    pos->setAttitude( rot );
    return pos;
}

// Force the Node and its children to be wireframe, overrides parents state
void
setWireFrameMode( osg::Node *srcNode )
{
    assert( srcNode != NULL );

    osg::StateSet *state = srcNode->getOrCreateStateSet();

    osg::PolygonMode *polyModeObj = 
        dynamic_cast<osg::PolygonMode*>( state->getAttribute( osg::StateAttribute::POLYGONMODE ));
    if ( !polyModeObj ) 
    {
        polyModeObj = new osg::PolygonMode;
        state->setAttribute( polyModeObj );    
    }

    polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
}

void
setAntiAliasing( osg::Node *node )
{
    osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet();
    osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    stateSet->setAttribute( blendFunc.get() );

    stateSet->setMode( GL_POINT_SMOOTH, osg::StateAttribute::ON );
    stateSet->setMode( GL_LINE_SMOOTH, osg::StateAttribute::ON );
    stateSet->setMode( GL_BLEND, osg::StateAttribute::ON );
    node->setStateSet( stateSet.get() );    
}

osg::ref_ptr<osg::Geometry>
drawEllipse( float radiusX,
             float radiusY,
             const osg::Vec4 &color,
             float z,
             int numPts )
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );
    
    float angle=0;
    for ( int i=0; i < numPts; i++ )
    {
        vertices->push_back( osg::Vec3( radiusX*cos(angle), radiusY*sin(angle), z ) );
        angle += 2*M_PI/(float)(numPts);
    }    

    osg::ref_ptr<osg::DrawElementsUInt> prim = 
        new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP);
    geometry->addPrimitiveSet( prim.get() );
    for ( uint i=0; i < vertices->size(); i++ )
        prim->push_back( i );

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back( color );
    geometry->setColorArray(colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

    return geometry;
}

osg::ref_ptr<osg::Geode>
drawCylinder( float height,
              float radius,
              const osg::Vec4 &color )
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    // The cylinder
    osg::ref_ptr<osg::Cylinder> cyl = new osg::Cylinder( osg::Vec3( 0, 0, 0 ), radius, height );
    osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(cyl.get());
    drawable->setColor( color );

    geode->addDrawable( drawable.get() );

    // Circles at top and bottom
    osg::ref_ptr<osg::Geometry> topCircle = drawEllipse( radius,
                                                         radius, 
                                                         osg::Vec4(0,0,0,1), 
                                                         height/2.0 );
    osg::ref_ptr<osg::Geometry> bottomCircle = drawEllipse( radius, 
                                                            radius, 
                                                            osg::Vec4(0,0,0,1), 
                                                            -height/2.0 );
    geode->addDrawable( topCircle.get() );
    geode->addDrawable( bottomCircle.get() );

    cout<<"TRACE(osgutil.cpp): drawn the cylinder." << endl;

    return geode;
}


}
