/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */
#ifndef ORCA_LOCALISE_UTIL_H
#define ORCA_LOCALISE_UTIL_H

#include <orca/localise2d.h>

namespace orcalocalnav {

bool localisationIsUncertain( const orca::Localise2dData &localiseData );

}
#endif
