/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <orcaice/orcaice.h>
#include <orcacm/orcacm.h>
#include <orcaprobe/orcaprobe.h>

#include "localise2dprobe.h"

using namespace std;
using namespace orcaprobefactory;

Localise2dProbe::Localise2dProbe( const orca::FQInterfaceName& name, orcaprobe::DisplayDriver& display,
                                const orcaice::Context& context )
    : InterfaceProbe(name,display,context)
{
    id_ = "::orca::Localise2d";
    
    addOperation( "getDescription" );
    addOperation( "getData" );
    addOperation( "getDataAtTime" );
    addOperation( "subscribe" );
    addOperation( "unsubscribe" );

    Ice::ObjectPtr consumer = this;
    callbackPrx_ = orcaice::createConsumerInterface<orca::Localise2dConsumerPrx>( context_, consumer );
}
    
int 
Localise2dProbe::loadOperationEvent( const int index, orcacm::OperationData& data )
{
    switch ( index )
    {
    case orcaprobe::UserIndex :
        return loadGetDescription( data );
    case orcaprobe::UserIndex+1 :
        return loadGetData( data );
    case orcaprobe::UserIndex+2 :
        return loadGetDataAtTime( data );
    case orcaprobe::UserIndex+3 :
        return loadSubscribe( data );
    case orcaprobe::UserIndex+4 :
        return loadUnsubscribe( data );
    }
    return 1;
}

int 
Localise2dProbe::loadGetData( orcacm::OperationData& data )
{
    orca::Localise2dData result;
    try
    {
        orca::Localise2dPrx derivedPrx = orca::Localise2dPrx::checkedCast(prx_);
        result = derivedPrx->getData();
        orcaprobe::reportResult( data, "data", orcaice::toString(result) );
    }
    catch( const orca::DataNotExistException& e )
    {
        orcaprobe::reportException( data, "data is not ready on the remote interface" );
    }
    catch( const orca::HardwareFailedException& e )
    {
        orcaprobe::reportException( data, "remote hardware failure" );
    }
    catch( const Ice::Exception& e )
    {
        stringstream ss;
        ss<<e<<endl;
        orcaprobe::reportException( data, ss.str() );
    }
    return 0;
}

int 
Localise2dProbe::loadGetDataAtTime( orcacm::OperationData& data )
{
    orcaprobe::reportNotImplemented( data );
    return 0;
}

int 
Localise2dProbe::loadGetDescription( orcacm::OperationData& data )
{
    orca::Localise2dDescription result;
    orcacm::ResultHeader res;

    try
    {
        orca::Localise2dPrx derivedPrx = orca::Localise2dPrx::checkedCast(prx_);
        result = derivedPrx->getDescription();
        orcaprobe::reportResult( data, "data", orcaice::toString(result) );
    }
    catch( const Ice::Exception& e )
    {
        stringstream ss;
        ss<<e<<endl;
        orcaprobe::reportException( data, ss.str() );
    }
    return 0;
}

int 
Localise2dProbe::loadSubscribe( orcacm::OperationData& data )
{
    try
    {
        orca::Localise2dPrx derivedPrx = orca::Localise2dPrx::checkedCast(prx_);
        derivedPrx->subscribe( callbackPrx_ );
        orcaprobe::reportSubscribed( data );

        // save the op data structure so we can use it when the data arrives
        subscribeOperationData_ = data;
    }
    catch( const Ice::Exception& e )
    {
        stringstream ss;
        ss<<e<<endl;
        orcaprobe::reportException( data, ss.str() );
    }
    return 0;
}

int 
Localise2dProbe::loadUnsubscribe( orcacm::OperationData& data )
{    
    try
    {
        orca::Localise2dPrx derivedPrx = orca::Localise2dPrx::checkedCast(prx_);
        derivedPrx->unsubscribe( callbackPrx_ );
        orcaprobe::reportUnsubscribed( data );
    }
    catch( const Ice::Exception& e )
    {
        stringstream ss;
        ss<<e<<endl;
        orcaprobe::reportException( data, ss.str() );
    }
    return 0;
}

void 
Localise2dProbe::setData(const orca::Localise2dData& result, const Ice::Current&)
{
    subscribeOperationData_.results.clear();
    orcaprobe::reportResult( subscribeOperationData_, "data", orcaice::toString(result) );
    display_.setOperationData( subscribeOperationData_ );
};