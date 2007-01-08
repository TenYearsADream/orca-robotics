/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_ROBOT2D_HARDWARE_HANDLER_H
#define ORCA2_ROBOT2D_HARDWARE_HANDLER_H

#include <orcaice/thread.h>
#include <orcaice/context.h>
#include <orcaice/notify.h>
#include <orcaice/proxy.h>
#include <orcaice/timer.h>

#include "hwdriver.h"

#include <orca/odometry2d.h>
#include <orca/velocitycontrol2d.h>

namespace robot2d
{

class HwHandler : public orcaice::Thread, public orcaice::NotifyHandler<orca::VelocityControl2dData>
{
public:

    HwHandler( orcaice::Buffer<orca::Odometry2dData>& odometryPipe,
               orcaice::Notify<orca::VelocityControl2dData>& commandPipe,
               const orcaice::Context& context );
    virtual ~HwHandler();

    // from Thread
    virtual void run();

    // from PtrNotifyHandler
    virtual void handleData( const orca::VelocityControl2dData & obj );

private:

    // network/hardware interface
    orcaice::Buffer<orca::Odometry2dData>& odometryPipe_;

    // generic interface to the hardware
    HwDriver* driver_;

    struct Config
    {
        bool isMotionEnabled;
        double maxSpeed;
        double maxSideSpeed;
        double maxTurnrate;
    };
    Config config_;

    // component current context
    orcaice::Context context_;

    // write status has to be protected to be accessed from both read and write threads
    orcaice::Proxy<bool> writeStatusPipe_;

    // debug
    orcaice::Timer readTimer_;
    orcaice::Timer writeTimer_;

    // utilities
    static void convert( const HwDriver::Robot2dData& internal, orca::Odometry2dData& network );
    static void convert( const orca::VelocityControl2dData& network, HwDriver::Robot2dCommand& internal );

};

} // namespace

#endif
