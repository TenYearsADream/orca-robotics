/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef PATHFOLLOWER_UI_H
#define PATHFOLLOWER_UI_H

#include <memory>
#include <QObject>
#include <orcaice/context.h>
#include <hydroqguipath/pathdesignscreen.h>
#include <hydroqguipath/pathfollowerbuttons.h>
#include <hydroqguipath/ipathinput.h>
#include <orcaqgui2dfactory/pathfollowerinput.h>
#include <orcaqgui2dfactory/pathfilehandler.h>

namespace hydroqgui {
    class GuiElementSet;
}

namespace hydroqguielementutil {
    class MouseEventManager;
    class IHumanManager;
    class ShortcutKeyManager;
}

namespace orcaqgui2d {

class PathFollower2dElement;    
class PathPainter;
    
//!
//! Handles all user interaction with the PathFollower
//!
//! @author Tobias Kaupp
//!
class PathFollowerUserInteraction  : public QObject, 
                                     public hydroqguipath::IPathUserInteraction
{
    Q_OBJECT

public:
    PathFollowerUserInteraction( PathFollower2dElement                          *pfElement,
                                 const std::string                              &proxyString,
                                 hydroqguielementutil::IHumanManager            &humanManager,
                                 hydroqguielementutil::MouseEventManager        &mouseEventManager,
                                 hydroqguielementutil::ShortcutKeyManager       &shortcutKeyManager,
                                 const hydroqgui::GuiElementSet                 &guiElementSet,
                                 const PathPainter                              &painter,
                                 const orcaice::Context                         &context,
                                 const hydroqguipath::PathInputFactory          *inputFactory );

    void noLongerMouseEventReceiver();
    void paint( QPainter *p );
    void setFocus( bool inFocus );
    void setUseTransparency( bool useTransparency ); 

    void mousePressEvent(QMouseEvent *e) 
        {pathInput_->processPressEvent(e);}
    
    void mouseMoveEvent(QMouseEvent *e) 
        {pathInput_->processMoveEvent(e);}
    
    void mouseReleaseEvent(QMouseEvent *e) 
        {pathInput_->processReleaseEvent(e);}
    
    void mouseDoubleClickEvent(QMouseEvent *e) 
        {pathInput_->processDoubleClickEvent(e);}
    

public slots:
    void savePathAs();
    void savePath();
    void waypointSettingsDialog();
    void waypointModeSelected();
    void send();
    void cancel();
    void allGo();
    void allStop();
    void go();
    void stop();
    void toggleEnabled();

private:

    PathFollower2dElement *pfElement_;
    std::string proxyString_;
    hydroqguielementutil::IHumanManager &humanManager_;
    hydroqguielementutil::MouseEventManager &mouseEventManager_;
    hydroqguielementutil::ShortcutKeyManager &shortcutKeyManager_;
    const hydroqgui::GuiElementSet &guiElementSet_;
    const PathPainter &painter_;
    orcaice::Context context_;
    const hydroqguipath::PathInputFactory *inputFactory_;
    hydroqguipath::WaypointSettings wpSettings_;

    // saving the path which the pathfollower *interface* holds
    // (as opposed to the path the user enters in green)
    QString ifacePathFileName_;
    bool haveIfacePathFileName_;
    std::auto_ptr<PathFileHandler> ifacePathFileHandler_;
    
    // handles all user path input through clicking, tables, etc.
    std::auto_ptr<hydroqguipath::IPathInput> pathInput_;
    
    // automatic saving of paths on send
    int numAutoPathDumps_;
    
    // remember the filename of the green user path
    QString loadPreviousPathFilename_;
    
    // sets up and destroys buttons and associated actions
    std::auto_ptr<hydroqguipath::PathfollowerButtons> buttons_;

    // Do we own the global mode?
    bool gotMode_;
    
    bool useTransparency_;
    
};

} // end of namespace

#endif
