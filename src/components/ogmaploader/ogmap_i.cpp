#include "ogmap_i.h"
#include <iostream>
#include <orcaice/orcaice.h>

using namespace std;
using namespace orca;
using namespace orcaice;

OgMapI::OgMapI( orca::OgMapDataPtr  theMap,
                const std::string  &tag,
                orcaice::Context    context )
    : context_(context)
{
    // Find IceStorm Topic to which we'll publish
    topicPrx_ = orcaice::connectToTopicWithTag<OgMapConsumerPrx>
        ( context_, consumerPrx_, tag );

    // Try to push the map out to IceStorm first
    try {
        context_.tracer()->debug( "Pushing to IceStorm", 2 );
        consumerPrx_->setData( theMap );
    }
    catch ( Ice::ConnectionRefusedException &e )
    {
        // This could happen if IceStorm dies.
        // If we're running in an IceBox and the IceBox is shutting down, 
        // this is expected (our co-located IceStorm is obviously going down).
        context_.tracer()->warning( "Failed push to IceStorm." );
    }

    theMap_ = theMap;
}

OgMapDataPtr
OgMapI::getData(const Ice::Current& current) const
{
    std::cout << "getData()" << std::endl;

    if ( theMap_ == 0 )
        throw orca::DataNotExistException("Map not loaded correctly.");
    
    return theMap_;
}

void
OgMapI::subscribe(const ::OgMapConsumerPrx& subscriber,
                  const Ice::Current& current)
{
    cout<<"subscribe()"<<endl;
    IceStorm::QoS qos;
    topicPrx_->subscribe( qos, subscriber );
}

void
OgMapI::unsubscribe(const ::OgMapConsumerPrx& subscriber,
                    const Ice::Current& current)
{
    cout<<"unsubscribe()"<<endl;
    topicPrx_->unsubscribe( subscriber );
}
