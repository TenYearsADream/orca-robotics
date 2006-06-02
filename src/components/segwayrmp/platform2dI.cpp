/*
 *  Orca Project: Components for robotics.
 *
 *  Copyright (C) 2004-2006
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <orcaice/orcaice.h>

#include "platform2dI.h"

using namespace std;
using namespace orca;
using namespace orcaice;
using namespace segwayrmp;

Platform2dI::Platform2dI( orcaice::PtrProxy<orca::Position2dDataPtr>    & position2dPipe,
                          orcaice::PtrNotify<orca::Velocity2dCommandPtr>& commandPipe,
                          orcaice::PtrProxy<orca::Platform2dConfigPtr>  & setConfigPipe,
                          orcaice::PtrProxy<orca::Platform2dConfigPtr>  & currentConfigPipe,
                          const IceStorm::TopicPrx                      & topic )
    : position2dPipe_(position2dPipe),
      commandPipe_(commandPipe),
      setConfigPipe_(setConfigPipe),
      currentConfigPipe_(currentConfigPipe),
      topic_(topic)
{
}

// served out the data to the client (it was stored here by the driver at the last read)
orca::Position2dDataPtr
Platform2dI::getData(const Ice::Current& current) const
{
    //std::cout << "Sending data back" << std::endl;

    // create a null pointer. data will be cloned into it.
    Position2dDataPtr data;

    try
    {
        position2dPipe_.get( data );
    }
    catch ( const orcaice::Exception & e )
    {
        throw orca::DataNotExistException( "try again later." );
    }

    //cout << data <<endl;
    return data;
}

orca::Position2dGeometryPtr
Platform2dI::getGeometry(const Ice::Current& current) const
{
    //std::cout << "Pretending to send geometry back" << std::endl;

    // @todo implement
    Position2dGeometryPtr geometry = new Position2dGeometry;

    return geometry;
}

void
Platform2dI::subscribe(const ::orca::Position2dConsumerPrx& subscriber, const ::Ice::Current&)
{
    //cout<<"subscription request"<<endl;
    IceStorm::QoS qos;
    topic_->subscribe( qos, subscriber );
}

void
Platform2dI::unsubscribe(const ::orca::Position2dConsumerPrx& subscriber, const ::Ice::Current&)
{
    //cout<<"unsubscription request"<<endl;
    topic_->unsubscribe( subscriber );
}

// Store incoming command in a proxy, it will be handled by the driver at the next opportunity.
void
Platform2dI::setCommand(const ::orca::Velocity2dCommandPtr& command, const ::Ice::Current& )
{
    // this is executed directly to hardware and may throw an orca::HardwareException
    commandPipe_.set( command );
}

orca::Platform2dConfigPtr
Platform2dI::getConfig(const ::Ice::Current& ) const
{
    //std::cout << "Sending config back" << std::endl;

    Platform2dConfigPtr config;
    currentConfigPipe_.get( config );

    return config;
}

void
Platform2dI::setConfig(const ::orca::Platform2dConfigPtr& config, const ::Ice::Current& )
{
    //cout<<"TRACE(laser_i.cpp): Config set." << endl;
    setConfigPipe_.set( config );
}
