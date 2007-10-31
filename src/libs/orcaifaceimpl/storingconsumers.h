/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
 
#ifndef ORCAIFACEIMPL_STORING_CONSUMERS_H
#define ORCAIFACEIMPL_STORING_CONSUMERS_H

#include <orcaifaceimpl/storingconsumerImpl.h>

#include <orca/rangescanner2d.h>
#include <orca/localise3d.h>
#include <orca/localise2d.h>
#include <orca/odometry3d.h>
#include <orca/odometry2d.h>
#include <orca/drivebicycle.h>
#include <orca/ogmap.h>
#include <orca/gps.h>

namespace orcaifaceimpl
{

//
// An attempt to minimise template kung-fu in component code.
//

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::RangeScanner2dPrx,
                        orca::RangeScanner2dConsumer,
                        orca::RangeScanner2dConsumerPrx,
                        orca::RangeScanner2dDataPtr>       StoringRangeScanner2dConsumerImpl;
typedef IceUtil::Handle<StoringRangeScanner2dConsumerImpl> StoringRangeScanner2dConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::Localise2dPrx,
                        orca::Localise2dConsumer,
                        orca::Localise2dConsumerPrx,
                        orca::Localise2dData>              StoringLocalise2dConsumerImpl;
typedef IceUtil::Handle<StoringLocalise2dConsumerImpl>     StoringLocalise2dConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::Localise3dPrx,
                        orca::Localise3dConsumer,
                        orca::Localise3dConsumerPrx,
                        orca::Localise3dData>              StoringLocalise3dConsumerImpl;
typedef IceUtil::Handle<StoringLocalise3dConsumerImpl>     StoringLocalise3dConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::Odometry2dPrx,
                        orca::Odometry2dConsumer,
                        orca::Odometry2dConsumerPrx,
                        orca::Odometry2dData>              StoringOdometry2dConsumerImpl;
typedef IceUtil::Handle<StoringOdometry2dConsumerImpl>     StoringOdometry2dConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::Odometry3dPrx,
                        orca::Odometry3dConsumer,
                        orca::Odometry3dConsumerPrx,
                        orca::Odometry3dData>              StoringOdometry3dConsumerImpl;
typedef IceUtil::Handle<StoringOdometry3dConsumerImpl>     StoringOdometry3dConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::DriveBicyclePrx,
                        orca::DriveBicycleConsumer,
                        orca::DriveBicycleConsumerPrx,
                        orca::DriveBicycleData>            StoringDriveBicycleConsumerImpl;
typedef IceUtil::Handle<StoringDriveBicycleConsumerImpl>   StoringDriveBicycleConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::OgMapPrx,
                        orca::OgMapConsumer,
                        orca::OgMapConsumerPrx,
                        orca::OgMapData>                   StoringOgMapConsumerImpl;
typedef IceUtil::Handle<StoringOgMapConsumerImpl>          StoringOgMapConsumerImplPtr;

//! Typedef for a common interface type. There's also a corresponding Ptr.
typedef StoringConsumerImpl<orca::GpsPrx,
                        orca::GpsConsumer,
                        orca::GpsConsumerPrx,
                        orca::GpsData>                     StoringGpsConsumerImpl;
typedef IceUtil::Handle<StoringGpsConsumerImpl>            StoringGpsConsumerImplPtr;

} // namespace

#endif
