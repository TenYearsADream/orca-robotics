/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCAICE_ICESTORM_UTILITIES_H
#define ORCAICE_ICESTORM_UTILITIES_H

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>

#include <orcaice/context.h>
#include <orcaice/exceptions.h>
#include <orcaice/configutils.h>
#include <hydroutil/sysutils.h>
#include <orcaice/printutils.h>
#include <orcaice/stringutils.h>

namespace orcaice
{
/*!
 *  @name IceStorm-Related Functions
 */
//@{

/*!
Adds an object to the adapter and returns a direct proxy to the object. A
globally unique name is supplied. This function is useful for creating
'consumer' objects, i.e. you subscribe for information to be pushed into
them by specifying the direct proxy to them.

This is a local operation which does not throw any exceptions.

@verbatim
Ice::ObjectPtr consumer = new Odometry2dConsumerI;
orca::Odometry2dConsumerPrx consumerPrx =
    orcaice::createConsumerInterface<orca::Odometry2dConsumerPrx>( context(), consumer );
@endverbatim

This function returns a two-way proxy. You can change the proxy type with standard Ice functions
(search the manual for "proxy factory methods").
@verbatim
consumerPrx = consumerPrx->ice_oneway();
@endverbatim
 */
template<class ConsumerProxyType>
ConsumerProxyType
createConsumerInterface( const Context  & context,
                         Ice::ObjectPtr & consumer )
{
    // create servant and tell adapter about it (let it make up a globally unique name)
    // NOTE: addWithUUID() does not throw exceptions.
    try
    {
        Ice::ObjectPrx obj = context.adapter()->addWithUUID( consumer );

        // make a direct proxy
        Ice::ObjectPrx prx = context.adapter()->createDirectProxy( obj->ice_getIdentity() );
        return ConsumerProxyType::uncheckedCast( prx );
    }
    catch( const Ice::ObjectAdapterDeactivatedException &e )
    {
        std::stringstream ss;
        ss << "orcaice::Component: Failed to create consumer interface because the adapter is destroyed : " << e;
        context.tracer().warning( ss.str() );
        throw orcaice::ComponentDeactivatingException( ERROR_INFO, ss.str() );
    }
}

// FUNCTIONS WITHOUT DOXYGEN TAGS ARE UTILITY FUNCTIONS
// THEY ARE PUBLICLY AVAILABLE BUT ARE NOT ADVERTIZED THROUGH DOXYGEN

/*
 * Behaves the same as the one above but connects to the topic manager
 * specified in the current properties.
 */
IceStorm::TopicPrx connectToIceStormTopicPrx( const Context &,
                                              const std::string & topicName,
                                              bool createIfMissing=false );

/*
 * Behaves the same as the one above but connects to the topic manager
 * specified in the communicator's properties.
 */
IceStorm::TopicPrx connectToIceStormTopicPrx( const Ice::CommunicatorPtr & communicator,
                                              const std::string & topicName,
                                              bool createIfMissing=false );

/*
 * Given the stingified proxy to topic manager and topic name, connect to the topic proxy.
 * If topic manager does not exist, throws Exception exception.
 */
IceStorm::TopicPrx connectToIceStormTopicPrxWithManager( const Ice::CommunicatorPtr & communicator,
                                    const std::string & topicName,
                                    const std::string & topicManagerString,
                                    bool createIfMissing=false );

/*
 * Publisher is used from the provider end. It is the consumer of information.
 * So you can push data into it.
 */
Ice::ObjectPrx connectToIceStormTopicPublisherPrx( const Ice::CommunicatorPtr & communicator,
                                    const std::string & topicName );

/*
 * Behaves like the one above.
 */
Ice::ObjectPrx connectToIceStormTopicPublisherPrx( const IceStorm::TopicPrx & topic );


/*!
Convenience function, behaves like @ref connectToTopicWithString but the proxy information
comes from the configuration file and the @p interfaceTag. 
The appropriate topic name is generated based on the fully-qualified interface name. Default
subtopic is "*".

Throws ConfigFileException if the interface name cannot be read for some reason.
 */
template<class ConsumerProxyType>
IceStorm::TopicPrx
connectToTopicWithTag( const Context           & context,
                       ConsumerProxyType       & publisher,
                       const std::string       & interfaceTag,
                       const std::string       & subtopic="*" )
{
    context.tracer().debug( "orcaice::connectToTopicWithTag() tag="+interfaceTag, 10 );

    // lookup the name of the interface in the config file and generate topic name.
    // this generates a standard topic name based on fully-qualified interface name.
    std::string topicName = orcaice::toString(
                    orcaice::getProvidedTopicWithTag( context, interfaceTag, subtopic ) );

    // do the conversion to string by hand, to cut dependency on libOrcaObj
    // see <orcaobj/stringutils.cpp>
//     orca::FQTopicName name = orcaice::getProvidedTopicWithTag( context, interfaceTag, subtopic );
//     std::string topicName =  name.iface + "/" + name.topic + "@" + name.platform + "/" + name.component;

    return connectToTopicWithString( context, publisher, topicName );
}

/*!
This function is used by potential publishers of information. It returns
a proxy to the IceStorm topic and sets proxy to the topic's @p publisher.
The publisher is used to distribute information to consumers and the
topic is used to subscribe/unsubscribe data consumers.

The address of IceStorm server is looked up in the @c IceStorm.TopicManager.Proxy property.

Raises NetworkException if the server cannot be reached or Exception if there is any other
problem.
 */
template<class ConsumerProxyType>
IceStorm::TopicPrx
connectToTopicWithString( const Context     & context,
                          ConsumerProxyType & publisher,
                          const std::string & topicName )
{
    IceStorm::TopicPrx topicPrx;

    try {
        const bool createIfMissing = true;
        //
        // set the proxy to the topic
        //
        topicPrx = connectToIceStormTopicPrx( context.communicator(), topicName, createIfMissing );

        Ice::ObjectPrx obj = connectToIceStormTopicPublisherPrx( topicPrx );
        //
        // set the proxy to the publisher
        //
        publisher = ConsumerProxyType::uncheckedCast(obj);
    }
    //catch ( const gbxutilacfr::Exception & e ) {
        // we'll catch it here if the topic manager does not exist
    //}
    catch ( Ice::ConnectionRefusedException &e )
    {
        // Give some feedback as to why this isn't working
        std::stringstream ss; ss<<"Error while connecting to IceStorm topic publisher '"<<topicName<<"': "<<e;
        initTracerError( context, ss.str(), 2 );
        initTracerInfo( context, "hint: Is IceStorm running?", 10 );
        throw orcaice::NetworkException( ERROR_INFO, ss.str() );
    }
    catch( const Ice::LocalException &e )
    {
        std::stringstream ss;
        ss<<"Error while connecting to IceStorm topic publisher '"<<topicName<<"': "<<e;
        initTracerError( context, ss.str(), 2 );
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
    catch ( Ice::Exception &e )
    {
        // Give some feedback as to why this isn't working
        std::stringstream ss; ss<<"Error while connecting to IceStorm topic publisher '"<<topicName<<"': "<<e;
        initTracerError( context, ss.str(), 2 );
        throw orcaice::NetworkException( ERROR_INFO, ss.str() );
    }

    return topicPrx;
}

//@}

} // namespace

#endif