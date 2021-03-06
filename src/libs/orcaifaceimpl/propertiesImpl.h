/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCA_PROPERTIES_IMPL_H
#define ORCA_PROPERTIES_IMPL_H

#include <memory>
#include <orca/properties.h>
#include <gbxsickacfr/gbxiceutilacfr/store.h>
#include <orcaice/context.h>
#include <orcaice/topichandler.h>

namespace gbxutilacfr { class Stoppable; }

namespace orcaifaceimpl {

//!
//! Implements the orca::Properties interface: Handles remote calls.
//!
class PropertiesImpl : public IceUtil::Shared
{
friend class PropertiesI;

public:
    //! constructor using interfaceTag (may throw ConfigFileException)
    PropertiesImpl( const std::string& interfaceTag, 
               const orcaice::Context& context );
    //! constructor using interfaceName
    PropertiesImpl( const orcaice::Context& context,
               const std::string& interfaceName );
    ~PropertiesImpl();

    // local interface:
    //! May throw gbxutilacfr::Exceptions.
    void initInterface();

    //! Sets up interface and connects to IceStorm. Catches all exceptions and retries
    //! until sucessful. At every iteration, checks if the activity (e.g. thread) was stopped.
    void initInterface( gbxutilacfr::Stoppable* activity, const std::string& subsysName="", int retryInterval=2 );

    //! A local call which sets the data reported by the interface
    void localSet( const orca::PropertiesData& data );

    //! A local call which sets the data reported by the interface, 
    //! and sends it through IceStorm
    void localSetAndSend( const orca::PropertiesData& data );

    //! A local call which gets the Store that holds remote 'setData' requests.
    gbxiceutilacfr::Store<orca::PropertiesData> &localGetRemotelySetDataStore()
        { return remotelySetDataStore_; }

private:
    void init();

    // remote call implementations, mimic (but do not inherit) the orca interface
    ::orca::PropertiesData internalGetData();
    void internalSetData( const ::orca::PropertiesData &data );
    IceStorm::TopicPrx internalSubscribe(const orca::PropertiesConsumerPrx& subscriber);

    gbxiceutilacfr::Store<orca::PropertiesData> dataStore_;
    gbxiceutilacfr::Store<orca::PropertiesData> remotelySetDataStore_;

    typedef orcaice::TopicHandler<orca::PropertiesConsumerPrx,orca::PropertiesData> PropertiesTopicHandler;
    std::auto_ptr<PropertiesTopicHandler> topicHandler_;

    Ice::ObjectPtr ptr_;
    const std::string interfaceName_;
    orcaice::Context context_;
};

typedef IceUtil::Handle<PropertiesImpl> PropertiesImplPtr;

}

#endif
