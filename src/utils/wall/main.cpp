/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2006-2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#include <orcaice/application.h>
#include "component.h"

//
// Build the component into a stand-alone application
//
int 
main(int argc, char * argv[])
{
    wall::Component component;
    orcaice::Application app( component );
    return app.main(argc, argv);
}