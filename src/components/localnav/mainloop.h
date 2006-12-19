/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */
#ifndef MAINLOOP_H
#define MAINLOOP_H

#include <orca/platform2d.h>
#include <orca/localise2d.h>
#include <orca/rangescanner2d.h>
#include <orcaice/context.h>
#include <orcaice/ptrbuffer.h>
#include <orcaice/thread.h>
#include <orcaice/heartbeater.h>
#include <orcaice/proxy.h>

#include <localnavmanager.h>

namespace localnav {

class PathMaintainer;
class PathFollower2dI;

//
// @brief the main executing loop of this component.
//
// Note: this thing self-destructs when run() returns.
//
//
// @author Alex Brooks
//
class MainLoop : public orcaice::Thread
{

public: 

    MainLoop( LocalNavManager                               &localNavManager,
              orcaice::PtrBuffer<orca::RangeScanner2dDataPtr> &obsBuffer,
              orcaice::PtrBuffer<orca::Localise2dDataPtr>   &locBuffer,
              orcaice::PtrBuffer<orca::Position2dDataPtr>   &odomBuffer,
              orcaice::Proxy<bool>                          &enabledPipe,
              orca::Platform2dPrx                           &platform2dPrx,
              PathMaintainer                                &pathMaintainer,
              orca::PathFollower2dConsumerPrx               &pathPublisher,
              const orcaice::Context & context );
    ~MainLoop();

    virtual void run();

private: 

    // Make sure all our sources of info are OK, and that there's something
    // in all our buffers
    void ensureBuffersNotEmpty();

    // Set the command to 'stop'
    void getStopCommand( orca::Velocity2dCommandPtr cmd );

    // See if we need to follow a new path, plus
    // see if we should update the world on our progress.
    void checkWithOutsideWorld( PathMaintainer &pathMaintainer );

    // Returns true if the timestamps differ by more than a threshold.
    bool areTimestampsDodgy( const orca::RangeScanner2dDataPtr &rangeData,
                             const orca::Localise2dDataPtr   &localiseData,
                             const orca::Position2dDataPtr   &odomData,
                             double                           threshold );

    void maybeSendHeartbeat();

    // The class that does the navigating
    LocalNavManager &localNavManager_;

    // Incoming observations and pose info
    orcaice::PtrBuffer<orca::RangeScanner2dDataPtr> &obsBuffer_;
    orcaice::PtrBuffer<orca::Localise2dDataPtr>   &locBuffer_;
    orcaice::PtrBuffer<orca::Position2dDataPtr>   &odomBuffer_;

    // Allows external enable/disable
    orcaice::Proxy<bool> &enabledPipe_;

    // data types
    orca::Localise2dDataPtr      localiseData_;
    orca::Position2dDataPtr      odomData_;
    orca::RangeScanner2dDataPtr    rangeData_;
    orca::Velocity2dCommandPtr   velocityCmd_;

    // Outgoing commands
    orca::Platform2dPrx &platform2dPrx_;

    // Keeps track of the path we're following
    PathMaintainer  &pathMaintainer_;

    // The object to publish path updates to
    orca::PathFollower2dConsumerPrx              &pathPublisher_;

    orcaice::Heartbeater heartbeater_;

    orcaice::Context context_;
};

}

#endif
