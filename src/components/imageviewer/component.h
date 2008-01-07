/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <orcaice/component.h>
#include <hydroiceutil/buffer.h>
#include <orca/camera.h>


namespace imageviewer{

class MainThread;

class Component : public orcaice::Component
{
public:

    Component();
    virtual ~Component();

    // component interface
    virtual void start();
    virtual void stop();

private:

    hydroiceutil::Buffer<orca::CameraData> dataPipe_;
    MainThread* mainloop_;
};
    
} // namespace

#endif
