/*
 *  Orca Project: Components for robotics.
 *
 *  Copyright (C) 2004-2006
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef ORCA2_TELEOP_KEYBOARD_NCURSES_DRIVER_H
#define ORCA2_TELEOP_KEYBOARD_NCURSES_DRIVER_H

#include <curses.h>

#include "../inputdriver.h"
#include "../displayhandler.h"

namespace teleop
{

class KeyboardNcurcesDriver : public InputDriver, public DisplayHandler
{
public:

    KeyboardNcurcesDriver( const InputDriver::Config &cfg );
    virtual ~KeyboardNcurcesDriver();

    virtual int enable();
    virtual int disable();

    // Blocks till new data is available
    virtual int read( orca::Velocity2dCommandPtr &data );

    // from DisplayHandler
    virtual void displayEvent( const Event e );
    virtual void displayCommand( const orca::Velocity2dCommandPtr & command,
                                 const bool vx=false, const bool vy=false, const bool w=false );

private:

    orca::Velocity2dCommandPtr command_;

    Config config_;

    double deltaSpeed_;     // [m/s]
    double deltaTurnrate_;  // [rad/sec]

    void keyboardHelp();

    // obscure ncurses stuff
    WINDOW* mainwin_;

};

} // namespace

#endif
