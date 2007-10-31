/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef PIXMAPLOADER_COMPONENT_H
#define PIXMAPLOADER_COMPONENT_H

#include <orcaice/component.h>
#include <orca/pixmap.h>
#include <orcaifaceimpl/pixmapImpl.h>

namespace pixmaploader {

//
// Deriving from orcaice::Component means we have to implement start() and stop()
//
class Component : public orcaice::Component
{
public:

    Component();
    virtual ~Component();

    virtual void start();
    virtual void stop();

private:

    //
    // EXTERNAL INTERFACE
    //
    orcaifaceimpl::PixMapImplPtr pixMapInterface_;
    
    void loadMapFromFile(orca::PixMapData &theMap);
};

}

#endif
