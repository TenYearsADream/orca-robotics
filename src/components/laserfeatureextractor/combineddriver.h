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
 
#ifndef ORCA2_LASERFEATUREEXTRACTOR_COMBINED_DRIVER_H
#define ORCA2_LASERFEATUREEXTRACTOR_COMBINED_DRIVER_H

#include <orcaice/context.h>
#include "algorithmdriver.h"
#include <vector>
#include "iextractor.h"

namespace laserfeatures
{

class CombinedDriver: public AlgorithmDriver
{

public:

    CombinedDriver( const orcaice::Context & context, double maxRange, int numberOfSamples );
    virtual ~CombinedDriver();
    
    virtual int computeFeatures( const orca::LaserScanner2dDataPtr &laserDataPtr,
                                 orca::PolarFeature2dDataPtr       &featureDataPtr );
    
private:

    std::vector<IExtractor*> extractors_;
};

} // namespace

#endif
