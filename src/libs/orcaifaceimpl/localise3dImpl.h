/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_ORCAIFACEIMPL_LOCALISE3D_IMPL_H
#define ORCA2_ORCAIFACEIMPL_LOCALISE3D_IMPL_H

#include <orca/localise3d.h>
#include <IceStorm/IceStorm.h>

// utilities
#include <orcaice/proxy.h>
#include <orcaice/context.h>

namespace orcaice {
    class Thread;
}

namespace orcaifaceimpl {

//!
//! Implements the Localise3d interface: Handles remote calls.
//!
class Localise3dImpl : public IceUtil::Shared
{
friend class Localise3dI;

public:
    //! Constructor
    Localise3dImpl( const orca::VehicleGeometryDescriptionPtr &geometry,
                     const std::string &interfaceTag,
                     const orcaice::Context &context );
    ~Localise3dImpl();

    // Local calls:
    //! Sets up interface and connects to IceStorm. May throw orcaice::Exceptions.
    void initInterface();

    //! Sets up interface and connects to IceStorm. Catches all exceptions and retries
    //! until sucessful. At every iteration, checks if the thread was stopped.
    void initInterface( orcaice::Thread* thread, int retryInterval=2 );

    //! A local call which sets the data reported by the interface
    void localSet( const orca::Localise3dData &data );

    //! A local call which sets the data reported by the interface, 
    //! and sends it through IceStorm
    void localSetAndSend( const orca::Localise3dData &data );

private:
    // remote call implementations, mimic (but do not inherit) the orca interface
    ::orca::Localise3dData internalGetData() const;
    ::orca::VehicleGeometryDescriptionPtr internalGetVehicleGeometry() const;
    void internalSubscribe(const ::orca::Localise3dConsumerPrx&);
    void internalUnsubscribe(const ::orca::Localise3dConsumerPrx&);
    
    orca::VehicleGeometryDescriptionPtr geometry_;

    orcaice::Proxy<orca::Localise3dData> dataProxy_;

    orca::Localise3dConsumerPrx    consumerPrx_;
    IceStorm::TopicPrx             topicPrx_;

    // Hang onto this so we can remove from the adapter and control when things get deleted
    Ice::ObjectPtr          ptr_;

    const std::string              interfaceTag_;
    orcaice::Context               context_;
};
typedef IceUtil::Handle<Localise3dImpl> Localise3dImplPtr;

} // namespace

#endif