/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef COMPONENT_H
#define COMPONENT_H

#include <orcaice/component.h>

namespace registryview {

class Component : public orcaice::Component
{
public:
    Component() :
        orcaice::Component( "RegistryView", orcaice::NoStandardInterfaces ) {};

    // from orcaice::Component
    virtual void start();
    virtual void stop() {};
};

}

#endif
