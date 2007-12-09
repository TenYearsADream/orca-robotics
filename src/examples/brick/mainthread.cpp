/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <orcaice/orcaice.h>

#include "mainthread.h"

using namespace std;
using namespace brick;

MainThread::MainThread( const orcaice::Context& context ) :
    SafeThread( context.tracer(), context.status(), "main" ),
    context_(context)
{
    context_.status()->setMaxHeartbeatInterval( subsysName(), 10.0 );
    context_.status()->ok( subsysName(), "initialized" );
}

MainThread::~MainThread()
{
}

void
MainThread::walk()
{
    // multi-try function
    orcaice::activate( context_, this );

    //
    // Read configuration settings
    //
    std::string prefix = context_.tag() + ".Config.";

    int sleepIntervalMs = orcaice::getPropertyAsIntWithDefault( context_.properties(),
            prefix+"SleepIntervalMs", 1000 );

    //
    // Main loop
    //   
    context_.tracer()->debug( "Entering main loop", 2 );
    context_.status()->ok( subsysName(), "Running main loop" );
    while( !isStopping() )
    {
        context_.tracer()->debug( "Running main loop", 5 );
        context_.status()->heartbeat( subsysName() );

        // here we can do something useful

        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepIntervalMs));
    }
    context_.tracer()->debug( "Exiting main loop", 2 );
}
