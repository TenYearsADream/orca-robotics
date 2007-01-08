/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

//#include <iostream> // debug
#include <orcaice/orcaice.h>

#include "component.h"
#include "nethandler.h"
#include "hwhandler.h"

using namespace orca;
using namespace robot2d;

Component::Component() :
    orcaice::Component( "Robot2d" ),
    netHandler_(0),
    hwHandler_(0)
{
}

Component::~Component()
{
    // do not delete handlers!!! They derive from Ice::Thread and self-destruct.
}

// warning: this function returns after it's done, all variable that need to be permanet must
//          be declared as member variables.
void
Component::start()
{
    //
    // Network handling loop
    //
    // the constructor may throw, we'll let the application shut us down
    netHandler_ = new NetHandler( odometryPipe_, commandPipe_, context() );
    // this thread will try to activate and register the adapter
    netHandler_->start();

    //
    // Hardware handling loop
    //
    // the constructor may throw, we'll let the application shut us down
    hwHandler_ = new HwHandler( odometryPipe_, commandPipe_, context() );
    hwHandler_->start();

    // the rest is handled by the application/service
}

void
Component::stop()
{
    tracer()->debug( "stopping component", 2 );
    orcaice::Thread::stopAndJoin( netHandler_ );
    orcaice::Thread::stopAndJoin( hwHandler_ );
    tracer()->debug( "stopped component", 2 );
}
