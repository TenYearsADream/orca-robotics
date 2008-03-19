/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp,
 *               George Mathews, Stef Williams.
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef FOREGROUND_EXTRACTOR_H
#define FOREGROUND_EXTRACTOR_H

#include <orca/laserscanner2d.h>
#include <orca/polarfeature2d.h>
#include "iextractor.h"
#include <orcaice/context.h>
#include <hydrolaserfeatureextract/hydrolaserfeatureextract.h>

namespace laserfeatures {

    class ForegroundExtractor: public IExtractor
{

public: 

    ForegroundExtractor( const orca::RangeScanner2dDescription &laserDescr,
                         const orcaice::Context                &context );

    // Adds laser features to the 'features' data structure
    void addFeatures( const orca::LaserScanner2dDataPtr &laserData,
                      orca::PolarFeature2dData &features );   

private: 
    
    std::auto_ptr<hydrolfextract::ForegroundExtractor> foregroundExtractor_;
};

} // namespace

#endif
