/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCAICE_COMONENTSTATUS_AGGREGATOR_H
#define ORCAICE_COMONENTSTATUS_AGGREGATOR_H

#include <hydroiceutil/localstatus.h>
#include <orca/status.h>

namespace orcaice
{
    
enum AggregatorPolicy
{

    // The subsystem with the "worst" health and its corresponding state define the component state and health
    AggregatorPolicyWorstHealth,
    
    // Component health and state are chosen independently: health is the worst health in the system.
    // State: if any subsystem is idle, initialising, or shutdown -> component state is set accordingly
    //        if all subsystems are ok -> component state is ok
    //        if any subsystem is finalising and has its flag set to take it into account -> component is finalising
    AggregatorPolicyStateBased
};

class ComponentStatusAggregator
{

public:
    
    ComponentStatusAggregator( AggregatorPolicy policy = AggregatorPolicyStateBased );
            
    void convert( const hydroiceutil::LocalComponentStatus &from,
                  const orca::FQComponentName              &name, 
                  orca::ComponentStatus                    &to );
            
private:
    
    AggregatorPolicy policy_;
    
    
};

}

#endif

