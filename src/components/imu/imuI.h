/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_IMU_I_H
#define ORCA2_IMU_I_H

#include <IceStorm/IceStorm.h>

// include provided interfaces
#include <orca/imu.h>

// for context()
#include <orcaice/orcaice.h>

// utilities
#include <orcaice/ptrbuffer.h>

namespace imu {

//
// Implements the Laser interface: Handles all our remote calls.
//
// The component interacts with this thing through the (thread-safe) buffers.
//
class ImuI : public orca::Imu
{
public:
    ImuI( const orca::ImuDescription& descr,
            const orcaice::Context & context);

    // remote calls:

    virtual ::orca::ImuData     getData(const ::Ice::Current& ) const;
    
    virtual ::orca::ImuDescription getDescription(const ::Ice::Current& ) const;

    virtual void subscribe(const ::orca::ImuConsumerPrx&, const ::Ice::Current& = ::Ice::Current());

    virtual void unsubscribe(const ::orca::ImuConsumerPrx&, const ::Ice::Current& = ::Ice::Current());

    // local
    void localSetData( const ::orca::ImuData& data );
    orca::ImuDescription localGetDescription() const;

private:

    // the driver will put the latest raw data into this buffer
    orcaice::Buffer<orca::ImuData> imuDataBuffer_;
    
    //publishers
    orca::ImuConsumerPrx imuPublisher_;

    //topics
    IceStorm::TopicPrx topicPrx_;

    orca::ImuDescription descr_;

    orcaice::Context context_;
};

} //namespace

#endif