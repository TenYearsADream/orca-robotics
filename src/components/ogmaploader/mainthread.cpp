/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <orcaice/orcaice.h>
#include <orcaifaceutil/ogmap.h>

#include "mainthread.h"
#include "fakemaploader.h"
#include "maploader.h"

using namespace std;
using namespace ogmaploader;

MainThread::MainThread( const orcaice::Context &context ) : 
    SubsystemThread( context.tracer(), context.status(), "MainThread" ),
    context_(context)
{
    // this subsystem will initialise and exit, but the component will continue running.
    setSubsystemType( gbxutilacfr::SubsystemEarlyExit );
}

void
MainThread::initialise()
{
    setMaxHeartbeatInterval( 10.0 );

    //
    // INITIAL CONFIGURATION
    //
    Ice::PropertiesPtr prop = context_.properties();
    std::string prefix = context_.tag() + ".Config.";

    //
    // LOAD THE MAP
    //
    orca::OgMapData theMap;

    std::string driverName = orcaice::getPropertyWithDefault( prop, prefix+"Driver", "fake" );
    if ( driverName == "fake" )
    {
        cout<<"TRACE(maploadermainthread.cpp): Instantiating fake driver" << endl;
        fakeLoadMap( theMap );
    }
    else if ( driverName == "file" )
    {
        try {
            loadMapFromFile( context_, theMap );
            const int recurse=999;
            const int expand=0;
            cout<<"TRACE(mainthread.cpp): Loaded map: " << ifaceutil::toString(theMap,recurse,expand) << endl;
        }
        catch ( const gbxutilacfr::Exception& e )
        {   
            // unrecoverable error
            context_.shutdown();
            throw;
        }        
    }
    else
    {
        // unrecoverable error
        context_.shutdown();
        throw gbxutilacfr::Exception( ERROR_INFO, "Unknown driver type: "+driverName );
    }

    //
    // EXTERNAL PROVIDED INTERFACES
    //
    ogMapImpl_ = new orcaifaceimpl::OgMapImpl( "OgMap", context_ );
    ogMapImpl_->initInterface( theMap, this, subsysName() );
}
