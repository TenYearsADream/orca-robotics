/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCA_ODOMETRY2D_CONSUMER_I_H
#define ORCA_ODOMETRY2D_CONSUMER_I_H

// include defnition of Ice runtime
#include <Ice/Ice.h>
#include <hydroiceutil/buffer.h>
#include <orca/odometry2d.h>

class Odometry2dConsumerI : public orca::Odometry2dConsumer
{
    public:
        Odometry2dConsumerI( hydroiceutil::Buffer<orca::Odometry2dData> & pipe );
        virtual void setData(const orca::Odometry2dData& data, const Ice::Current&);

    private:
        hydroiceutil::Buffer<orca::Odometry2dData> & pipe_;
};

#endif
