/*
 * Orca-Robotics Project: Components for robotics
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <orcaice/application.h>
#include <orcaice/component.h>
#include <orcaice/orcaice.h>
#include <hydroutil/hydroutil.h>
#include "detail/privateutils.h"

using namespace std;

namespace orcaice {

namespace {

/*
    Order priority for different sources of configuration parameters, from lowest to highest:
        1. orca factory defaults
        2. global config file
        3. component config file
        4. command line arguments
*/
void setProperties( Ice::PropertiesPtr   &properties,
                    const Ice::StringSeq &commandLineArgs,
                    const std::string    &componentTag )
{
    // pre-parse Orca-specific command line arguments
    // (nothing here right now)
    // orcaice::parseOrcaCommandLineOptions( args );

    // Level 4. Highest priority, apply properties from the command line arguments
    // read in all command line options starting with '--", but not "-"
    // note that something like --bullshit will be parsed to --bullshit=1
    // Note that this is a standard Ice function.
    properties->parseCommandLineOptions( "", commandLineArgs );
    initTracerInfo( componentTag+": Loaded command line properties" );

    // Level 3. Now, apply properties from this component's config file (do not force!)
    detail::addPropertiesFromApplicationConfigFile( properties, commandLineArgs, componentTag );

    // Level 2. Now, apply properties from the global Orca config file
    detail::addPropertiesFromGlobalConfigFile( properties, componentTag );

    // Level 1. apply Orca factory defaults
    orcaice::detail::setFactoryProperties( properties, componentTag );
    initTracerInfo( componentTag+": Loaded factory default properties" );

    // Level 0. sort out platform and component names, apply defaults, set adapter names.
    orcaice::detail::postProcessComponentProperties( properties, componentTag );
}

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Application::Application( orcaice::Component &component, Ice::SignalPolicy policy ) :
    Ice::Application(policy),
    component_(component),
    isComponentStopped_(false)
{
}

int
Application::orcaMain(int argc, char* argv[])
{
    // convert to string sequence for convenience
    Ice::StringSeq args = Ice::argsToStringSeq( argc, argv );

    // parse command line arguments for flags which require immediate action
    for ( unsigned int i=0; i<args.size(); ++i ) {
        if ( !args[i].compare( 0,2, "-h" ) ||
             !args[i].compare( 0,6, "--help" ) )
        {
            orcaice::initTracerInfo( component_.help( hydroutil::basename(args[0]) ) );
            // nothing to clean up yet
            exit(0);
        }
        else if ( !args[i].compare( 0,2, "-v" ) ||
                  !args[i].compare( 0,9, "--version" ) )
        {
            orcaice::detail::printAllVersions( component_ );
            // nothing to clean up yet
            exit(0);
        }
    }

    // print version information on the first line
    orcaice::detail::printAllVersions( component_ );

    Ice::InitializationData initData;
    // Note that we don't use the version which takes arguments so that the config file which may be
    // specified by mistake with --Ice.Config is not loaded.
    initData.properties = Ice::createProperties();

    // Set the component's properties based on the various sources from which properties can be read
    setProperties( initData.properties, args, component_.context().tag() );

    // now pass the startup options to Ice which will start the Communicator
    return Ice::Application::main( argc, argv, initData );
}

