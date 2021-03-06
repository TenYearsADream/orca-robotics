/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2006-2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <Ice/Ice.h>
#include <orcaice/orcaice.h>
#include <orcacm/orcacm.h>
#include <orcaprobe/orcaprobe.h>
#include <IceGrid/Admin.h>

#include "browserthread.h"
#include "browserevents.h"

using namespace std;
using namespace probe;

BrowserThread::BrowserThread( orcaprobe::AbstractDisplay & display,
                                std::vector<orcaprobe::Factory*> &factories,
                                const orcaice::Context & context ) :
    SafeThread(context.tracer()),
    factories_(factories),
    display_(display),
    events_(new hydroiceutil::EventQueue),
    context_(context)
{
}

void
BrowserThread::chooseActivate()
{
    events_->add( Activate );
}

void 
BrowserThread::chooseReload()
{
    events_->add( Reload );
}

void 
BrowserThread::chooseUp()
{
    events_->add( Up );
}

void 
BrowserThread::chooseTop()
{
    events_->add( Top );
}

void 
BrowserThread::choosePick( int pick )
{
    events_->add( new probe::PickEvent( pick ) );
}

void 
BrowserThread::chooseFilter( const std::string & filter )
{
    events_->add( Filter );
}

void
BrowserThread::chooseDeactivate()
{
    events_->add( Deactivate );
}

void 
BrowserThread::walk()
{
    context_.tracer().debug( "Start receiving browser events" );

    hydroiceutil::EventPtr event;
    int timeoutMs = 500;
    
    while ( !isStopping() )
    {
        if ( !events_->timedGet( event, timeoutMs ) ) {
            continue;
        }

        switch ( event->type() )
        {
        // approx in order of call frequency
        case Pick : {
//             cout<<"pick event"<<endl;
            PickEventPtr e = PickEventPtr::dynamicCast( event );
            pick_ = e->pick;
            pick();
            break;
        }
        case Up : {
            //cout<<"up event"<<endl;
            up();
            break;
        }
        case Top : {
            //cout<<"top event"<<endl;
            top();
            break;
        }
        case Reload : {
//             cout<<"reload event"<<endl;
            reload();
            break;
        }
        case Filter : {
            //cout<<"filter event"<<endl;
            filterRegistry();
            break;
        }
        case Activate : {
            cout<<"activate event"<<endl;
            activate();
            break;
        }
        case Fault : {
            //cout<<"fault event"<<endl;
            fault();
            break;
        }
        case Deactivate : {
            //cout<<"stop event"<<endl;
            deactivate();
            break;
        }
        default : {
            cout<<"unknown browser event "<<event->type()<<". Ignoring..."<<endl;
            events_->add( Fault );
        }
        } // switch
    } // while
}

void 
BrowserThread::loadRegistry()
{
    cout<<"loading registry data for :"<<context_.communicator()->getDefaultLocator()->ice_toString()<<endl;
    
    //
    // remote call!
    //
    display_.showNetworkActivity( true );
    registryHomeData_ = orcacm::getRegistryHomeData( context_, 
            context_.communicator()->getDefaultLocator()->ice_toString() );
    display_.showNetworkActivity( false );

    registryData_ = orcacm::home2hierarch1( registryHomeData_ );

    display_.setRegistryData( registryData_ );
}

void 
BrowserThread::showRegistry()
{
    display_.setFocus( orcaprobe::AbstractDisplay::RegistryFocus );
}

void 
BrowserThread::filterRegistry()
{
//     cout<<"filtering registry data for :"<<context_.communicator()->getDefaultLocator()->ice_toString()<<endl;

    // simply call showRegistryData() again. the driver will filter it.
    display_.setRegistryData( registryData_ );
}

void 
BrowserThread::loadPlatform()
{
    if ( pick_ >= (int)registryData_.platforms.size() ) {
        events_->add( Fault );
        return;
    }

//     cout<<"loading platform data for "<<registryData_.platforms[pick_].name<<endl;
    lastPlatformPick_ = pick_;
    
    bool tryToPing = true;
    int tracePing = orcaice::getPropertyAsIntWithDefault( context_.properties(), context_.tag()+".Config.TracePing", 0 );

    platformData_ = orcacm::home2hierarch2( registryHomeData_, registryData_.platforms[pick_], tryToPing, tracePing );

    display_.setPlatformData( platformData_ );
}

