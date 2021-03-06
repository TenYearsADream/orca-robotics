#ifndef GOALPLANNER_PROGRESSMONITOR_H
#define GOALPLANNER_PROGRESSMONITOR_H

#include <orca/pathfollower2d.h>
#include <IceUtil/IceUtil.h>
#include <orcaice/context.h>
#include <gbxsickacfr/gbxiceutilacfr/store.h>

namespace gbxutilacfr { class Stoppable; }

namespace goalplanner {

//
// @brief Keeps track of our current path and how far along we are.
//
// We have to recreate the functionality of orcaiface::ConsumerImpl here
// because PathFollower2dConsumer has non-standard methods.
//
// @author Alex Brooks
//
class ProgressMonitor : public IceUtil::Shared
{

public: 
    friend class PathFollower2dConsumerI;

    // constructor using interfaceTag (may throw ConfigFileException)
    ProgressMonitor( const orcaice::Context& context );
    ~ProgressMonitor();

    // copied from orcaiface::ConsumerImpl
    void subscribeWithTag( const std::string& interfaceTag, 
                          gbxutilacfr::Stoppable* activity, const std::string& subsysName="", 
                          int retryInterval=2, int retryNumber=-1 );
    void subscribeWithString( const std::string& proxyString, 
                          gbxutilacfr::Stoppable* activity, const std::string& subsysName="", 
                          int retryInterval=2, int retryNumber=-1 );
    void unsubscribe();

    //
    // Functions called locally
    //

    // If we're currently following a path (ie both activated and enabled), 
    // returns true and sets currentWp to the waypoint we're currently seeking.
    // Otherwise, returns false.
    bool getCurrentWp( orca::Waypoint2d &currentWp );
    // return val same as getCurrentWp
    bool getCurrentPath( orca::PathFollower2dData &pathData, int &wpIndex );

private: 

    void internalSetState( const orca::PathFollower2dState &state );
    void internalSetData( const orca::PathFollower2dData &data );

    bool followingPath() const;

    bool gotData_;
    orca::PathFollower2dData pathData_;
    orca::PathFollower2dState state_;

    IceUtil::Mutex mutex_;

    // Proxy to the internal consumer interface implementation
    orca::PathFollower2dConsumerPrx consumerPrx_;

    // Hang onto this so we can remove from the adapter and control when things get deleted
    Ice::ObjectPtr consumerPtr_;

    // Store the proxy of the interface after we subscribed to it. 
    gbxiceutilacfr::Store<IceStorm::TopicPrx> topic_;

    // Component context.
    orcaice::Context context_;
};

typedef IceUtil::Handle<ProgressMonitor> ProgressMonitorPtr;

}

#endif
