#include "pathfollowerinterface.h"
#include <iostream>
#include <cmath>
#include <orcaobj/orcaobj.h>
#include <orcaice/orcaice.h>

using namespace std;

namespace localnav {

namespace {
    
    bool isDodgy( const orca::PathFollower2dData &pathData, std::string &reason )
    {
        std::stringstream ss;
        bool dodgy=false;
        for ( unsigned int i=0; i < pathData.path.size(); i++ )
        {
            const orca::Waypoint2d &wp = pathData.path[i];

            if ( wp.distanceTolerance < 1e-3 )
            {
                ss << "Waypoint " << i << ": tiny distance tolerance: " 
                   << wp.distanceTolerance << "m" << endl;
                dodgy = true;
            }
            if ( wp.headingTolerance < 1e-3 )
            {
                ss << "Waypoint " << i << ": tiny heading tolerance: " 
                   << wp.headingTolerance*180.0/M_PI << "deg" << endl;
                dodgy = true;
            }
            if ( wp.maxApproachSpeed < 1e-3 )
            {
                ss << "Waypoint " << i << ": tiny maxApproachSpeed: " 
                   << wp.maxApproachSpeed << "m/s" << endl;
                dodgy = true;
            }
            if ( wp.maxApproachTurnrate < 1e-3 )
            {
                ss << "Waypoint " << i << ": tiny maxApproachTurnrate: " 
                   << wp.maxApproachTurnrate*180.0/M_PI << "deg/s" << endl;
                dodgy = true;
            }
        }
        reason = ss.str();
        return dodgy;
    }

}

////////////////////////////////////////////////////////////////////////////////

PathFollowerInterface::PathFollowerInterface( const Clock &clock,
                                              const std::string &interfaceTag,
                                              const orcaice::Context &context )
    : pathFollower2dImpl_(new orcaifaceimpl::PathFollower2dImpl(*this,interfaceTag,context)),
      clock_(clock),
      context_(context)
{    
    // We're inactive on initialization
    wpIndexStore_.set( -1 );

    // But enabled
    enabledStore_.set( true );
}

void
PathFollowerInterface::setData( const orca::PathFollower2dData &pathData, bool activateImmediately )
{
    // Sanity check
    std::string reason;
    if ( !orcaobj::isSane( pathData, reason ) )
    {
        stringstream ss;
        ss << "PathFollower2dI: Received screwy path: " << orcaobj::toString(pathData) << endl << reason;
        context_.tracer().warning( ss.str() );
        throw orca::MalformedParametersException( reason );
    }

    if ( isDodgy( pathData, reason ) )
    {
        stringstream ss;
        ss << "Received dodgy path: " << orcaobj::toVerboseString(pathData) << endl << reason;
        context_.tracer().warning( ss.str() );
    }

    cout<<"TRACE(pathfollower2dI.cpp): Received new path: " << orcaobj::toVerboseString(pathData) << endl;
    cout<<"TRACE(pathfollower2dI.cpp): activateImmediately: " << activateImmediately << endl;

    pendingPathRequestStore_.set( pathData );
    if ( activateImmediately )
        activateNow();
}

double
PathFollowerInterface::timeSinceActivate( const orca::Time &activationTime )
{
    double timeSince = orcaice::timeDiffAsDouble( clock_.time(), activationTime );
    return timeSince;
}

void
PathFollowerInterface::activateNow()
{
    orca::Time now = clock_.time();
    pendingActivationRequestStore_.set( now );
}

int
PathFollowerInterface::getWaypointIndex() const
{
    int ret;
    wpIndexStore_.get( ret );
    return ret;
}

bool
PathFollowerInterface::getAbsoluteActivationTime( orca::Time &activationTime ) const
{
    int wpIndex;
    wpIndexStore_.get( wpIndex );
    if ( wpIndex != -1 )
    {
        activationTimeStore_.get( activationTime );
        return true;
    }
    else
        return false;
}

bool
PathFollowerInterface::getRelativeActivationTime( double &secondsSinceActivation ) const
{
    int wpIndex;
    wpIndexStore_.get( wpIndex );
    if ( wpIndex != -1 )
    {
        orca::Time timeActivated;
        activationTimeStore_.get( timeActivated );
        secondsSinceActivation = orcaice::timeDiffAsDouble( clock_.time(), timeActivated );
        return true;
    }
    else
        return false;
}

void
PathFollowerInterface::setEnabled( bool enabled )
{
    enabledStore_.set( enabled );
    // let the consumers know that the enabled state changed
    pathFollower2dImpl_->localSetEnabledState( enabled );
}

bool
PathFollowerInterface::enabled() const
{
    bool enabled;
    enabledStore_.get( enabled );
    return enabled;
}

void 
PathFollowerInterface::updateWaypointIndex( int index )
{
    wpIndexStore_.set( index );
    pathFollower2dImpl_->localSetWaypointIndex( index );
}

void
PathFollowerInterface::serviceRequests( bool                     &gotNewPath,
                                        orca::PathFollower2dData &pathData,
                                        bool                     &gotActivation,
                                        orca::Time               &pathStartTime )
{
    gotNewPath    = false;
    gotActivation = false;

    if ( pendingPathRequestStore_.isNewData() )
    {
        gotNewPath = true;
        pendingPathRequestStore_.get( pathData );

        pathFollower2dImpl_->localSetAndSend( pathData );
    }
    if ( pendingActivationRequestStore_.isNewData() )
    {
        gotActivation = true;
        pendingActivationRequestStore_.get( pathStartTime );

        activationTimeStore_.set( pathStartTime );
        wpIndexStore_.set( 0 );
        pathFollower2dImpl_->localSetWaypointIndex(0);
        pathFollower2dImpl_->localSetActivationTime( pathStartTime, timeSinceActivate(pathStartTime) );
    }
}

}
