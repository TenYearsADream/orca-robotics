/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCAICE_CONNECT_UTILITIES_H
#define ORCAICE_CONNECT_UTILITIES_H

#include <orcaice/context.h>
#include <orcaice/exceptions.h>
#include <orcaice/configutils.h>
#include <orcaice/printutils.h>
#include <orcaice/stringutils.h>

namespace orcaice
{
/*!
 *  @name Connection Functions
 */
//@{

/*!
Tries to reach the remote interface specified with proxyString by calling
ice_ping() function. Returns TRUE if the ping was successful and FALSE if not.
Writes diagnostic information into @c diagnostic string.
 
Catches all exceptions. Does not throw.  
  
Implementation note: this function does not need to be templated because
ice_ping() is implemented by all Ice objects regardless of type.
 */
bool isInterfaceReachable( const Context& context, const std::string& proxyString, std::string& diagnostic );

//! Tries to connect the interface described in the config file and gets its ID (Slice object type).
//! Throws NetworkException if the interface is unreachable.
std::string getInterfaceIdWithString( const Context& context, const std::string& proxyString );

//! Tries to connect the interface described in the config file and gets its ID (Slice object type).
//! Throws NetworkException if the interface is unreachable.
//! Throws ConfigFileException if there's a problem with reading from the config file.
std::string getInterfaceIdWithTag( const Context& context, const std::string& interfaceTag );

//
//  Implementation Note:
//  the two 'connectToInterface' functions have to be templated, unfortunately,
//  to ensure we're connecting to the right type.
/*!
Create an Ice proxy @p proxy for the remote server based on a stringified proxy @p proxyString.
The proxy can be direct or indirect. For indirect proxies with platform name set to @e local, 
hostname is used as the platform name.

Throws TypeMismatchException if fails to connect to the remote interface or if
it is of the wrong type.

In the event of a failed connection, adds an interpretive message and re-throws a NetworkException.

Example: to connect to interface of type MyInterface, use
@verbatim
MyInterfacePrx myInterfacePrx;
orcaice::connectToInterfaceWithString( context_, myInterfacePrx, "iface@platform/component" );
@endverbatim
 */
template<class InterfaceType>
void
connectToInterfaceWithString( const Context                             &context,
                              ::IceInternal::ProxyHandle<InterfaceType> &proxy,
                              const std::string                         &proxyString)
{
    // for indirect proxies only: if platform is set to 'local', replace it by hostname
    std::string resolvedProxyString = resolveLocalPlatform( context, proxyString );

    Ice::ObjectPrx base = context.communicator()->stringToProxy( resolvedProxyString );

    // check with the server that the one we found is of the right type
    // the check operation is remote and may fail (see sec.6.11.2)
    try {
        proxy = ::IceInternal::ProxyHandle<InterfaceType>::checkedCast( base );
        // got some answer, check that it's the right type
        if ( !proxy ) {
            std::string errString = "Required interface '" + resolvedProxyString +
                "' is of wrong type."+
                "  Tried to connect proxy of type "+InterfaceType::ice_staticId()+
                " to remote interface of type "+base->ice_id();
            throw orcaice::TypeMismatchException( ERROR_INFO, errString );
        }
    }
    catch ( const Ice::ConnectionRefusedException& e )
    {
        std::stringstream ss;
        ss << "(while connecting to interface '" << resolvedProxyString << "') cannot reach the adaptor: "<<e.what();
        throw orcaice::NetworkException( ERROR_INFO, ss.str() );
    }
    catch ( const Ice::ObjectNotExistException& e )
    {
        std::stringstream ss;
        ss << "(while connecting to interface '" << resolvedProxyString << "') reached the adaptor but not the interface: "<<e.what();
        throw orcaice::NetworkException( ERROR_INFO, ss.str() );
    }
    catch ( const std::exception& e )
    {
        std::stringstream ss;
        ss << "(while connecting to interface '" << resolvedProxyString << "') something unexpected: "<<e.what();
        throw orcaice::NetworkException( ERROR_INFO, ss.str() );
    }

    // @todo Record who we connected to as a required interface so that Home can report this.
}

/*!
Convenience function, behaves like @ref connectToInterfaceWithString but the proxy information
comes from the configuration file and the @p interfaceTag.

@verbatim
MyInterfacePrx myInterfacePrx;
orcaice::connectToInterfaceWithTag( context_, myInterfacePrx, "MyInterface" );
@endverbatim
Throws ConfigFileException if the interface name cannot be read for some reason.
 */
template<class InterfaceType>
void
connectToInterfaceWithTag( const Context       & context,
                           InterfaceType       & proxy,
                           const std::string   & interfaceTag )
{
    // this may throw ConfigFileException, we don't catch it, let the user catch it at the component level
    std::string proxyString = orcaice::getRequiredInterfaceAsString( context, interfaceTag );

    // now that we have the stingified proxy, use the function above.
    connectToInterfaceWithString( context, proxy, proxyString );
}

//!
//! Connects to the interface and gets the description.
//! Relies on standard naming: "getDescription()".
//!
template<class InterfacePrxType,typename DescriptionType>
DescriptionType
getDescriptionWithTag( const Context     &context,
                       const std::string &interfaceTag )
{
    InterfacePrxType interfacePrx;
    connectToInterfaceWithTag( context, interfacePrx, interfaceTag );
    return interfacePrx->getDescription();
}

//!
//! Connects to the interface and gets the description.
//! Relies on standard naming: "getDescription()".
//!
template<class InterfacePrxType,typename DescriptionType>
DescriptionType
getDescriptionWithString( const Context     &context,
                          const std::string &interfaceString )
{
    InterfacePrxType interfacePrx;
    connectToInterfaceWithString( context, interfacePrx, interfaceString );
    return interfacePrx->getDescription();
}

//@}

} // namespace

#endif
