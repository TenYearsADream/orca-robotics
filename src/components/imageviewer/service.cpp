/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include "service.h"
#include "component.h"

using namespace std;
using namespace imageviewer;

extern "C"
{
    // Factory function

    IceBox::Service* create( Ice::CommunicatorPtr communicator )
    {
        return new imageviewer::Service;
    }
}

namespace imageviewer {

Service::Service()
{
    component_ = new imageviewer::Component;
}

}
