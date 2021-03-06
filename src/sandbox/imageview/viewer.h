/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Duncan Mercer, Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef IMAGEVIEW_VIEWER_H
#define IMAGEVIEW_VIEWER_H

#include <orca/image.h>
#include <orcaice/context.h>
#include <stdexcept>

namespace imageview {

//! Actual base class for viewers
//! This class doesn't impose any interface other than creation of the actual
//! viewer
class Viewer
{
public:
    Viewer( const orcaice::Context& context );
    virtual ~Viewer() 
    {
    }
  
    // exceptions
    class BadViewerCreation : public std::logic_error 
    {
    public:
        BadViewerCreation(std::string type) : 
            std::logic_error("Cannot create Viewer of type " + type)
        {
        }
    };

    //! Factory for viewers
    static Viewer* factory( const std::string& type, const orcaice::Context& context );

    //! setup the viewer. 
    //! input argument = local image buffer where all the images are copied into
    virtual void initialise()=0;
    //! function that does all the displaying
    virtual void display( orca::ImageDataPtr image )=0;
    
protected:
    
    orcaice::Context context_;

    
};


} // namespace

// Used for dynamically loading driver
// extern "C" {
//     Viewer* createViewerFactory();
// }

#endif

