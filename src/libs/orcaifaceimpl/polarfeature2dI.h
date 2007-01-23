/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_IFACEIMPL_POLARFEATURE2D_I_H
#define ORCA2_IFACEIMPL_POLARFEATURE2D_I_H

#include <IceStorm/IceStorm.h>

// include provided interfaces
#include <orca/polarfeature2d.h>

// utilities
#include <orcaice/ptrproxy.h>
#include <orcaice/context.h>

namespace orcaifaceimpl {

//!
//! Implements the interface
//!
class PolarFeature2dI : public virtual orca::PolarFeature2d
{
public:
    PolarFeature2dI( const std::string                     &ifaceTag,
                     const orcaice::Context                &context );

    //
    // Remote calls:
    //

    virtual ::orca::PolarFeature2dDataPtr     getData(const ::Ice::Current& ) const;

    virtual void subscribe(const ::orca::PolarFeature2dConsumerPrx&, const ::Ice::Current& = ::Ice::Current());

    virtual void unsubscribe(const ::orca::PolarFeature2dConsumerPrx&, const ::Ice::Current& = ::Ice::Current());


    //
    // Local calls:
    //

    //! may throw orcaice::Exceptions
    void initInterface();

    //! A local call which sets the data reported by the interface
    void localSet( const orca::PolarFeature2dDataPtr& data );

    //! A local call which sets the data reported by the interface, 
    //! and sends it through IceStorm
    void localSetAndSend( const ::orca::PolarFeature2dDataPtr &data );

private:

    // Holds the latest data
    orcaice::PtrProxy<orca::PolarFeature2dDataPtr> dataProxy_;

    // The topic to which we'll publish
    IceStorm::TopicPrx              topicPrx_;
    // The interface to which we'll publish
    orca::PolarFeature2dConsumerPrx consumerPrx_;

    std::string                     ifaceTag_;
    orcaice::Context                context_;
};

} // namespace

#endif
