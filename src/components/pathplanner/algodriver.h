/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */
 
#ifndef ORCA2_PATHPLANNER_ALGORITHM_DRIVER_H
#define ORCA2_PATHPLANNER_ALGORITHM_DRIVER_H

#include <orca/pathplanner2d.h>
#include <orca/ogmap.h>

#include <orcapathplan/pathplanutils.h>

namespace pathplanner
{

//
// Base-class for path-planning drivers
//
class AlgoDriver 
{

public:

    virtual ~AlgoDriver() {};
    
    // Computes the path
    virtual void computePath(   const orca::OgMapDataPtr          & ogMapDataPtr,
                                const orca::PathPlanner2dTaskPtr  & taskPtr,
                                const orca::PathPlanner2dDataPtr  & pathDataPtr ) = 0;

protected:

};

} // namespace

#endif
