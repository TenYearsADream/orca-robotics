/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */
#include <orcaice/orcaice.h>

#include "component.h"
#include "mainloop.h"
// implementations of Ice objects
#include "localise2dI.h"
#include "odometry2dconsumerI.h"

using namespace std;
using namespace orca;
using namespace faithlocaliser;

Component::Component()
    : orcaice::Component( "FaithLocaliser" ),
      mainLoop_(0)
{
}

Component::~Component()
{
}

void
Component::start()
{
    //
    // INITIAL CONFIGURATION
    //

    Ice::PropertiesPtr prop = properties();
    std::string prefix = tag();
    prefix += ".Config.";

    // basic configs
    double StdDevPosition = orcaice::getPropertyAsDoubleWithDefault( prop, prefix+"StdDevPosition", 0.05 );
    double StdDevHeading = orcaice::getPropertyAsDoubleWithDefault( prop, prefix+"StdDevHeading", 1.0 );
    int HistoryDepth = orcaice::getPropertyAsIntWithDefault( prop, prefix+"HistoryDepth", 50 );

    historyBuffer_.configure(HistoryDepth);
    //
    // EXTERNAL PROVIDED INTERFACE
    //
    // Find IceStorm Topic to which we'll publish
    IceStorm::TopicPrx localiseTopicPrx = orcaice::connectToTopicWithTag<Localise2dConsumerPrx>
        ( context(), localise2dPublisher_, "Localise2d" );
    
    // create servant for direct connections
    Ice::ObjectPtr localise2dObj_ = new Localise2dI( localiseTopicPrx, locBuffer_, historyBuffer_, context() );
    orcaice::createInterfaceWithTag( context(), localise2dObj_, "Localise2d" );

    ////////////////////////////////////////////////////////////////////////////////

    //
    // ENABLE NETWORK CONNECTIONS
    //
    // this may throw, but may as well quit right then
    activate();

    //
    // EXTERNAL REQUIRED INTERFACES
    //
    Odometry2dPrx odo2dPrx;
    orcaice::connectToInterfaceWithTag<orca::Odometry2dPrx>( context(),
                                                             odo2dPrx,
                                                             "Odometry2d" );

    // create a callback object to recieve scans
    Ice::ObjectPtr consumer = new Odometry2dConsumerI( posPipe_ );
    orca::Odometry2dConsumerPrx consumerPrx =
        orcaice::createConsumerInterface<orca::Odometry2dConsumerPrx>( context(), consumer );

    //
    // Subscribe for data
    //
    // will try forever until the user quits with ctrl-c
    // TODO: this will not actually quit on ctrl-c
    while ( true ) //isActive() )
    {
        try
        {
            odo2dPrx->subscribe( consumerPrx );
            break;
        }
        catch ( const orca::SubscriptionFailedException & e )
        {
            tracer()->error( "failed to subscribe for data updates. Will try again after 3 seconds." );
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(3));
        }
    }

    //
    // MAIN DRIVER LOOP
    //

    mainLoop_ = new MainLoop(localise2dPublisher_,
                             posPipe_,
			                 locBuffer_,
                             historyBuffer_,
                             StdDevPosition,
                             StdDevHeading,
                             context() );
    
    mainLoop_->start();
}

void
Component::stop()
{
    tracer()->debug( "stopping component", 5 );
    orcaice::stopAndJoin( mainLoop_ );
    tracer()->debug( "stopped component", 5 );
}
