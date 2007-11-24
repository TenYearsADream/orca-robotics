/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_TELEOP_COMPONENT_H
#define ORCA2_TELEOP_COMPONENT_H

#include <orcaice/component.h>

namespace teleop
{

class NetworkThread;
class TermDisplayThread;
class InputThread;

class Component : public orcaice::Component
{
public:

    Component();
    virtual ~Component();

    // component interface
    virtual void start();
    virtual void stop();

private:
    NetworkThread* networkThread_;
    TermDisplayThread* displayThread_;
    InputThread* inputThread_;
};

} // namespace

#endif
