/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <orcaice/orcaice.h>
#include <orcaifacestring/status.h>
#include "statusconsumerImpl.h"

using namespace std;
namespace statusmon {

StatusConsumerImpl::StatusConsumerImpl( const Config& config, const orcaice::Context& context ) :
    orcaifaceimpl::ConsumerImpl<orca::StatusPrx,orca::StatusConsumer,orca::StatusConsumerPrx,orca::StatusData>(context),
    hasValidData_(false),
    isDestructing_(false),
    config_(config)
{
    IceUtil::Mutex::Lock lock(statusMutex_);
    lastDataReceivedTime_ = IceUtil::Time::now();
}

StatusConsumerImpl::~StatusConsumerImpl()
{
    IceUtil::Mutex::Lock lock(statusMutex_);
    isDestructing_ = true;
}

void
StatusConsumerImpl::subscribe()
{
    if ( isDestructing_ ) {
        return;
    }
    
    IceUtil::Mutex::Lock lock(statusMutex_);
    lastResubscribeTime_  = IceUtil::Time::now();

    std::string proxyStr = string("status@")+config_.platformName+"/"+config_.componentName;
    // this may throw
    subscribeWithString( proxyStr );
}


void
StatusConsumerImpl::dataEvent( const orca::StatusData& data ) 
{
    IceUtil::Mutex::Lock lock(statusMutex_);

    statusData_ = data;
    hasValidData_ = true;
    lastDataReceivedTime_ = IceUtil::Time::now();
}

bool
StatusConsumerImpl::getStatus( StatusDetails &details )
{
    IceUtil::Mutex::Lock lock(statusMutex_);

    details.isStale = false;
    details.dataAvailable = hasValidData_;
    details.statusData = statusData_;
    
    // if we've never received any data, tell the caller to subscribe us
    if (!hasValidData_) return true;

    // the return value
    bool shouldResubscribe=false;
    
    IceUtil::Time timeSinceLastUpdate = IceUtil::Time::now() - lastDataReceivedTime_;
    details.secSinceHeard = timeSinceLastUpdate.toSeconds();

    if ( details.secSinceHeard > config_.resubscribeTimeout )
    {
        IceUtil::Time timeSinceLastResubscribe = IceUtil::Time::now() - lastResubscribeTime_;
        if ( timeSinceLastResubscribe.toSeconds() > config_.resubscribeInterval )
        {
            stringstream ss;
            ss << "StatusConsumerImp(" << config_.platformName << "/"<< config_.componentName << "): Haven't heard from Status for "<< timeSinceLastUpdate << " Caller should try to resubscribe!";
            context_.tracer().warning( ss.str() );
            
            shouldResubscribe=true;
        }

        if ( details.secSinceHeard > config_.staleTimeout )
        {
            details.isStale = true;
        }
    }
    
    return shouldResubscribe;

}

} // namespace