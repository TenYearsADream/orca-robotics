/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef RANGESCANNER2D_CONSUMER_I_H
#define RANGESCANNER2D_CONSUMER_I_H

#include <iostream>

#include <orca/laserscanner2d.h>
#include <orcaice/orcaice.h>

namespace lasermon {

class RangeScanner2dConsumerI : public orca::RangeScanner2dConsumer
{
    public:
        virtual void setData(const orca::RangeScanner2dDataPtr& data, const Ice::Current&)
        {
            int skipReturns = 0;
            try {
                // Is it a laser scan?
                std::cout << orcaice::toString( orca::LaserScanner2dDataPtr::dynamicCast( data ), skipReturns ) << std::endl;
            }
            catch ( IceUtil::NullHandleException & )
            {
                // Wasn't a laser scan.
                std::cout << orcaice::toString( data, skipReturns ) << std::endl;
            }
        }
};

typedef IceUtil::Handle<RangeScanner2dConsumerI> RangeScanner2dConsumerIPtr;

}

#endif
