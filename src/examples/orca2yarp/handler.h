/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
 
#ifndef ORCA2_ORCA2YARP_HANDLER_H
#define ORCA2_ORCA2YARP_HANDLER_H

#include <hydroutil/thread.h>
#include <orcaice/context.h>

namespace orca2yarp
{

class Handler: public hydroutil::Thread
{    	
public:
    Handler( const orcaice::Context& context );
    ~Handler();

    virtual void run();

private:
    orcaice::Context context_;

};

} // namespace
#endif


