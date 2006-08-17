/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */
 
#include <iostream>
#include <assert.h>
#include <orcaice/orcaice.h>
#include <orcamisc/orcamisc.h>
#include "algohandler.h"
#include "pathplanner2dI.h"
#include "skeletongraphicsI.h"
#include "fakedriver.h"
#include "simplenavdriver.h"
#include "astardriver.h"
#include "skeletondriver.h"

using namespace std;
using namespace orca;
using namespace pathplanner;

AlgoHandler::AlgoHandler( const orcaice::Context & context )
    : driver_(0),
      pathPlannerTaskProxy_(0),
      context_(context)
{
    initNetwork();
    initDriver();
}

AlgoHandler::~AlgoHandler()
{
    delete pathPlannerTaskProxy_;
}

void
AlgoHandler::initNetwork()
{
    //
    // REQUIRED INTERFACE: OgMap
    //

    while( isActive() )
    {
        try
        {
            orcaice::connectToInterfaceWithTag<orca::OgMapPrx>( context_, ogMapPrx_, "OgMap" );
            break;
        }
        catch ( const orcaice::NetworkException & e )
        {
            context_.tracer()->error( "failed to connect to remote object. Will try again after 3 seconds." );
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(3));
        }
        // NOTE: connectToInterfaceWithTag() can also throw ConfigFileException,
        //       but if this happens it's ok if we just quit.
    }

    //
    // PROVIDED INTERFACES
    //
    

    // PathPlanner2d
    // create the proxy/buffer for tasks
    pathPlannerTaskProxy_ = new orcaice::PtrProxy<PathPlanner2dTaskPtr>; 
    pathPlannerDataProxy_ = new orcaice::PtrProxy<PathPlanner2dDataPtr>;

    pathPlannerI_ = new PathPlanner2dI( *pathPlannerTaskProxy_, *pathPlannerDataProxy_, context_ );
    Ice::ObjectPtr pathPlannerObj = pathPlannerI_;
    //pathPlanner2dObj_ = new PathPlanner2dI( *pathPlannerTaskProxy_, *pathPlannerDataProxy_ );
    // two possible exceptions will kill it here, that's what we want
    orcaice::createInterfaceWithTag( context_, pathPlannerObj, "PathPlanner2d" );
}

void
AlgoHandler::initDriver()
{

    // get the map once
    try
    {
        ogMapDataPtr_ = ogMapPrx_->getData();
    }
    catch ( const orca::DataNotExistException & e )
    {
        cout<<"warning: "<<e.what<<endl;
    }


    //
    // Read settings
    //
    std::string prefix = context_.tag() + ".Config.";

    double traversabilityThreshhold = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"TraversabilityThreshhold", 0.3 );
    double robotDiameterMetres = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"RobotDiameterMetres", 0.8 );
    int doPathOptimization = orcaice::getPropertyAsIntWithDefault( context_.properties(), prefix+"DoPathOptimization", 0 );

    // based on the config parameter, create the right driver
    string driverName = orcaice::getPropertyWithDefault( context_.properties(), prefix+"Driver", "simplenav" );
    context_.tracer()->debug( std::string("loading ")+driverName+" driver",3);
    if ( driverName == "simplenav" )
    {
        driver_ = new SimpleNavDriver( robotDiameterMetres,
                                       traversabilityThreshhold,
                                       doPathOptimization );
    }
    else if ( driverName == "skeletonnav" || driverName == "sparseskeletonnav" )
    {
        // QGraphics2d
        SkeletonGraphicsI* graphicsI = new SkeletonGraphicsI( context_, "SkeletonGraphics" );
        Ice::ObjectPtr graphicsObj = graphicsI;
        orcaice::createInterfaceWithTag( context_, graphicsObj, "SkeletonGraphics" ); 

        bool useSparseSkeleton = (driverName == "sparseskeletonnav");
        driver_ = new SkeletonDriver( ogMapDataPtr_,
                                      graphicsI,
                                      robotDiameterMetres,
                                      traversabilityThreshhold,
                                      doPathOptimization,
                                      useSparseSkeleton );
    }
    else if ( driverName == "astar" )
    {
        driver_ = new AStarDriver( robotDiameterMetres,
                                   traversabilityThreshhold,
                                   doPathOptimization  );
    }
    else if ( driverName == "fake" )
    {
        driver_ = new FakeDriver();
    }
    else {
        string errorStr = "Unknown driver type.";
        context_.tracer()->error( errorStr);
        context_.tracer()->info( "Valid driver values are {'simplenav', 'skeletonnav', 'sparseskeletonnav', 'astar', 'fake'}" );
        throw orcaice::Exception( ERROR_INFO, errorStr );
    }

    context_.tracer()->debug("driver instantiated",5);
}

