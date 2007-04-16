/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_ORCAIFACEIMPL_DRIVEBICYCLE_IFACE_H
#define ORCA2_ORCAIFACEIMPL_DRIVEBICYCLE_IFACE_H

#include <orca/drivebicycle.h>
// #include <orca/vehicledescription.h>
#include <IceStorm/IceStorm.h>

// utilities
#include <orcaice/proxy.h>
#include <orcaice/context.h>

namespace orcaifaceimpl {

//!
//! Implements the DriveBicycle interface: Handles remote calls.
//!
class DriveBicycleIface : public IceUtil::Shared
{
public:
    DriveBicycleIface( const std::string &ifaceTag,
                       orcaice::Proxy<orca::DriveBicycleData>& commandProxy,
                       const orca::VehicleDescription& descr,
                       const orcaice::Context &context );
    ~DriveBicycleIface();

    // remote calls:

    ::orca::DriveBicycleData getData() const;

    void subscribe(const ::orca::DriveBicycleConsumerPrx& );

    void unsubscribe(const ::orca::DriveBicycleConsumerPrx& );

    orca::VehicleDescription getDescription() const
        { return description_; }

    void setCommand( const ::orca::DriveBicycleData& );

    // Local calls:

    //! may throw orcaice::Exceptions
    void initInterface();

    //! A local call which sets the data reported by the interface
    void localSet( const orca::DriveBicycleData &data );

    //! A local call which sets the data reported by the interface, 
    //! and sends it through IceStorm
    void localSetAndSend( const orca::DriveBicycleData &data );

private:
    
    // outgoing data
    orcaice::Proxy<orca::DriveBicycleData> dataProxy_;

    orca::DriveBicycleConsumerPrx    consumerPrx_;
    IceStorm::TopicPrx             topicPrx_;

    // Hang onto this so we can remove from the adapter and control when things get deleted
    Ice::ObjectPtr          ptr_;

    const std::string                 ifaceTag_;
    // incoming data
    orcaice::Proxy<orca::DriveBicycleData>& commandProxy_;
    const orca::VehicleDescription    description_;
    orcaice::Context                  context_;
};
typedef IceUtil::Handle<DriveBicycleIface> DriveBicycleIfacePtr;

} // namespace

#endif