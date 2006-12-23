/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2006 Alex Brooks, Alexei Makarenko, Tobias Kaupp, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCAPLAYER_CONVERT_UTILITIES_H
#define ORCAPLAYER_CONVERT_UTILITIES_H

// Orca objects
#include <orca/laserscanner2d.h>
#include <orca/position2d.h>
#include <orca/position3d.h>
#include <orca/localise2d.h>

// Player proxies
namespace PlayerCc
{
class LaserProxy;
class Position2dProxy;
class Position3dProxy;
class SimulationProxy;
}

namespace orcaplayer
{
/*!
 *  @name Player to Orca Conversion Functions
 */
//@{

//! Converts position data to orca::Position2dData.
void convert( const PlayerCc::Position2dProxy & proxy, orca::Position2dData& obj );

//! Converts player position3d data to orca::Position3dData.
void convert( const PlayerCc::Position3dProxy & proxy, orca::Position3dData& obj );

//! Converts laser data to orca::LaserScanner2dData. If convertHeaders is FALSE, only the data
//! (ranges, intensities) are processed and the header information (fieldOfView, maxRange, etc.) 
//! is left untouched.
void convert( const PlayerCc::LaserProxy & proxy, orca::LaserScanner2dDataPtr& obj, bool convertHeaders=true );

//! Converts laser configuration to values of orca::RangeScanner2dConfig.
void convert( const PlayerCc::LaserProxy & proxy, 
         double &maxRange, double &fieldOfView, double &startAngle, int &numberOfSamples );

//! Converts Stage simulated position data to orca::Localise2dData.
//! @note Access functions in SimulationProxy are not const.
void convert( PlayerCc::SimulationProxy & proxy, orca::Localise2dData& obj, const std::string & id );
//void convert( const PlayerCc::SimulationProxy & proxy, orca::Localise2dData& obj, const std::string & id );

//@}

/*!
 *  @name Orca to Player Conversion Functions
 */
//@{
void convert( const orca::Position2dData& obj, PlayerCc::Position2dProxy & proxy );

void convert( const orca::LaserScanner2dDataPtr& obj, PlayerCc::LaserProxy & proxy );
//@}

} // namespace

#endif