void 
AlgoHandler::run()
{
    assert( driver_ );
    assert( pathPlannerTaskProxy_ );

    // we are in a different thread now, catch all stray exceptions
    try
    {

    PathPlanner2dTaskPtr taskPtr; 
    PathPlanner2dDataPtr pathDataPtr = new PathPlanner2dData;   

    while ( isActive() )
    {
        //
        //  ======== waiting for a task (blocking) =======
        //
        context_.tracer()->info("waiting for a new task");
        
        while ( isActive() )
        {
            int ret = pathPlannerTaskProxy_->getNext( taskPtr, 1000 );
            if ( ret!=0 )
            {
                // context_.tracer()->info("waiting for a new task");      
            }
            else
            {
                context_.tracer()->info("task arrived");  
                break;
            }
        }
        
        //
        // ===== tell driver to compute the path ========
        //
        
        // input: ogmap, task; output: path
        try 
        {
            context_.tracer()->info("telling driver to compute the path now");
            driver_->computePath( ogMapDataPtr_, taskPtr, pathDataPtr );
            driver_->addWaypointParameters( pathDataPtr );
        }
        catch ( orcapathplan::Exception &e )
        {
            std::stringstream ss;
            // TODO: display taskPtr.
            ss << "Couldn't compute path: " // << orcaice::toString(taskPtr)
               << endl << "Problem was: " << e.what();
            context_.tracer()->error( ss.str() );
            
            // Don't break here. Set error codes according to the thrown exception. Client can evaluate error codes.
            
//             if ( e.what() == "Start point was not within the map." ||  e.what() =="Start point was not traversable." || e.what() == "Couldn't connect start cell to skeleton" )
//             {
//                 pathDataPtr->result = PathStartNotValid;
//             }
//             
//             if ( e.what() == "End point was not within the map." ||  e.what() =="End point was not traversable." || e.what() == "Couldn't connect skeleton to goal")
//             {
//                 pathDataPtr->result = PathDestinationNotValid;
//             }
//             
//             if ( e.what() == "Couldn't compute potential function along skeleton")
//             {
//                 pathDataPtr->result = OtherError;
//             }
//             
//             if ( e.what() == "Computed potential function but could not compute path.")
//             {
//                 pathDataPtr->result = PathDestinationUnreachable;   
//             }
            
        }

        //
        // ======= send result (including error code) ===============
        //
        context_.tracer()->info("sending off the resulting path");

        // There are three methods to let other components know about the computed path:
        // 1. using the proxy
        if (taskPtr->prx!=0)
        {
            taskPtr->prx->setData( pathDataPtr );
        }
        // 2. and 3.: use getData or icestorm
        pathPlannerI_->localSetData( pathDataPtr );

        // resize the pathDataPtr: future tasks might not compute a path successfully and we would resend the old path
        pathDataPtr->path.resize( 0 );

    }

    //
    // unexpected exceptions
    //
    } // try
    catch ( const orca::OrcaException & e )
    {
        context_.tracer()->print( e.what );
        context_.tracer()->error( "unexpected (remote?) orca exception.");
        if ( context_.isApplication() ) {
            context_.tracer()->info( "this is a stand-alone component. Quitting...");
            context_.communicator()->destroy();
        }
    }
    catch ( const orcaice::Exception & e )
    {
        context_.tracer()->print( e.what() );
        context_.tracer()->error( "unexpected (local?) orcaice exception.");
        if ( context_.isApplication() ) {
            context_.tracer()->info( "this is a stand-alone component. Quitting...");
            context_.communicator()->destroy();
        }
    }
    catch ( const Ice::Exception & e )
    {
        cout<<e<<endl;
        context_.tracer()->error( "unexpected Ice exception.");
        if ( context_.isApplication() ) {
            context_.tracer()->info( "this is a stand-alone component. Quitting...");
            context_.communicator()->destroy();
        }
    }
    catch ( const std::exception & e )
    {
        // once caught this beast in here, don't know who threw it 'St9bad_alloc'
        cout<<e.what()<<endl;
        context_.tracer()->error( "unexpected std exception.");
        if ( context_.isApplication() ) {
            context_.tracer()->info( "this is a stand-alone component. Quitting...");
            context_.communicator()->destroy();
        }
    }
    catch ( ... )
    {
        context_.tracer()->error( "unexpected exception from somewhere.");
        if ( context_.isApplication() ) {
            context_.tracer()->info( "this is a stand-alone component. Quitting...");
            context_.communicator()->destroy();
        }
    }
    
    // wait for the component to realize that we are quitting and tell us to stop.
    waitForStop();
}
