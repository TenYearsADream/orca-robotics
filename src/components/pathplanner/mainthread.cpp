/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
 
#include <iostream>
#include <assert.h>
#include <orcaice/orcaice.h>
#include <orcaobj/orcaobj.h>
#include <hydroutil/hydroutil.h>
#include "mainthread.h"
#include "pathplanner2dI.h"
#include "fakedriver.h"
#include "skeletondriver.h"

#include <orcaogmap/orcaogmap.h>

#ifdef QT4_FOUND
    #include "skeletongraphicsI.h"
#endif


using namespace std;
using namespace orca;
using namespace pathplanner;

namespace {

    class DistBasedCostEvaluator : public hydropathplan::CostEvaluator {
    public:
        DistBasedCostEvaluator( double distanceThreshold,
                                double costMultiplier )
            : distanceThreshold_(distanceThreshold),
              costMultiplier_(costMultiplier)
            {}

        double costAtDistFromObstacle( double distInMetres ) const
            {
                if ( distInMetres < 0 )
                    return NAN;
                else if ( distInMetres < distanceThreshold_ ) 
                    return costMultiplier_; 
                else 
                    return 1;
            }
        
    private:
        double distanceThreshold_;
        double costMultiplier_;
    };

}

MainThread::MainThread( const orcaice::Context & context )
    : hydroiceutil::SubsystemThread( context_.tracer(), context.status(), "MainThread" ),
      pathPlannerTaskBuffer_( 100, hydroiceutil::BufferTypeQueue ),
      context_(context)
{
    subStatus().setMaxHeartbeatInterval( 30.0 );
}

void
MainThread::initNetwork()
{
    //
    // REQUIRED INTERFACE: OgMap
    //
    subStatus().initialising("Connecting to OgMap" );
    orca::OgMapPrx ogMapPrx;    
    orcaice::connectToInterfaceWithTag<orca::OgMapPrx>( context_, ogMapPrx, "OgMap", this, subsysName() );

    // get the og map once
    subStatus().initialising("Getting Og Map" );
    orca::OgMapData ogMapSlice;
    try
    {
        ogMapSlice = ogMapPrx->getData();
        stringstream ss;
        ss << "MainThread::"<<__func__<<"(): got ogMap: " << orcaobj::toString( ogMapSlice );
        context_.tracer().info( ss.str() );
    }
    catch ( const orca::DataNotExistException & e )
    {
        std::stringstream ss;
        ss << "algohandler::"<<__func__<<": DataNotExistException: "<<e.what;
        context_.tracer().warning( ss.str() );
    }
    // convert into internal representation
    orcaogmap::convert(ogMapSlice,ogMap_);
    
    //
    // PROVIDED INTERFACES
    //

    // PathPlanner2d
    subStatus().initialising("Creating PathPlanner2d Interface" );
    pathPlannerI_ = new PathPlanner2dI( pathPlannerTaskBuffer_, context_ );
    Ice::ObjectPtr pathPlannerObj = pathPlannerI_;
    
    // two possible exceptions will kill it here, that's what we want
    orcaice::createInterfaceWithTag( context_, pathPlannerObj, "PathPlanner2d" );
}

