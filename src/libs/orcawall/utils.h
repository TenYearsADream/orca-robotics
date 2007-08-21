/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_ORCAWALL_UTILS_H
#define ORCA2_ORCAWALL_UTILS_H

#include <string>

namespace orcawall
{

//! Parse required interface tag into interface type and suffix.
//! The suffix is a number,e.g. '0', '1', '99', etc.
//! Throws a std::string error if parsing error is encountered.
void parseInterfaceTag( const std::string& tag, std::string& type, std::string& suffix );

} //namespace

#endif