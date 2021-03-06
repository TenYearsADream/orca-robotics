/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

// orca includes
#include <orcaice/orcaice.h>
#include <orca/odometry2d.h>
#include <orcaifaceimpl/odometry2d.h>

// player includes
#include <libplayerc++/playerc++.h>

#include "mainthread.h"

namespace player2orca {

MainThread::MainThread( const orcaice::Context& context ) :
    SafeThread(context.tracer()),
    context_(context)
{
}

MainThread::~MainThread()
{
}

void
MainThread::walk()
{
    // we are in a different thread now, catch all stray exceptions
    try
    {

    // read configuration parameters (config file + command line)
    std::string prefix = context_.tag()+".Config.";
    std::string playerHost = orcaice::getPropertyWithDefault( 
            context_.properties(), prefix+"Player.Host", "localhost" );
    int playerPort = orcaice::getPropertyAsIntWithDefault( 
            context_.properties(), prefix+"Player.Port", 6665 );

    // setup provided interface Odometry2d (use generic vehicle description)
    orca::VehicleDescription descr;
    orcaifaceimpl::Odometry2dImplPtr odometry2dI = 
        new orcaifaceimpl::Odometry2dImpl( descr, "Odometry2d", context_ );
    // this may fail, catch exceptions
    odometry2dI->initInterface();

    // orca data structure
    orca::Odometry2dData odometry2dData;

    // init Player, this may fail, catch exceptions
    PlayerCc::PlayerClient playerServer( playerHost.c_str(), playerPort );
    PlayerCc::Position2dProxy positionProxy( &playerServer, 0 );

    //
    // Main loop, ctrl-C will exit from this
    //   
    while( !isStopping() )
    {
        // get data from the player server, blocking read
        playerServer.Read();

        // prepare an orca message
        odometry2dData.timeStamp = orcaice::toOrcaTime( positionProxy.GetDataTime() );
    
        odometry2dData.pose.p.x = positionProxy.GetXPos();
        odometry2dData.pose.p.y = positionProxy.GetYPos();
        odometry2dData.pose.o   = positionProxy.GetYaw();
        
        odometry2dData.motion.v.x = positionProxy.GetXSpeed();
        odometry2dData.motion.v.y = positionProxy.GetYSpeed();
        odometry2dData.motion.w   = positionProxy.GetYawSpeed();

        odometry2dData.odometryWasReset = false;

        // check that we were not told to terminate while we were sleeping
        if ( !isStopping() ) {
            context_.tracer().debug( "Setting data", 5 );
            odometry2dI->localSetAndSend( odometry2dData );
        } 
    }

    } // try
    catch ( const Ice::CommunicatorDestroyedException & e )
    {
        context_.tracer().debug( "net handler cought CommunicatorDestroyedException",1);        
        // it's ok, we must be quitting.
    }
    //
    // unexpected exceptions
    //
    catch ( ... )
    {
        context_.tracer().error( "Unexpected exception from somewhere.");
        context_.communicator()->destroy();
    }

    // wait for the component to realize that we are quitting and tell us to stop.
    waitForStop();
    context_.tracer().debug( "MainThread stopped", 2 );
}

} // namespace
