/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_HOME_INTERFACE_ICE
#define ORCA2_HOME_INTERFACE_ICE

#include <orca/common.ice>
#include <orca/ocm.ice>

module orca
{
/*!
    @ingroup orca_interfaces
    @defgroup orca_interface_home Home
    @brief Component meta information.
    @{
*/

/*!
    @brief Contains data on the component's interfaces.
*/
struct HomeData
{    
    //! Time when data was measured.
    Time timeStamp;

    //! The Home interface itself is also included in this list of provided interfaces.
    ComponentData comp;
};

/*!
    @brief Access to information about components interfaces.
*/
interface Home
{
    //! Returns component data.
    idempotent HomeData getData();
};

/*!
    @}
*/
}; // module

#endif