int
Application::run( int argc, char* argv[] )
{
    // communicator is already initialized by Ice::Application
    // all defaults are already applied
    Ice::PropertiesPtr props = communicator()->getProperties();

    // what to do when a signal is received (e.g. Ctrl-C)
    if ( props->getPropertyAsInt("Orca.Application.CallbackOnInterrupt") ) {
        // normally, we want to get a change to shut down our component before
        //shutting down the communicator
        callbackOnInterrupt();
    }
    else {
        shutdownOnInterrupt();
    }

    // print all prop's now, after some stuff was added, e.g. Tag.AdapterId
    // note: is it possible that some of the prop's got stripped off by Ice::Application::main()? I don't think so.
    if ( props->getPropertyAsInt( "Orca.Component.PrintProperties" ) ) {
        orcaice::detail::printComponentProperties( props, component_.context().tag() );
    }

    // create the one-and-only component adapter
    try
    {
	  adapter_ = communicator()->createObjectAdapter(component_.context().tag());
	}
	catch( const Ice::InitializationException& e )
	{
		stringstream ss;
		ss << "(while creating component adapter) : " << e.what();
        initTracerError( ss.str() );
        initTracerInfo( "Application quitting. Orca out." );
        return 1;
	}
    initTracerInfo( component_.context().tag()+": Created object adapter." );

    //
    // Give the component all the stuff it needs to initialize
    //
    orca::FQComponentName fqCompName;
    fqCompName.platform = props->getProperty( component_.context().tag()+".Platform" );
    fqCompName.component = props->getProperty( component_.context().tag()+".Component" );

    bool isApp = true;
    component_.init( fqCompName, isApp, adapter_ );
    initTracerInfo( component_.context().tag()+": Application initialized." );

    //
    // Cannot change anything in Component::context_ after this step.
    //
    if ( props->getPropertyAsInt( "Orca.Component.PrintContext" ) ) {
        orcaice::detail::printComponentContext( component_.context() );
    }

    //
    // Start the component, catching all exceptions
    //
    stringstream exceptionSS;
    try
    {
        component_.start();
        // for the components which hug the thread, eg. Qt-based, this will be printed at shutdown
        // this is optional because after the comp is started we can't assume that dumping to cout
        // will produce the desired result (e.g. if ncurses are used)
        if ( props->getPropertyAsInt( "Orca.Component.PrintStarted" ) ) {
            initTracerInfo( component_.context().tag()+": Component started." );
        }
    }
    catch ( const std::exception &e ) {
        exceptionSS << "(while starting component) : " << e.what();
    }
    catch ( const std::string &e ) {
        exceptionSS << "(while starting component) : " << e;
    }
    catch ( const char *e ) {
        exceptionSS << "(while starting component) : " << e;
    }
    catch ( ... ) {
        exceptionSS << "(while starting component) .";
    }

    if ( !exceptionSS.str().empty() ) {
        initTracerError( component_.context().tag()+": "+exceptionSS.str() );
        initTracerInfo( component_.context().tag()+": Application quitting. Orca out." );
        return 1;
    }

    // component started without a problem. now will wait for the communicator to shutdown
    // this will typically happen after a signal is recieved from Ctrl-C or from IceGrid.
    communicator()->waitForShutdown();
    initTracerInfo( component_.context().tag()+": Communicator has shut down." );

    stopComponent();

    adapter_->waitForDeactivate();
    initTracerInfo( component_.context().tag()+": Adapter deactivated." );

    initTracerInfo( component_.context().tag()+": Orca out." );

    // communicator will be destroyed by Ice::Application
    return 0;
}

void
Application::stopComponent()
{
    IceUtil::Mutex::Lock lock(mutex_);

    if ( isComponentStopped_ )
        return;

    // first tell component to shutdown
    initTracerInfo( component_.context().tag()+": Stopping component..." );
    component_.stop();
//     initTracerInfo( component_.context().tag()+": Finalising component..." );
    component_.finalise();
    initTracerInfo( component_.context().tag()+": Component stopped." );
    isComponentStopped_ = true;
}

// this function is only called if we choose to do callbackOnInterrupt()
void
Application::interruptCallback( int signal )
{
    initTracerInfo( component_.context().tag()+": Received interrupt signal." );
    stopComponent();

    // now we can just destroy the communicator
    initTracerInfo( component_.context().tag()+": Shutting down communicator." );
    // (being extra careful here, copy exception handling from Ice::Application::destroyOnInterruptCallback() )
    stringstream exceptionSS;
    try
    {
        assert(communicator() != 0);
        // we just shutdown the communicator, Ice::Application will destroy it later.
        //
        communicator()->shutdown();
    }
    catch(const std::exception& ex) {
        exceptionSS << " (while shutting down in response to signal " << signal << "): " << ex.what();
    }
    catch(const std::string& msg) {
        exceptionSS << " (while shutting down in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg) {
        exceptionSS << " (while shutting down in response to signal " << signal << "): " << msg;
    }
    catch(...) {
        exceptionSS << " (while shutting down in response to signal " << signal << "): unknown exception";
    }

    if ( !exceptionSS.str().empty() )
        initTracerError( component_.context().tag()+": "+exceptionSS.str() );
}

} // namespace
