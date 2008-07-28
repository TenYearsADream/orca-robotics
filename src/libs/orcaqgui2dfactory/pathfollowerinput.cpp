#include <orcaqgui2dfactory/pathfolloweruserinteraction.h>
#include <orcaqgui2dfactory/pathconversionutil.h>
#include "pathfollowerinput.h"

namespace orcaqgui2d {

PathFollowerInput::PathFollowerInput ( PathFollowerUserInteraction         &pathFollowerUI,
                                       WaypointSettings                    *wpSettings,
                                       hydroqguielementutil::IHumanManager &humanManager,
                                       const QString                       &lastSavedPathFile )
    : pathFollowerUI_(pathFollowerUI)
{
    guiPath_.reset( new GuiPath() );
    pathDesignScreen_.reset( new PathDesignScreen( *guiPath_.get(), wpSettings, humanManager ) );
    pathDesignTableWidget_.reset( new PathDesignTableWidget( this, *guiPath_.get() ) );
    pathFileHandler_.reset( new PathFileHandler( humanManager, lastSavedPathFile ) );
}

void 
PathFollowerInput::paint( QPainter *painter )
{
    pathDesignScreen_->paint( painter );
}

void
PathFollowerInput::setUseTransparency( bool useTransparency ) 
{
    pathDesignScreen_->setUseTransparency( useTransparency );
}

void 
PathFollowerInput::processPressEvent( QMouseEvent* e)
{
    pathDesignScreen_->processPressEvent( e );
}

void PathFollowerInput::processReleaseEvent( QMouseEvent* e ) 
{
    pathDesignScreen_->processReleaseEvent( e );
    
    if ( pathDesignTableWidget_->isHidden() ) 
        pathDesignTableWidget_->show();
    pathDesignTableWidget_->refreshTable();
}

void 
PathFollowerInput::updateWpSettings( WaypointSettings* wpSettings )
{
    pathDesignScreen_->updateWpSettings( wpSettings );
}

void 
PathFollowerInput::setWaypointFocus( int waypointId )
{
    pathDesignScreen_->setWaypointFocus( waypointId );
}

bool
PathFollowerInput::getPath( orca::PathFollower2dData &pathData ) const
{    
    int size = pathDesignTableWidget_->numberOfLoops() * guiPath_->size();
    //cout << "DEBUG(pathinput.cpp): getPath: size of waypoints is " << size << endl;
    if (size==0) return false;
    
    const float timeOffset = guiPath_->last().timeTarget + pathDesignScreen_->secondsToCompleteLoop();
    guiPathToOrcaPath( *guiPath_.get(), pathData.path, pathDesignTableWidget_->numberOfLoops(), timeOffset );
    
    return true;
}

void 
PathFollowerInput::savePath( const QString &filename )
{
    int numLoops = pathDesignTableWidget_->numberOfLoops();
    
    float timeOffset = 0.0;
    if (numLoops > 1)
        timeOffset = guiPath_->last().timeTarget + pathDesignScreen_->secondsToCompleteLoop();
  
    pathFileHandler_->savePath( filename, *guiPath_.get(), numLoops, timeOffset );  
}

void
PathFollowerInput::loadPath( const QString &filename ) 
{  
    pathFileHandler_->loadPath( filename, *guiPath_.get() );
    pathDesignTableWidget_->refreshTable(); 
}

void 
PathFollowerInput::loadPreviousPath()
{
    pathFileHandler_->loadPreviousPath( *guiPath_.get() );
    pathDesignTableWidget_->refreshTable(); 
}

void
PathFollowerInput::sendPath() 
{ 
    pathFollowerUI_.send(); 
}

void 
PathFollowerInput::cancelPath() 
{ 
    pathFollowerUI_.cancel(); 
}

}