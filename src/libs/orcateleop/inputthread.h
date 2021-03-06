/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef  INPUT_THREAD_H
#define  INPUT_THREAD_H

#include <memory>
#include <gbxsickacfr/gbxiceutilacfr/safethread.h>
#include <orcaice/context.h>
#include <hydrointerfaces/humaninput2d.h>
#include <hydrodll/dynamicload.h>

namespace orcateleop
{

class Network;
// class InputDriver;

class InputThread : public gbxiceutilacfr::SafeThread
{
public:

    InputThread( Network* network, const orcaice::Context& context );

    // from SafeThread
    virtual void walk();

private:

    Network* network_;

    // The library that contains the driver factory (must be declared first so it's destructed last!!!)
    std::auto_ptr<hydrodll::DynamicallyLoadedLibrary> driverLib_;
    // Generic driver for the hardware
    std::auto_ptr<hydrointerfaces::HumanInput2d> driver_;

    // component current context
    orcaice::Context context_;

};

} // namespace

#endif
