/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
 
#include <iostream>
#include <cmath> 
#include <qpainter.h>

#include "wifipainter.h"

using namespace orca; 
using namespace std;

namespace orcaqgui2d
{

WifiPainter::WifiPainter()
{
}

WifiPainter::~WifiPainter()
{
}

void
WifiPainter::clear()
{
}

void 
WifiPainter::setData( const WifiData& wifiData )
{
}

void WifiPainter::paint( QPainter *painter, int z )
{
    if ( z != Z_FOREGROUND ) return;
}

} // namespace

