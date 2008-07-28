/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef PATHFOLLOWER_INPUT_H
#define PATHFOLLOWER_INPUT_H

#include <hydroqguipath/pathdesignscreen.h>
#include <hydroqguipath/pathdesigntablewidget.h>
#include <hydroqguipath/ipathinput.h>
#include <hydroqguipath/pathutils.h>
#include <orcaqgui2dfactory/pathfilehandler.h>

namespace orcaqgui2d {
    
class PathFollowerUserInteraction;

//!
//! An implementation of PathInput for the PathFollower
//! Enables user to click on screen and manipulate path using a table
//!
//! @author Tobias Kaupp
//!
class PathFollowerInput : public IPathInput
{ 
            
    public:
        PathFollowerInput( PathFollowerUserInteraction         &pathFollowerUI,
                           WaypointSettings                    *wpSettings,
                           hydroqguielementutil::IHumanManager &humanManager,
                           const QString                       &lastSavedPathFile );
        
        virtual ~PathFollowerInput() {};  
  
        virtual void paint( QPainter *painter );
        virtual void setUseTransparency( bool useTransparency );
        
        virtual void processPressEvent( QMouseEvent* e);
        virtual void processReleaseEvent( QMouseEvent* e );
        virtual void processDoubleClickEvent( QMouseEvent* e) {};
        virtual void processMoveEvent( QMouseEvent* e) {};
        
        virtual void updateWpSettings( WaypointSettings* wpSettings );
        
        virtual void savePath( const QString &filename );
        virtual void loadPath( const QString &filename );
        virtual void loadPreviousPath();
        
        virtual void sendPath();
        virtual void cancelPath();
        
        virtual void setWaypointFocus( int waypointId );

        virtual bool getPath( orca::PathFollower2dData &pathData ) const;    
        
    private:
               
        PathFollowerUserInteraction &pathFollowerUI_;
        std::auto_ptr<PathDesignScreen> pathDesignScreen_;
        std::auto_ptr<PathDesignTableWidget> pathDesignTableWidget_;
        std::auto_ptr<GuiPath> guiPath_;
        std::auto_ptr<PathFileHandler> pathFileHandler_;
       
};

//!
//! A factory used to create a PathFollowerInput object at runtime
//!
class DefaultPathFollowerInputFactory : public PathFollowerInputFactory
{
public:
    
    std::auto_ptr<IPathInput> 
            createPathFollowerInput( PathFollowerUserInteraction         &pathFollowerUI,
                                     WaypointSettings                    *wpSettings,
                                     hydroqguielementutil::IHumanManager &humanManager,
                                     const QString                       &lastSavedPathFile ) const
            {
                std::auto_ptr<IPathInput> input;
                input.reset( new PathFollowerInput( pathFollowerUI, wpSettings, humanManager, lastSavedPathFile ) );
                return input;     
            }
};      

}

#endif