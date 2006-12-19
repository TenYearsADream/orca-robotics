/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp,
 *               George Mathews, Stef Williams.
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */
#ifndef REFLECTOREXTRACTOR__H
#define REFLECTOREXTRACTOR__H

#include <orca/laserscanner2d.h>
#include <orca/polarfeature2d.h>
#include "iextractor.h"
#include <orcaice/context.h>

namespace laserfeatures {

class ReflectorExtractor : public IExtractor
{

public: 

    ReflectorExtractor( const orcaice::Context & context, double laserMaxRange, int numReturns );

    // Adds laser features to the 'features' data structure
    void addFeatures( const orca::LaserScanner2dDataPtr    &laserData,
                      orca::PolarFeature2dDataPtr &features );

private: 

    double maxDeltaRangeNearReflector_;
    double maxDeltaRangeWithinReflector_;
    double minReflectorBrightness_;
    double laserMaxRange_;
    double pFalsePositive_;
    double pTruePositive_;
    double rangeSd_;
    double bearingSd_;
    
    // '-1' means 'invalid cluster'
    double calcPFalsePositive( const orca::LaserScanner2dDataPtr &laserData,
                               int start,
                               int end );

    // 
    // Helper functions.  These assume that you're not asking for
    // info about the first or last return
    //

    // Returns the change in range between this return and the previous one.
    double deltaRange( const orca::LaserScanner2dDataPtr &laserData,
                       int returnNum )
        { 
            assert( returnNum != 0 );
            return laserData->ranges[returnNum] - laserData->ranges[returnNum-1]; 
        }

    bool isSketchy( const orca::LaserScanner2dDataPtr &laserData,
                    int returnNum );

};

} // namespace

#endif