void
MainThread::initDriver()
{
    subStatus().initialising("Initialising Driver" );
    //
    // Read settings
    //
    std::string prefix = context_.tag() + ".Config.";

    double traversabilityThreshhold = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"TraversabilityThreshhold", 0.3 );
    double robotDiameterMetres = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"RobotDiameterMetres", 0.8 );
    int doPathOptimization = orcaice::getPropertyAsIntWithDefault( context_.properties(), prefix+"DoPathOptimization", 0 );
    bool jiggleWaypointsOntoClearCells = orcaice::getPropertyAsIntWithDefault( context_.properties(), prefix+"JiggleWaypointsOntoClearCells", true );

    // based on the config parameter, create the right driver
    string driverName = orcaice::getPropertyWithDefault( context_.properties(), prefix+"Driver", "simplenav" );
    context_.tracer().debug( std::string("loading ")+driverName+" driver",3);
    
    if ( driverName == "simplenav" || driverName == "astar")
    {
        hydropathplan::IPathPlanner2d *pathPlanner=NULL;
        
        if (driverName == "simplenav") {
            pathPlanner = new hydropathplan::SimpleNavPathPlanner( ogMap_,
                                                           robotDiameterMetres,
                                                           traversabilityThreshhold,
                                                           doPathOptimization );
        } else if (driverName == "astar") {
            pathPlanner = new hydropathplan::AStarPathPlanner( ogMap_,
                                                           robotDiameterMetres,
                                                           traversabilityThreshhold,
                                                           doPathOptimization );
        }
        driver_.reset( new GenericDriver( pathPlanner,
                                          ogMap_,
                                          robotDiameterMetres,
                                          traversabilityThreshhold,
                                          doPathOptimization,
                                          jiggleWaypointsOntoClearCells,
                                          context_ ) );
    }    
    else if ( driverName == "skeletonnav" || driverName == "sparseskeletonnav" )
    {
        bool useSparseSkeleton = (driverName == "sparseskeletonnav");

        double distanceThreshold = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"Skeleton.Cost.DistanceThreshold", 0.3 );
        double costMultiplier = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"Skeleton.Cost.CostMultiplier", 10 );
        double addExtraSparseSkelNodes = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"Skeleton.SparseSkelAddExtraNodes", 1 );
        double sparseSkelExtraNodeResolution = orcaice::getPropertyAsDoubleWithDefault( context_.properties(), prefix+"Skeleton.SparseSkelExtraNodeResolution", 5 );

        costEvaluator_.reset( new DistBasedCostEvaluator( distanceThreshold, costMultiplier ) );

        try {
            driver_.reset( new SkeletonDriver( ogMap_,
                                               robotDiameterMetres,
                                               traversabilityThreshhold,
                                               doPathOptimization,
                                               jiggleWaypointsOntoClearCells,
                                               useSparseSkeleton,
                                               addExtraSparseSkelNodes,
                                               sparseSkelExtraNodeResolution,
                                               *costEvaluator_,
                                               context_ ) );
        }
        catch ( hydropathplan::Exception &e )
        {
            // unrecoverable error
            context_.shutdown(); 
            std::stringstream ss;
            ss << "Trouble constructing a skeletondriver" << endl << "Problem was: " << e.what();
            throw hydropathplan::Exception( ss.str() );  // this will exit
        }
        
    }
    else if ( driverName == "fake" )
    {
        driver_.reset( new FakeDriver() );
    }
    else {
        // unrecoverable error
        context_.shutdown(); 
        string errorStr = "Unknown driver type.";
        context_.tracer().error( errorStr);
        context_.tracer().info( "Valid driver values are {'simplenav', 'skeletonnav', 'sparseskeletonnav', 'astar', 'fake'}" );
        throw hydroutil::Exception( ERROR_INFO, errorStr );
    }

    context_.tracer().debug("driver instantiated",5);
}

