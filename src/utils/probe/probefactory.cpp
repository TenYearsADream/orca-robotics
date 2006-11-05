/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2006 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#include "probefactory.h"

// interface handlers
#include "binaryswitchprobe.h"
#include "cpuprobe.h"
#include "homeprobe.h"
#include "laserprobe.h"
#include "powerprobe.h"

using namespace probe;

bool 
ProbeFactory::isSupported( const std::string & id )
{
    if ( id == "::orca::BinarySwitch" || 
        id == "::orca::Cpu" || 
        id == "::orca::Home" || 
        id == "::orca::Laser" || 
        id == "::orca::Power" ) 
    {
        return true;
    }
    else {
        return false;
    }
}

InterfaceProbe* 
ProbeFactory::create( const std::string & id, const orca::FQInterfaceName & name, 
                      DisplayDriver & display, const orcaice::Context & context )
{
    if ( !isSupported( id ) ) {
        return 0;
    }

    if ( id == "::orca::BinarySwitch" ) {
        return new BinarySwitchProbe( name, display, context );
    }
    else if ( id == "::orca::Cpu" ) {
        return new CpuProbe( name, display, context );
    }
    else if ( id == "::orca::Home" ) {
        return new HomeProbe( name, display, context );
    }
    else if ( id == "::orca::Laser" ) {
        return new LaserProbe( name, display, context );
    }
    else if ( id == "::orca::Power" ) {
        return new PowerProbe( name, display, context );
    }

    // shouldn't get here
    return 0;
}
