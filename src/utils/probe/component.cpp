/*
 * Orca-Robotics Project: Components for robotics
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2006-2007 Alexei Makarenko
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <algorithm>

#include <orcaice/orcaice.h>
#include <orcaprobe/orcaprobe.h>
#include <hydrodll/dynamicload.h>
#include "component.h"
#include <hydroutil/hydroutil.h>
#include "browserthread.h"

#include "term-iostream/termiostreamdisplay.h"
#ifdef HAVE_GUI_QT_DRIVER
#   include "gui-qt/guiqtdisplay.h"
#endif

using namespace std;

namespace probe
{

static const char *DEFAULT_FACTORY_NAME="libOrcaProbeFactory.so";

orcaprobe::Factory* loadFactory( hydrodll::DynamicallyLoadedLibrary &lib )
{
    orcaprobe::Factory *f =
        hydrodll::dynamicallyLoadClass<orcaprobe::Factory,FactoryMakerFunc>
                                        (lib, "createFactory");
    return f;
}

Component::Component()
    : orcaice::Component( "Probe", orcaice::NoStandardInterfaces )
{
}

Component::~Component()
{
    assert( libraries_.size() == factories_.size() );
    for ( unsigned int i=0; i < libraries_.size(); i++ ){
//         cout<<"deleting factory #"<<i<<endl;
        delete factories_[i];
//         cout<<"deleting library #"<<i<<endl;
        delete libraries_[i];
    }
}

std::vector<std::string>
Component::loadPluginLibraries( const std::string & factoryLibNames )
{
    // Parse space-separated list of lib names
    vector<string> libNames = hydroutil::toStringSeq( factoryLibNames, ' ' );

    // this will be a listing of unique supported interfaces
    std::vector<std::string> supportedInterfaces;
    std::vector<std::string> ifaces;

    for ( unsigned int i=0; i < libNames.size(); i++ )
    {
        stringstream ss;
        ss << "Loading factory library: " << libNames[i];
        context().tracer().info( ss.str() );

        try {
            hydrodll::DynamicallyLoadedLibrary *lib = new hydrodll::DynamicallyLoadedLibrary(libNames[i]);
            orcaprobe::Factory *f = loadFactory( *lib );
            libraries_.push_back(lib);
            factories_.push_back(f);

            ifaces = f->supportedTypes();
            for ( unsigned int j=0; j<ifaces.size(); ++j ) {
                supportedInterfaces.push_back( ifaces[j] );
            }
        }
        catch (hydrodll::DynamicLoadException &e)
        {
            cout << "ERROR(loggercomponent.cpp): " << e.what() << endl;
            throw;
        }
    }

    if ( factories_.empty() ) {
        std::string err = "No log factories were loaded.";
        context().tracer().error( err );
        throw err;
    }

    // eliminate duplicates from the listing of supported interfaces
    std::sort( supportedInterfaces.begin(), supportedInterfaces.end() );
    std::unique( supportedInterfaces.begin(), supportedInterfaces.end() );

    return supportedInterfaces;
}

void
Component::start()
{
    Ice::PropertiesPtr props = context().properties();
    std::string prefix = context().tag()+".Config.";

    string libNames = orcaice::getPropertyWithDefault( props, prefix+"FactoryLibNames", DEFAULT_FACTORY_NAME );
    // returns a listing of unique supported interfaces, for display drivers to know what's supported
    std::vector<std::string> supportedInterfaces = loadPluginLibraries( libNames );

    // which driver to load?
    std::string driverName = orcaice::getPropertyWithDefault( props, prefix+"Driver", "term-iostream" );

    // generic interface to the user interface
    orcaprobe::IBrowser* browser = 0;
    // generic interface to the display interface
    orcaprobe::AbstractDisplay* display = 0;

    if ( driverName == "gui-qt" )
    {
#ifdef HAVE_GUI_QT_DRIVER
        context().tracer().info( "Loading GUI Qt driver.");
        display = new GuiQtDisplay( supportedInterfaces );
#else
        throw gbxutilacfr::Exception( ERROR_INFO, "Can't instantiate driver type 'gui-qt' because it was not compiled." );
#endif
    }
    else if ( driverName == "term-iostream" )
    {
        context().tracer().info( "Loading terminal iostream driver.");
        display = new TermIostreamDisplay( supportedInterfaces );
    }
    else {
        std::string errorStr = "Unknown driver type." + driverName + " Cannot talk to hardware.";
        context().tracer().error( errorStr);
        throw gbxutilacfr::HardwareException( ERROR_INFO, errorStr );
    }
    context().tracer().debug( "Loaded probe driver.");

    BrowserThread browserThread( *display, factories_, context() );
    browser = &browserThread;
    browserThread.start();

    // for Qt driver, this will not return
    context().tracer().debug( "Enabling probe driver.");
    display->enable( browser );

    // normally the application does this,
    // now we have to because Qt display keeps the thread
    context().communicator()->shutdown();

    // the rest is handled by the application/service
}

void
Component::stop()
{
    // this component cannot quit nicely because display drivers are using the main thread
}

} // namespace