void 
BrowserThread::showPlatform()
{
    display_.setFocus( orcaprobe::AbstractDisplay::PlatformFocus );
}

void 
BrowserThread::loadComponent()
{
//     cout<<"loading component data for "<<endl; //<<orcaice::toString(platformData_.homes[pick_].name)<<endl;
    lastComponentPick_ = pick_;
    
    //
    // remote call!
    //
    display_.showNetworkActivity( true );
    componentData_ = orcacm::getComponentHomeData( context_, platformData_.homes[pick_].proxy );
    display_.showNetworkActivity( false );

    // todo: this is a bit ugly
    componentData_.locatorString = platformData_.locatorString;

//     cout<<"DEBUG: got a list of "<<componentData_.provides.size()<<" provided interfaces for "<<orcaice::toString(componentData_.name)<<endl;
    display_.setComponentData( componentData_ );
}

void 
BrowserThread::showComponent()
{
    display_.setFocus( orcaprobe::AbstractDisplay::ComponentFocus );
}

// NOTE: no remote operations in here!
void 
BrowserThread::loadInterface()
{   
//     cout<<"DEBUG: will load interface "<<pick_<<"/"<<componentData_.provides.size()<<endl;
//     cout<<"DEBUG: loading interface data for "<<componentData_.provides[pick_].name<<endl;
    lastInterfacePick_ = pick_;

    interfaceData_.locatorString = componentData_.locatorString;
    interfaceData_.name.platform = componentData_.name.platform;
    interfaceData_.name.component = componentData_.name.component;
    interfaceData_.name.iface = componentData_.provides[pick_].name;
    interfaceData_.id = componentData_.provides[pick_].id;   

    //
    // Load interface handler
    //
    // one of them must support it, otherwise it would not have an index
    for ( unsigned int i=0; i < factories_.size(); ++i )
    {
        // if this interface is not supported, skip this factory
        if ( factories_[i]->isSupported( interfaceData_.id ) ) {
            // special case: the interface is actually created every time, using smart pointer to avoid mem leaks
            ifaceProbe_ = factories_[i]->create( interfaceData_.id, interfaceData_.name, componentData_.adminPrx, display_, context_ );
        }
    }

    // if specific probe was not found in the factories, load generic probe
    if ( ifaceProbe_==0 ) {
        ifaceProbe_ = new orcaprobe::InterfaceProbe( interfaceData_.name, componentData_.adminPrx, display_, context_ );
    }
    
    // local call
    interfaceData_.operations = ifaceProbe_->operations();

    display_.setInterfaceData( interfaceData_ );
}

void 
BrowserThread::showInterface()
{
    display_.setFocus( orcaprobe::AbstractDisplay::InterfaceFocus );
}

void 
BrowserThread::loadOperation()
{
    //cout<<"loading operation data for "<<interfaceData_.operations[pick_].name<<endl;
    lastOperationPick_ = pick_;

    //
    // remote call!
    //
    display_.showNetworkActivity( true );

    try
    {
        if ( ifaceProbe_->loadOperation( pick_, operationData_ ) ) {
            cout<<ifaceProbe_->toString()<<" failed to load operation "<<pick_<<endl;
            cout<<"Shutting down..."<<endl;
            exit(1);
        }
    }   
    catch( const std::exception& e )
    {
        stringstream ss;
        ss<<e.what()<<endl;
        orcaprobe::reportException( operationData_, ss.str() );
    }

    display_.showNetworkActivity( false );

    // todo: this is a bit ugly
    operationData_.locatorString = interfaceData_.locatorString;

    display_.setOperationData( operationData_ );
}

void 
BrowserThread::showOperation()
{
    display_.setFocus( orcaprobe::AbstractDisplay::OperationFocus );
}

void 
BrowserThread::quit()
{
    cout<<"Quitting..."<<endl;
    cout<<"Not implemented, use Ctrl-C."<<endl;
}
