/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCA_LASER2D_LASERSCANNER2D_IMPL_H
#define ORCA_LASER2D_LASERSCANNER2D_IMPL_H

#include <IceStorm/IceStorm.h>

// include provided interfaces
#include <orca/laserscanner2d.h>

// utilities
#include <orcaice/ptrstore.h>
#include <orcaice/context.h>

namespace hydroiceutil {
    class Thread;
}

namespace orcaifaceimpl {

//!
//! Sets up an implementation of the remote interface, and handles remote calls.
//!
class LaserScanner2dImpl : public IceUtil::Shared
{
friend class LaserScanner2dI;

public:

    //! constructor using interfaceTag (may throw ConfigFileException)
    LaserScanner2dImpl( const orca::RangeScanner2dDescription &descr,
                        const std::string                     &interfaceTag,
                        const orcaice::Context                &context );
    //! constructor using interfaceName
    LaserScanner2dImpl( const orca::RangeScanner2dDescription &descr,
                        const orcaice::Context                &context,
                        const std::string                     &interfaceName );
    ~LaserScanner2dImpl();

    // local interface:
    //! Sets up interface and connects to IceStorm. May throw hydroutil::Exceptions.
    void initInterface();

    //! Sets up interface and connects to IceStorm. Catches all exceptions and retries
    //! until sucessful. Before every attempt checks if the thread was stopped. After every
    //! If a non-empty subsystem name is supplied, sends a Status heartbeat after each attempt 
    //! (@see hydroutil::Status).
    void initInterface( hydroiceutil::Thread* thread, const std::string& subsysName="", int retryInterval=2 );

    //! A local call which sets the data reported by the interface
    void localSet( const orca::LaserScanner2dDataPtr& data );

    //! A local call which sets the data reported by the interface, 
    //! and sends it through IceStorm
    void localSetAndSend( const ::orca::LaserScanner2dDataPtr &data );

private:
    // remote call implementations, mimic (but do not inherit) the orca interface
    ::orca::RangeScanner2dDataPtr     internalGetData() const;
    ::orca::RangeScanner2dDescription internalGetDescription() const;
    void internalSubscribe(const ::orca::RangeScanner2dConsumerPrx& );
    void internalUnsubscribe(const ::orca::RangeScanner2dConsumerPrx& );

    // Holds the latest data
    orcaice::PtrStore<orca::LaserScanner2dDataPtr> dataStore_;

    orca::RangeScanner2dDescription descr_;

    // The topic to which we'll publish
    IceStorm::TopicPrx              topicPrx_;
    // The interface to which we'll publish
    orca::RangeScanner2dConsumerPrx consumerPrx_;

    // Hang onto this so we can remove from the adapter and control when things get deleted
    Ice::ObjectPtr          ptr_;

    const std::string        interfaceName_;
    const std::string        topicName_;
    orcaice::Context   context_;
};
typedef IceUtil::Handle<LaserScanner2dImpl> LaserScanner2dImplPtr;

} // namespace

#endif
