/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#include <orcaobj/mathdefs.h>
#include <iostream>
#include <sstream>
#include <orcaqgui/exceptions.h>
#include <orcaqgui/cov2d.h>
#include "paintutils.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLES

// Robot size [pixel]
const double FRAME_LENGTH = 10.0;
const double FRAME_WIDTH = 8.0;

// How much uncertainty to draw
const int N_SIGMA = 3;
const int N_LENGTH = 3;

// Thickness of lines [pixel]
const double THIN_LINE_THICKNESS = 2.0;
    
////////////////////////////////////////////////////////////////////////////////

namespace orcaqgui {

PoseHistory::PoseHistory( double lineThickness )
    : lineThickness_(lineThickness),
      totalPoints_(0)
{}
    
void
PoseHistory::addPoint( const double x, const double y )
{
    if ( histories_.isEmpty() )
    {
        QPolygonF history;
        history.push_back( QPointF(x,y) );
        totalPoints_++;
        histories_.push_back( history );
    }
    
    const QPointF last = histories_.last().last();
    double dist = hypotf( last.x()-x, last.y()-y );
    
    if ( dist > _historyMaxDistance )
    {
        // start a new history
        QPolygonF history;
        history.push_back( QPointF(x,y) );
        totalPoints_++;
        histories_.push_back( history );
    }
    else
    {
        histories_.last().push_back( QPointF(x,y) );  
        totalPoints_++;  
    }
    
    if (totalPoints_>_historyMaxLength)
    {
        // take a point from the beginning off
        histories_.first().remove( 0,1 );
        if (histories_.first().size()==0)
            histories_.remove( 0,1 );
    }
}

void
PoseHistory::paint( QPainter *p, const QColor &colour )
{
    QMatrix m = p->worldMatrix();

    for (int i=0; i<histories_.size(); i++)
    {
        p->setPen( QPen( colour, lineThickness_/m.m11() ) );
        p->drawPolyline( histories_[i] );
    }
}

void
paintOrigin( QPainter *p, QColor colour )
{
    p->save();
    
    QMatrix m = p->worldMatrix();  // this is m2win matrix
    const double radius  = 6.0/m.m11(); 
    const double cross = 12.0/m.m11();
    const double lineThickness = THIN_LINE_THICKNESS/m.m11();
    
    p->setBrush( QBrush() );
    p->setPen( QPen( colour, lineThickness ) );

    // display origin
    p->drawEllipse( QRectF(-radius,-radius, 2.0*radius, 2.0*radius) );
    p->drawLine( QPointF(0.0,0.0), QPointF(cross,0.0) );
    p->drawLine( QPointF(0.0,0.0), QPointF(0.0,cross) );
    
    p->setFont( QFont("Helvetica [Cronyx]", 12) );
    
    QPointF xlabel = QPointF(cross,0.0) * m;       // x-label position in window cs
    QPointF ylabel = QPointF(0.0,cross) * m;       // y-label position in window cs

    p->setWorldMatrix( QMatrix() );
    
    p->drawText( xlabel, "x" );
    p->drawText( ylabel, "y" );
    
    p->restore();
}

void
paintPlatformPose( QMatrix &m2win, QPainter *p, QColor colour, float transparencyMultiplier )
{
    const double length = FRAME_LENGTH/m2win.m11();
    const double width =  FRAME_WIDTH/m2win.m11();
    const double lineThickness = THIN_LINE_THICKNESS/m2win.m11();
    
    // paint body
    p->setBrush( colour );
    // The outline
    p->setPen( QPen( Qt::black, lineThickness ) );
    
    p->drawRect( QRectF( -length, -width, 2.0*length,2.0*width) ); // top,left, width,height

    p->drawLine( QPointF(0.0,0.0), QPointF(length,width) );
    p->drawLine( QPointF(0.0,0.0), QPointF(length,-width) );
}

void
paintUncertaintyInfo( QMatrix &m2win, QPainter *p, QColor colour, float thetaMean, float pxx, float pxy, float pyy, float ptt )
{
    paintUncertaintyWedge( m2win, p, colour, thetaMean, ptt );
    paintCovarianceEllipse( m2win, p, colour, pxx, pxy, pyy );
}

void
paintUncertaintyWedge( QMatrix &m2win, QPainter *p, QColor colour, float thetaMean, float ptt )
{
    const double length = FRAME_LENGTH/m2win.m11();
    const double lineThickness = THIN_LINE_THICKNESS/m2win.m11();
    
    p->save();
    {
        p->rotate( RAD2DEG(thetaMean) );
        p->setPen( QPen(colour, lineThickness) );
        p->setBrush( Qt::NoBrush );
        int pTheta = (int) (ptt * (180.0/M_PI));
        p->drawPie( QRectF(-N_LENGTH*length,-N_LENGTH*length,
                    2.0*N_LENGTH*length,2.0*N_LENGTH*length),
                    -N_SIGMA*pTheta*16, 2*N_SIGMA*pTheta*16 );
    }
    p->restore();
}

void
paintCovarianceEllipse( QMatrix &m2win, QPainter *p, QColor colour, float pxx, float pxy, float pyy )
{
    const double lineThickness = THIN_LINE_THICKNESS/m2win.m11();
    
    // Quick checks first (note that this is a necessary but not
    // sufficient condition for positive-definiteness)
    if ( pxx < 0.0 ||
         pyy < 0.0 ||
         fabs(pxy) >= sqrt( pxx*pyy ) ||
         (isnan(pxx)||isinf(pxx))    ||
         (isnan(pxy)||isinf(pxy))    ||
         (isnan(pyy)||isinf(pyy))    )
    {
        std::stringstream ss;
        ss << "paintCovarianceEllipse(): covariance matrix not PD: pxx,pxy,pyy = "<<pxx<<","<<pxy<<","<<pyy;
//         ss << endl << "fabs(pxy)-sqrt(pxx*pyy): " << fabs(pxy)<< "-" <<sqrt(pxx*pyy)<<" = "<< fabs(pxy)-sqrt(pxx*pyy);
//         ss << endl << "fabs(pxy) > sqrt( pxx*pyy ): " << (fabs(pxy) > sqrt( pxx*pyy ));
//         ss << endl << "isnan||isinf(pxx): " << (isnan(pxx)||isinf(pxx));
//         ss << endl << "isnan||isinf(pxy): " << (isnan(pxy)||isinf(pxy));
//         ss << endl << "isnan||isinf(pyy): " << (isnan(pyy)||isinf(pyy));
        throw orcaqgui::Exception( ss.str() );
    }

    // Work out the details of the uncertainty ellipse
    double a, b, psi;
    Cov2d cov( pxx, pxy, pyy );
    cov.ellipse( a, b, psi );

    QPointF covPoint(a,b);
    float psiDeg = (int)ceil( psi *180/M_PI );            // rot. angle into princ. axes

    // Paint uncertainty ellipse
    p->save();
    {
        p->rotate( psiDeg );
        p->setPen( QPen( colour, lineThickness ) );
        p->setBrush( Qt::NoBrush );
        p->drawEllipse( QRectF( -N_SIGMA*a,
                                -N_SIGMA*b,
                                2*N_SIGMA*a,
                                2*N_SIGMA*b ) );
    }
    p->restore();
}


QColor
getTransparentVersion( const QColor &c, float transparencyMultiplier )
{
    // Lower alpha gives a more transparent version.
    int alpha = (int) (c.alpha()*transparencyMultiplier);
    return QColor( c.red(),
                   c.green(),
                   c.blue(),
                   alpha );
}

void paintWaypoint( QPainter *p,
                    QColor &fillColor,
                    QColor &drawColor,
                    int   targetHeading,
                    float distanceTolerance,
                    int   headingTolerance )
{
    p -> setPen( QPen( drawColor, 0.1 ) );
    p -> drawEllipse( QRectF( -distanceTolerance,
                      -distanceTolerance,
                      2*distanceTolerance,
                      2*distanceTolerance) );       // draws circle as tolerances
    p -> setBrush( fillColor );
    int a = -(targetHeading-headingTolerance);
    int alen = -2*headingTolerance;
    // !!!!!!!!!! WARNING!!!!!!!!!!!!!!!!!!!!
    // Qt documentation is wrong: positive is clockwise and negative is counterclockwise
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    p -> drawPie( QRectF( -distanceTolerance,
                  -distanceTolerance,
                  2*distanceTolerance,
                  2*distanceTolerance ),
    a,
    alen );
}

} // namespace