void 
MainThread::walk()
{
    activate( context_, this, subsysName() );

    initNetwork();
    initDriver();

    assert( driver_.get() );

    // we are in a different thread now, catch all stray exceptions

    PathPlanner2dTask task; 
    PathPlanner2dData pathData;   

    subStatus().setMaxHeartbeatInterval( 30 );
    subStatus().ok();

    while ( !isStopping() )
    {
        try
        {
            
            //
            //  ======== waiting for a task (blocking) =======
            //
            context_.tracer().info("waiting for a new task");
            bool haveTask=false;
            
            while ( !isStopping() )
            {
                int timeoutMs = 1000;
                int ret=0;
                try {
                    pathPlannerTaskBuffer_.getAndPop( task );
                }
                catch ( const hydroutil::Exception & e ) {
                    ret = pathPlannerTaskBuffer_.getAndPopNext( task, timeoutMs );
                }
                if ( ret==0 ) {
                    haveTask = true;
                    context_.tracer().info("task arrived");  
                    break;
                }
                subStatus().ok();
            }
            
            // the only way of getting out of the above loop without a task
            // is if the user pressed Ctrl+C, ie we have to quit
            if (!haveTask) break;
            
            //
            // ===== tell driver to compute the path ========
            //
            
            // input: ogmap, task; output: path
            try 
            {
                context_.tracer().info("telling driver to compute the path now");
                driver_->computePath( task, pathData );
                pathData.result = orca::PathOk;
                pathData.resultDescription = "All good";
            }
            catch ( hydropathplan::PathStartNotValidException &e )
            {
                std::stringstream ss;
                ss << "Couldn't compute path: " << orcaobj::toVerboseString(task) << endl << "Problem was: " << e.what();
                context_.tracer().error( ss.str() );
                pathData.resultDescription = ss.str();
                pathData.result = orca::PathStartNotValid;
            }
            catch ( hydropathplan::PathDestinationNotValidException &e )
            {
                std::stringstream ss;
                ss << "Couldn't compute path: " << orcaobj::toVerboseString(task) << endl << "Problem was: " << e.what();
                context_.tracer().error( ss.str() );
                pathData.resultDescription = ss.str();
                pathData.result = orca::PathDestinationNotValid;
            }
            catch ( hydropathplan::PathDestinationUnreachableException &e )
            {
                std::stringstream ss;
                ss << "Couldn't compute path: " << orcaobj::toVerboseString(task) << endl << "Problem was: " << e.what();
                context_.tracer().error( ss.str() );
                pathData.resultDescription = ss.str();
                pathData.result = orca::PathDestinationUnreachable;
            }
            catch ( hydropathplan::Exception &e )
            {
                std::stringstream ss;
                ss << "Couldn't compute path: " << orcaobj::toVerboseString(task) << endl << "Problem was: " << e.what();
                context_.tracer().error( ss.str() );
                pathData.resultDescription = ss.str();
                pathData.result = orca::PathOtherError;
            }

            //
            // ======= send result (including error code) ===============
            //
            context_.tracer().info("sending off the resulting path");
            context_.tracer().debug(orcaobj::toVerboseString(pathData));
    
            // There are three methods to let other components know about the computed path:
            // 1. using the proxy
            if (task.prx!=0)
            {
                task.prx->setData( pathData );
            }
            else
            {
                context_.tracer().warning( "task.prx was zero!" );
            }
            // 2. and 3.: use getData or icestorm
            pathPlannerI_->localSetData( pathData );
    
            // resize the pathData: future tasks might not compute a path successfully and we would resend the old path
            pathData.path.resize( 0 );
    
            int numTasksWaiting = pathPlannerTaskBuffer_.size();
            if ( numTasksWaiting > 1 )
            {
                stringstream ss;
                ss << "Tasks are piling up: there are " << numTasksWaiting << " in the queue.";
                subStatus().warning( ss.str() );
            }
            else
            {
                subStatus().ok();
            }
        //
        // unexpected exceptions
        //
        } // try
        catch ( const orca::OrcaException & e )
        {
            stringstream ss;
            ss << "unexpected (remote?) orca exception: " << e << ": " << e.what;
            subStatus().fault( ss.str() );
        }
        catch ( const hydroutil::Exception & e )
        {
            stringstream ss;
            ss << "unexpected (local?) orcaice exception: " << e.what();
            subStatus().fault( ss.str() );
        }
        catch ( const Ice::Exception & e )
        {
            stringstream ss;
            ss << "unexpected Ice exception: " << e;
            subStatus().fault( ss.str() );
        }
        catch ( const std::exception & e )
        {
            // once caught this beast in here, don't know who threw it 'St9bad_alloc'
            stringstream ss;
            ss << "unexpected std exception: " << e.what();
            subStatus().fault( ss.str() );
        }
        catch ( ... )
        {
            subStatus().fault( "unexpected exception from somewhere.");
        }
    
    } // end of while
    
    // wait for the component to realize that we are quitting and tell us to stop.
    waitForStop();
}
