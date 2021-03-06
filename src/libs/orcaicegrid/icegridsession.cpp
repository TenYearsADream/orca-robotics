/*
 * Orca-Robotics Project: Components for robotics
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <sstream>
#include <Ice/Ice.h>

#include "icegridsession.h"
#include <orcaice/orcaice.h>
#include <gbxsickacfr/gbxiceutilacfr/timer.h>

using namespace std;

namespace orcaicegrid {

IceGridSession::IceGridSession( const orcaice::Context &context,
        const IceGrid::RegistryObserverPrx& reg,
        const IceGrid::NodeObserverPrx& node,
        const IceGrid::ApplicationObserverPrx& app,
        const IceGrid::AdapterObserverPrx& adpt,
        const IceGrid::ObjectObserverPrx& obj ) :
    SafeThread(context.tracer()),
    timeoutSec_(0),
    registryObserverPrx_(reg),
    nodeObserverPrx_(node),
    applicationObserverPrx_(app),
    adapterObserverPrx_(adpt),
    objectObserverPrx_(obj),
    context_(context)
{
    stateStore_.set(Disconnected);
}

bool
IceGridSession::tryCreateSession()
{
//     cout<<"TRACE(icegridadminsession.cpp): tryCreateSession()" << endl;

    // config file parameters
    Ice::PropertiesPtr props = context_.properties();
    std::string prefix = context_.tag() + ".Config.";

    std::string instanceName = context_.properties()->getPropertyWithDefault(
            prefix+"IceGrid.InstanceName", "IceGrid" );
    IceGrid::RegistryPrx registry;

    //
    // connect to the Registry
    //
    std::stringstream exceptionSS;
    try {
        string regName = instanceName+"/Registry";
        registry = IceGrid::RegistryPrx::checkedCast( context_.communicator()->stringToProxy(regName) );

        if( !registry )
        {
            context_.tracer().error( "IceGridSession: Could not contact registry" );
            return false;
        }

        Ice::ConnectionPtr conn = registry->ice_getConnection();
        cout<<"Connected to the Registry:"<<endl;
        cout<<"Proxy\t\t[ "<<registry->ice_toString()<<" ]"<<endl;
        cout<<"Connection\t[ "<<conn->toString()<<" ]"<<endl;
    }
    catch ( const Ice::CommunicatorDestroyedException & ) {
        // This is OK, we're shutting down
        return false;
    }
    catch ( const std::exception &e ) {
        exceptionSS << "IceGridSession: Error contacting registry: " << e.what();
    }
    catch ( ... ) {
        exceptionSS << "IceGridSession: Unknown error contacting registry.";
    }

    if ( !exceptionSS.str().empty() ) {
        context_.tracer().error( exceptionSS.str() );
        return false;
    }

    //
    // create Session
    //
    try
    {
        session_ = registry->createAdminSession( "icegridsession-no-access-control",
                                                 "icegridsession-no-access-control" );
        timeoutSec_ = registry->getSessionTimeout();
        stringstream ss; ss<<"IceGridSession: Created session (timeout="<<timeoutSec_<<"s)";
        context_.tracer().info( ss.str() );
    }
    catch ( const Ice::CommunicatorDestroyedException & ) {
        // This is OK, we're shutting down
        return false;
    }
    catch( const std::exception& e) {
        exceptionSS << "IceGridSession: Error creating Admin Session: " << e.what();
    }
    catch( ... ) {
        exceptionSS << "IceGridSession: Unknown exception when creating Admin Session.";
    }

    if ( !exceptionSS.str().empty() ) {
        context_.tracer().error( exceptionSS.str() );
        return false;
    }

//     cout<<"TRACE(icegridadminsession.cpp): tryCreateSession() returning ok" << endl;
    return true;
}

void
IceGridSession::walk()
{
    // This outer loop is repeated on session failure.
    while( !isStopping() )
    {
        try {
            // Connect to the Registry and create the session
            stateStore_.set( Connecting );

            while ( !isStopping() )
            {
                if ( tryCreateSession() && connectedEvent() )
                {
                    //
                    // Connected!
                    //
                    stateStore_.set( Connected );

                    // register observers if any of them are not NULL
                    trySetObservers();
                    break;
                }
                IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(2));
            }

            gbxiceutilacfr::Timer keepAliveTimer;

            // From now on, keep it alive
            while ( !isStopping() )
            {
                gbxiceutilacfr::checkedSleep( this, IceUtil::Time::seconds( timeoutSec_/4 ), 1000 );

                if ( keepAliveTimer.elapsedSec() > timeoutSec_ )
                {
                    stringstream ss;
                    ss << "IceGridSession: Failed to send keepAlives quicker than timeout.  timeoutSec_="
                        <<timeoutSec_<<", elapsed=" << keepAliveTimer.elapsedSec();
                    context_.tracer().warning( ss.str() );
                }
                context_.tracer().debug( "IceGridSession: sending keepAlive()", 10 );

                try
                {
                    //
                    // remote call
                    //
                    session_->keepAlive();

                    keepAliveTimer.restart();
                }
                catch( const Ice::CommunicatorDestroyedException & ) {
                    // This is OK, we're shutting down.
                    break;
                }
                catch( const std::exception& e ) {
                    context_.tracer().warning( "IceGridSession: Failed to keep session alive: "+string(e.what()) );
                    break;
                }
            }
        }
        catch( const Ice::CommunicatorDestroyedException & ) {
            // This is OK, we're shutting down.
        }
        catch ( std::exception &e ) {
            stringstream ss; ss << "IceGridSession: Caught stray exception: " << e.what();
            context_.tracer().warning( ss.str() );
        }
        catch ( ... ) {
            context_.tracer().warning( "IceGridSession: caught unknown stray exception." );
        }

        //
        // Lost it!
        //
        stateStore_.set( Disconnected );
        disconnectedEvent();

    } // while

    // disconnecting before shut down
    stateStore_.set( Disconnecting );

    try {
        // Destroying the session_ will release all allocated objects.
        context_.tracer().info( "IceGridSession: Destroying session." );
        session_->destroy();
    }
    catch ( ... )
    {
        // Not much we can do about it...
    }

    stateStore_.set( Disconnected );
    disconnectedEvent();
}

IceGridSession::SessionState
IceGridSession::getState()
{
    SessionState state;
    stateStore_.get( state );
    return state;
}

void
IceGridSession::setObservers(
        const IceGrid::RegistryObserverPrx& reg,
        const IceGrid::NodeObserverPrx& node,
        const IceGrid::ApplicationObserverPrx& app,
        const IceGrid::AdapterObserverPrx& adpt,
        const IceGrid::ObjectObserverPrx& obj )
{
    {
        IceUtil::Mutex::Lock lock( observerMutex_ );
        registryObserverPrx_ = reg;
        nodeObserverPrx_ = node;
        applicationObserverPrx_ = app;
        adapterObserverPrx_ = adpt;
        objectObserverPrx_ = obj;
    }

    trySetObservers();
}

void
IceGridSession::trySetObservers()
{
    SessionState state;
    stateStore_.get( state );

    if ( state != Connected ) {
        context_.tracer().warning( "IceGridSession: Could not set observers because the session is not connected yet." );
        return;
    }

    try
    {
        IceUtil::Mutex::Lock lock( observerMutex_ );
        // register observers if ANY of them are not NULL
        int obsCount = (int)(registryObserverPrx_!=0) + (int)(nodeObserverPrx_!=0)
                     + (int)(applicationObserverPrx_!=0) + (int)(adapterObserverPrx_!=0)
                     + (int)(objectObserverPrx_!=0);
        if ( obsCount )
        {
            session_->setObservers( registryObserverPrx_, nodeObserverPrx_,
                        applicationObserverPrx_, adapterObserverPrx_, objectObserverPrx_ );

            stringstream ss;
            ss << "IceGridSession: Set " << obsCount << " observers.";
            context_.tracer().info( ss.str() );
        }
    }
    catch ( const IceGrid::ObserverAlreadyRegisteredException& ) {
        // This is OK.
        context_.tracer().warning( "IceGridSession: Got ObserverAlreadyRegisteredException exception." );
    }
}

std::string
IceGridSession::toString( SessionState state )
{
    switch ( state )
    {
        case Disconnected :
            return "Disconnected";
        case Disconnecting :
            return "Disconnecting";
        case Connected :
            return "Connected";
        case Connecting :
            return "Connecting";
        default :
            return "???Undefined???";
    }
}

} // namespace
