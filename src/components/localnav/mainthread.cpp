/*
 * Orca-Robotics Project: Components for robotics
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#include <iostream>
#include <cmath>
#include <orcaice/orcaice.h>
#include <orcaobj/datetime.h>
#include <orcaobj/rangescanner2d.h>
#include <orcaobj/localise2d.h>
#include <orcaobj/vehicledescription.h>
#include <hydroutil/realtimestopwatch.h>
#include <hydroutil/cpustopwatch.h>
#include <orcanavutil/orcanavutil.h>
#include "mainthread.h"
#include <orcalocalnavtest/testsimutil.h>
#include "stats.h"
#include <orcapublish/rangescanner2dpublisher.h>
#include <orcapublish/localise2dpublisher.h>
#include <orcapublish/odometry2dpublisher.h>
#include <orcapublish/ogmappublisher.h>

using namespace std;

namespace localnav {

namespace {

// Returns true if the timestamps differ by more than a threshold.
bool areTimestampsDodgy( const orca::RangeScanner2dDataPtr &rangeData,
                         const orca::Localise2dData        &localiseData,
                         const orca::Odometry2dData        &odomData,
                         double                             thresholdSec )
{
    if ( fabs( orcaice::timeDiffAsDouble( rangeData->timeStamp, localiseData.timeStamp ) ) >= thresholdSec )
        return true;
    if ( fabs( orcaice::timeDiffAsDouble( rangeData->timeStamp, odomData.timeStamp ) ) >= thresholdSec )
        return true;

    return false;
}

}

////////////////////////////////////////////////////////////////////////////////

MainThread::MainThread( const orcaice::Context &context )
    : orcaice::SubsystemThread( context.tracer(), context.status(), "MainThread" ),
      context_(context)
{
}

void
MainThread::initialise()
{
    setMaxHeartbeatInterval( 10.0 );

    clock_.reset( new orcalocalnav::Clock( hydrotime::Time(0,0) ) );

    Ice::PropertiesPtr prop = context_.properties();
    std::string prefix = context_.tag();
    prefix += ".Config.";

    testMode_ = orcaice::getPropertyAsIntWithDefault( prop, prefix+"TestInSimulationMode", 0 );
    timestampsCheckEnabled_ = orcaice::getPropertyAsIntWithDefault( prop, prefix+"TimestampsCheckEnabled", 1 );

    // Create our provided interface
    pathFollowerInterface_.reset( new orcalocalnav::PathFollowerInterface( *clock_, "PathFollower2d", context_ ) );

    //
    // Instantiate bogus info sources in test-in-simulation-mode
    //
    if ( testMode_ )
    {
        string tprefix = prefix+"Test.";

        int seed = orcaice::getPropertyAsIntWithDefault( prop, tprefix+"RandomSeed", 0 );
        srand(seed);

        orcalocalnavtest::Simulator::Config cfg;
        cfg.maxLateralAcceleration =
            orcaice::getPropertyAsDoubleWithDefault( prop, tprefix+"MaxLateralAcceleraton", 1.0 );
        cfg.checkLateralAcceleration =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"CheckLateralAcceleration", 0 );
        cfg.checkDifferentialConstraints =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"CheckDifferentialConstraints", 0 );
        cfg.checkVelocityConstraints =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"CheckVelocityConstraints", 1 );
        cfg.useRoom =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"UseRoom", 1 );
        cfg.batchMode =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"BatchMode", 1 );
        cfg.numIterationsBatch =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"NumIterationsBatch", 0 );
        cfg.numIterationsLimit =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"NumIterationsLimit", 9999 );
        cfg.applyNoises = false;

        int numObstacles =
            orcaice::getPropertyAsIntWithDefault( prop, tprefix+"NumObstacles", 25 );

        const double WORLD_SIZE = 40.0;
        const double CELL_SIZE = 0.1;
        hydroogmap::OgMap ogMap = orcalocalnavtest::setupMap( WORLD_SIZE, CELL_SIZE, numObstacles, cfg.useRoom );

        int numWaypoints = orcaice::getPropertyAsIntWithDefault( prop, prefix+"Test.NumWaypoints", 10 );
        bool turnOnSpot = orcaice::getPropertyAsIntWithDefault( prop, prefix+"Test.TurnOnSpot", 1 );
        bool stressTiming = orcaice::getPropertyAsIntWithDefault( prop, prefix+"Test.StressTiming", 1 );
        bool regularWaypointGrid = orcaice::getPropertyAsIntWithDefault( prop, prefix+"Test.RegularWaypointGrid", 0 );

        orca::PathFollower2dData testPath = orcalocalnavtest::getTestPath( ogMap,
                                                                           numWaypoints,
                                                                           stressTiming,
                                                                           regularWaypointGrid,
                                                                           turnOnSpot,
                                                                           cfg.useRoom );
        pathFollowerInterface_->setData( testPath, true );

        hydropublish::RangeScanner2dPublisherFactoryPtr rangeScanner2dPublisherFactory(
            new orcapublish::RangeScanner2dPublisherFactory( context_ ) );
        hydropublish::Localise2dPublisherFactoryPtr localise2dPublisherFactory(
            new orcapublish::Localise2dPublisherFactory( context_ ) );
        hydropublish::Odometry2dPublisherFactoryPtr odometry2dPublisherFactory(
            new orcapublish::Odometry2dPublisherFactory( context_ ) );
        hydropublish::OgMapPublisherFactoryPtr ogMapPublisherFactory(
            new orcapublish::OgMapPublisherFactory( context_ ) );

        testSimulator_.reset( new orcalocalnavtest::Simulator( *rangeScanner2dPublisherFactory,
                                                               *localise2dPublisherFactory,
                                                               *odometry2dPublisherFactory,
                                                               *ogMapPublisherFactory,
                                                               ogMap,
                                                               testPath,
                                                               cfg ) );

        bool evaluateAlg = orcaice::getPropertyAsIntWithDefault( prop, prefix+"Test.EvaluateAlg", 0 );
        if ( evaluateAlg )
        {
            std::string saveFile = orcaice::getPropertyWithDefault( prop, prefix+"Test.AlgEvalSaveFile", "algEval.txt" );
            algorithmEvaluator_.reset( new AlgorithmEvaluator(saveFile) );
        }
    }

    //
    // Instantiate the driver factory
    //
    std::string driverLibName =
        orcaice::getPropertyWithDefault( prop, prefix+"DriverLib", "libOrcaLocalNavVfh.so" );
    context_.tracer().debug( "Component: Loading driver library "+driverLibName, 4 );
    try {
        // Dynamically load the driver from its library
        driverLib_.reset( new hydrodll::DynamicallyLoadedLibrary(driverLibName) );
        driverFactory_.reset(
            hydrodll::dynamicallyLoadClass<orcalocalnav::DriverFactory,LocalNavDriverFactoryMakerFunc>
            ( *driverLib_, "createLocalNavDriverFactory" ) );
    }
    catch (hydrodll::DynamicLoadException &e)
    {
        context_.tracer().error( e.what() );
        throw;
    }

    health().ok();

    //
    // establish connections
    //
    setup();
}

void
MainThread::work()
{
    setMaxHeartbeatInterval( 1.0 );

    orcalocalnav::IDriver::Inputs inputs;
    inputs.stalled = false;
    inputs.obsRanges.resize( scannerDescr_.numberOfSamples );

    bool prevIsEnabled = pathFollowerInterface_->enabled();

    // a simple class which summarizes motion information
    Stats historyStats;
    std::stringstream historySS;

    while ( !isStopping() )
    {
        try
        {
            checkWithOutsideWorld();

            getInputs( inputs.currentVelocity,
                       inputs.localisePose,
                       inputs.poseTime,
                       inputs.isLocalisationUncertain,
                       inputs.obsRanges,
                       inputs.obsTime );

            // Tell everyone what time it is, boyeee
            clock_->setTime( hydrotime::Time( inputs.obsTime.seconds, inputs.obsTime.useconds ) );

            // Time how long this takes us
            // hydroutil::RealTimeStopwatch timer;
            hydroutil::CpuStopwatch timer;

            // pathMaintainer knows about the whole path in global coords and where
            // we are in that path. So get the next set of current goals in local
            // coord system for the pathplanner.
            // Also return a flag indicating if we have an active goal

            bool wpIncremented = false;
            bool haveGoal = pathMaintainer_->getActiveGoals( inputs.goals,
                                                             driver_->waypointHorizon(),
                                                             inputs.localisePose,
                                                             wpIncremented );

            if ( haveGoal )
            {
                if ( wpIncremented )
                {
                    assert( inputs.goals.size() > 0 );
                    speedLimiter_->setIntendedSpeedThisLeg( inputs.goals[0] );
                }

                // If we do have an active goal, limit the max speed for the current goal
                // and get the pathplanner to work out the next set of actions
                speedLimiter_->constrainMaxSpeeds( inputs.goals[0], inputs.currentVelocity.lin() );
            }

            // The actual driver which determines the path and commands to send to the vehicle.
            // The odometry is required for the velocity, which isn't contained
            // in Localise2d.
            hydronavutil::Velocity velocityCmd;
            try {
                velocityCmd = driver_->getCommand( inputs );
            }
            catch ( std::exception &e )
            {
                stringstream ss;
                ss << "(while getting command from driver with inputs: " << orcalocalnav::toString(inputs) << ") failed: " << e.what();
                throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
            }


            if ( algorithmEvaluator_.get() )
                timer.stop();

            // For big debug levels, give feedback through tracer.
            {
                std::stringstream ss;
                ss << "MainThread: Setting command: " << velocityCmd;
                context_.tracer().debug( ss.str(), 5 );
            }

            // send zero stop-command as soon we "take our hands of the wheel"
            // (otherwise the platform will execute the last command for a while)
            // Note that we call enabled() only once in order to avoid possibility of state
            // change between our calls.
            bool isEnabled = pathFollowerInterface_->enabled();
            bool isJustDisabled = ( prevIsEnabled && !isEnabled );
            if ( isJustDisabled ) {
                hydronavutil::Velocity zeroVelocityCmd( 0.0, 0.0 );
                sendCommandToPlatform( zeroVelocityCmd );
            }
            prevIsEnabled = isEnabled;

            // keep track of our approximate motion for history
            historyStats.addData( inputs.poseTime.seconds, inputs.poseTime.useconds,
                           inputs.localisePose, inputs.currentVelocity,
                           isEnabled );
            historySS.str(" ");
            historySS << historyStats.distance()<<" "<<historyStats.timeInMotion()<<" "<<historyStats.maxSpeed();
            context_.history().report( historySS.str() );

            // Only send the command if we're enabled.
            if ( isEnabled ) {
                sendCommandToPlatform( velocityCmd );
            }
            else {
                context_.tracer().debug( "Doing nothing because disabled" );
                health().ok();
                continue;
            }

            std::stringstream timerSS;
            timerSS << "MainThread: time to make and send decision: " << timer.elapsedSeconds()*1000.0 << "ms";
            context_.tracer().debug( timerSS.str(), 3 );

            if ( testMode_ && algorithmEvaluator_.get() )
            {
                algorithmEvaluator_->evaluate( timer.elapsedSeconds(), *testSimulator_ );
            }

            if ( testMode_ )
            {
                bool pathCompleted, pathFailed;
                testSimulator_->checkProgress( pathCompleted, pathFailed );
                if ( pathCompleted )
                {
                    cout<<"TRACE(mainthread.cpp): test PASSED" << endl;
                    context_.shutdown();
                    break;
                }
                if ( pathFailed )
                {
                    cout << "ERROR(mainthread.cpp): test FAILED" << endl;
                    exit(1);
                }
            }

            if ( inputs.isLocalisationUncertain )
                health().warning( "Localisation is uncertain, but everything else is OK." );
            else
                health().ok();
        } // try
        catch ( orca::EStopTriggeredException &e )
        {
            // This shouldn't generate a fault
            stringstream ss;
            // Make this possibly-frequent warning a bit more human-readable than the others
            ss << "Emergency stop: " << e.what;
            health().warning( ss.str() );
        }
        catch ( ... )
        {
            // before doing anything else, stop the vehicle!
            stopVehicle();

            orcaice::catchMainLoopExceptions( health() );
        }
    }
}

///////////////////

void
MainThread::ensureProxiesNotEmpty()
{
    // Ensure that there's something in our proxys
    while ( !isStopping() )
    {
        bool gotObs  = !obsConsumer_->store().isEmpty();
        bool gotLoc  = !locConsumer_->store().isEmpty();
        bool gotOdom = !odomConsumer_->store().isEmpty();


        if ( gotObs && gotLoc && gotOdom )
        {
            context_.tracer().info( "Received at least one data item from every provided interface." );
            return;
        }
        else
        {
            stringstream ss;
            ss << "Still waiting for intial data to arrive.  gotObs="<<gotObs<<", gotLoc="<<gotLoc<<", gotOdom="<<gotOdom;
            context_.tracer().warning( ss.str() );
            health().heartbeat();
            sleep(1);
        }
    }
}

void
MainThread::getVehicleDescription()
{
    while ( !isStopping() )
    {
        try
        {
            vehicleDescr_ = velControl2dPrx_->getDescription();
            break;
        }
        catch ( ... ) {
            orcaice::catchExceptionsWithStatusAndSleep( "getting vehicle description", health() );
        }
    }
}

void
MainThread::getRangeScannerDescription()
{
    orcaice::getDescriptionWithTag<orca::RangeScanner2dPrx,orca::RangeScanner2dDescription>
        ( context_, "Observations", scannerDescr_, this, subsysName() );
}

void
MainThread::setup()
{
    if ( !testMode_ )
    {
        context_.tracer().debug( "Connecting to VelocityControl2d" );
        orcaice::connectToInterfaceWithTag( context_, velControl2dPrx_, "VelocityControl2d", this, subsysName() );
        // check for stop signal after retuning from multi-try
        if ( isStopping() )
            return;

        context_.tracer().debug( "Subscribing to Odometry2d" );
        odomConsumer_ = new orcaifaceimpl::StoringOdometry2dConsumerImpl(context_);
        odomConsumer_->subscribeWithTag( "Odometry2d", this, subsysName() );

        context_.tracer().debug( "Subscribing to Localise2d" );
        locConsumer_ = new orcaifaceimpl::StoringLocalise2dConsumerImpl(context_);
        locConsumer_->subscribeWithTag( "Localisation", this, subsysName() );

        context_.tracer().debug( "Subscribing to RangeScanner2d" );
        obsConsumer_ = new orcaifaceimpl::StoringRangeScanner2dConsumerImpl(context_);
        obsConsumer_->subscribeWithTag( "Observations", this, subsysName() );

        context_.tracer().debug( "Getting vehicleDescription" );
        getVehicleDescription();

        context_.tracer().debug( "Getting scannerDescription" );
        getRangeScannerDescription();

        ensureProxiesNotEmpty();

        // Set the initial time
        orca::RangeScanner2dDataPtr obs;
        obsConsumer_->store().get( obs );
        clock_->setTime( hydrotime::Time( obs->timeStamp.seconds, obs->timeStamp.useconds ) );
    }
    else
    {
        vehicleDescr_ = testSimulator_->vehicleDescription();
        scannerDescr_ = testSimulator_->rangeScanner2dDescription();
        clock_->setTime( testSimulator_->time() );
    }

    stringstream descrStream;
    descrStream << "Working with the following vehicle: " << orcaobj::toString(vehicleDescr_) << endl;
    descrStream << "Working with the following range scanner: " << orcaobj::toString(scannerDescr_) << endl;
    context_.tracer().info( descrStream.str() );

    pathMaintainer_.reset( new orcalocalnav::PathMaintainer( *pathFollowerInterface_, *clock_, context_ ) );
    speedLimiter_.reset( new orcalocalnav::SpeedLimiter( context_ ) );

    //
    // Instantiate the driver
    //
    driver_.reset( driverFactory_->createDriver( context_, vehicleDescr_, scannerDescr_ ) );

    //
    // Enable our external interface
    //
    pathFollowerInterface_->initInterface( this, subsysName() );
}

void
MainThread::stopVehicle()
{
    //
    // Loop around this -- it might be an emergency, so we want to be certain.
    //
    while ( !isStopping() )
    {
        std::stringstream exceptionSS;
        try {
            sendCommandToPlatform( hydronavutil::Velocity(0,0) );
            break;
        }
        catch ( Ice::CommunicatorDestroyedException &e )
        {
            // This is OK: it means that the communicator shut down (eg via Ctrl-C)
        }
        catch ( ... ) {
            orcaice::catchExceptionsWithStatusAndSleep( "getting vehicle description", health(), gbxutilacfr::SubsystemFault, 500 );
        }
    }
}

void
MainThread::sendCommandToPlatform( const hydronavutil::Velocity &cmd )
{
    if ( testMode_ )
    {
        testSimulator_->act( cmd );
    }
    else
    {
        velControl2dPrx_->setCommand( orcanavutil::convert(cmd) );
    }
}

void
MainThread::getInputs( hydronavutil::Velocity &velocity,
                       hydronavutil::Pose     &localisePose,
                       orca::Time             &poseTime,
                       bool                   &isLocalisationUncertain,
                       std::vector<float>     &obsRanges,
                       orca::Time             &obsTime )
{
    if ( testMode_ )
    {
        velocity                = testSimulator_->velocity();
        localisePose            = testSimulator_->pose();
        isLocalisationUncertain = false;
        obsTime.seconds         = testSimulator_->time().seconds();
        obsTime.useconds        = testSimulator_->time().useconds();
        poseTime.seconds        = testSimulator_->time().seconds();
        poseTime.useconds       = testSimulator_->time().useconds();
        testSimulator_->getObsRanges( obsRanges );
    }
    else
    {
        // The rangeScanner provides the 'clock' which triggers new action.
        const int TIMEOUT_MS = 1000;
        int sensorRet = obsConsumer_->store().getNext( orcaRangeData_, TIMEOUT_MS );
        if ( sensorRet != 0 )
        {
            if ( isStopping() || context_.isDeactivating() ) {
               throw orcaice::ComponentDeactivatingException( ERROR_INFO, "Failed to get inputs because the component is deactivating" );
            }

            stringstream ss;
            ss << "Timeout waiting for range data: no data for " << TIMEOUT_MS << "ms.";
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        }
        locConsumer_->store().get( orcaLocaliseData_ );
        odomConsumer_->store().get( orcaOdomData_ );

        if (timestampsCheckEnabled_)
        {
            // Check timestamps are in sync
            const double TS_PROXIMITY_THRESHOLD = 1.0; // seconds
            if ( areTimestampsDodgy( orcaRangeData_, orcaLocaliseData_, orcaOdomData_, TS_PROXIMITY_THRESHOLD ) )
            {
                stringstream ss;
                ss << "Timestamps are more than "<<TS_PROXIMITY_THRESHOLD<<"sec apart: " << endl
                << "\t rangeData:    " << orcaobj::toString(orcaRangeData_->timeStamp) << endl
                << "\t localiseData: " << orcaobj::toString(orcaLocaliseData_.timeStamp) << endl
                << "\t odomData:     " << orcaobj::toString(orcaOdomData_.timeStamp) << endl
                << "Maybe something is wrong: Stopping.";
                throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
            }
        }
        else
        {
            // Enforce equality of timestamps
            orcaLocaliseData_.timeStamp = orcaRangeData_->timeStamp;
            orcaOdomData_.timeStamp     = orcaRangeData_->timeStamp;
        }

        //
        // Convert
        //
        velocity = orcanavutil::convertToVelocity( orcaOdomData_ );
        localisePose = orcanavutil::convertToPose( orcaLocaliseData_ );
        poseTime = orcaLocaliseData_.timeStamp;
        obsRanges = orcaRangeData_->ranges;
        obsTime  = orcaRangeData_->timeStamp;
        isLocalisationUncertain = orcaobj::localisationIsUncertain( orcaLocaliseData_ );
    }
}

void
MainThread::checkWithOutsideWorld()
{
    pathMaintainer_->checkWithOutsideWorld();
}

}
