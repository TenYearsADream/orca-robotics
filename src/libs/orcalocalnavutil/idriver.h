/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef ORCALOCALNAVTIL_LOCALNAVDRIVER_H
#define ORCALOCALNAVTIL_LOCALNAVDRIVER_H

#include <orca/velocitycontrol2d.h>
#include <orca/rangescanner2d.h>
#include <orcaice/context.h>
#include <orca/vehicledescription.h>
#include <hydronavutil/pose.h>

#include <orcalocalnav/goal.h>

namespace orcalocalnavutil {

//
// @author Alex Brooks
//
// Base class for local navigation drivers.
// The manager sets the goal location (in the robot's coordinate system)
// by modifying the goal.
//
class IDriver
{

public: 

    IDriver() {};
    virtual ~IDriver() {};

    // The number of waypoints we look into the future (and therefore
    // expect to be provided with)
    virtual int waypointHorizon() { return 1; }

    //
    // Sets cmd.
    //
    // The pose is in a global coordinate frame, while the goals are
    // in the local coordinate frame.
    //
    virtual void getCommand( bool                                   stalled,
                             bool                                   localisationUncertain,
                             const hydronavutil::Pose               &pose,
                             const orca::Twist2d                   &currentVelocity,
                             const orca::Time                      &poseAndVelocityTime,
                             const orca::RangeScanner2dDataPtr      obs,
                             const std::vector<orcalocalnav::Goal> &goals,
                             orca::VelocityControl2dData           &cmd ) = 0;
};

// Helper class to instantiate drivers
class DriverFactory {
public:
    virtual ~DriverFactory() {};
    virtual IDriver *createDriver( const orcaice::Context &context,
                                   const orca::VehicleDescription &vehicleDescr,
                                   const orca::RangeScanner2dDescription &rangeScannerDescr ) const=0;
};

} // namespace

// Function for dynamically instantiating drivers.
// A driver must have a function like so:
// extern "C" {
//     localnav::DriverFactory *createDriverFactory();
// }
typedef orcalocalnavutil::DriverFactory *DriverFactoryMakerFunc();

#endif