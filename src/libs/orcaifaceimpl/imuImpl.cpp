/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>

#include <orcaice/orcaice.h>
#include "imuImpl.h"
#include "util.h"

using namespace std;

namespace orcaifaceimpl {

//////////////////////////////////////////////////////////////////////

//
// This is the implementation of the slice-defined interface
//
class ImuI : public orca::Imu
{
public:
    //! constructor
    ImuI( ImuImpl &impl )
        : impl_(impl) {}
    virtual ~ImuI() {}

    // remote interface

    virtual ::orca::ImuData getData(const ::Ice::Current& ) const
        { return impl_.internalGetData(); }

    virtual ::orca::ImuDescription getDescription(const ::Ice::Current& ) const
        { return impl_.internalGetDescription(); }

    virtual void subscribe(const ::orca::ImuConsumerPrx& consumer,
                           const ::Ice::Current& = ::Ice::Current())
        { impl_.internalSubscribe( consumer ); }

    virtual void unsubscribe(const ::orca::ImuConsumerPrx& consumer,
                             const ::Ice::Current& = ::Ice::Current())
        { impl_.internalUnsubscribe(consumer); }

private:
    ImuImpl &impl_;
};

//////////////////////////////////////////////////////////////////////

ImuImpl::ImuImpl( const orca::ImuDescription &descr,
                                const std::string              &interfaceTag,
                                const orcaice::Context         &context )
    : descr_(descr),
      interfaceName_(getInterfaceNameFromTag(context,interfaceTag)),
      topicName_(getTopicNameFromInterfaceName(context,interfaceName_)),
      context_(context)
{
}

ImuImpl::ImuImpl( const orca::ImuDescription &descr,
                                const orcaice::Context         &context,
                                const std::string              &interfaceName )
    : descr_(descr),
      interfaceName_(interfaceName),
      topicName_(getTopicNameFromInterfaceName(context,interfaceName)),
      context_(context)
{
}

ImuImpl::~ImuImpl()
{
    tryRemoveInterface( context_, interfaceName_ );
}

void
ImuImpl::initInterface()
{
    // Find IceStorm Topic to which we'll publish
    topicPrx_ = orcaice::connectToTopicWithString<orca::ImuConsumerPrx>
        ( context_, consumerPrx_, topicName_ );

    // Register with the adapter
    // We don't have to clean up the memory we're allocating here, because
    // we're holding it in a smart pointer which will clean up when it's done.
    ptr_ = new ImuI( *this );
    orcaice::createInterfaceWithString( context_, ptr_, interfaceName_ );
}

void 
ImuImpl::initInterface( hydroiceutil::Thread* thread, const std::string& subsysName, int retryInterval )
{
    topicPrx_ = orcaice::connectToTopicWithString<orca::ImuConsumerPrx>
        ( context_, consumerPrx_, topicName_, thread, subsysName, retryInterval );

    ptr_ = new ImuI( *this );
    orcaice::createInterfaceWithString( context_, ptr_, interfaceName_, thread, subsysName, retryInterval );
}

::orca::ImuData 
ImuImpl::internalGetData() const
{
    context_.tracer().debug( "ImuImpl::internalGetData()", 5 );

    if ( dataStore_.isEmpty() )
    {
        std::stringstream ss;
        ss << "No data available! (interface="<<interfaceName_<<")";
        throw orca::DataNotExistException( ss.str() );
    }

    orca::ImuData data;
    dataStore_.get( data );
    return data;
}

::orca::ImuDescription
ImuImpl::internalGetDescription() const
{
    return descr_;
}

void 
ImuImpl::internalSubscribe(const ::orca::ImuConsumerPrx& subscriber )
{
    context_.tracer().debug( "ImuImpl::internalSubscribe(): subscriber='"+subscriber->ice_toString()+"'", 4 );
    try {
        topicPrx_->subscribeAndGetPublisher( IceStorm::QoS(), subscriber->ice_twoway() );
    }
    catch ( const IceStorm::AlreadySubscribed & e ) {
        std::stringstream ss;
        ss <<"Request for subscribe but this proxy has already been subscribed, so I do nothing: "<< e;
        context_.tracer().debug( ss.str(), 2 );
    }
    catch ( const Ice::Exception & e ) {
        std::stringstream ss;
        ss <<"ImuImpl::internalSubscribe: failed to subscribe: "<< e << endl;
        context_.tracer().warning( ss.str() );
        throw orca::SubscriptionFailedException( ss.str() );
    }
}

void 
ImuImpl::internalUnsubscribe(const ::orca::ImuConsumerPrx& subscriber)
{
    context_.tracer().debug( "ImuImpl::internalSubscribe(): subscriber='"+subscriber->ice_toString()+"'", 4 );
    topicPrx_->unsubscribe( subscriber );
}

void
ImuImpl::localSet( const orca::ImuData& data )
{
    dataStore_.set( data );
}

void
ImuImpl::localSetAndSend( const orca::ImuData& data )
{
//     cout<<"TRACE(ImuIface.cpp): localSetAndSend: " << orcaice::toString(data) << endl;

    dataStore_.set( data );

    // Try to push to IceStorm.
    tryPushToIceStormWithReconnect<orca::ImuConsumerPrx,orca::ImuData>
        ( context_,
          consumerPrx_,
          data,
          topicPrx_,
          interfaceName_,
          topicName_ );
}

} // namespace