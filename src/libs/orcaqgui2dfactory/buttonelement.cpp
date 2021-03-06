#include "buttonelement.h"
#include <iostream>
#include <orcaice/orcaice.h>
#include <hydroqgui/exceptions.h>

using namespace std;

namespace orcaqgui2d {

ButtonElement::ButtonElement( const hydroqguielementutil::GuiElementInfo &guiElementInfo,
                              const orcaice::Context                     &context )
    : hydroqguielementutil::GuiElement(guiElementInfo),
      context_(context),
      proxyString_(guiElementInfo.uniqueId.toStdString()),
      isConnected_(false)
{
}

void
ButtonElement::connect()
{
    try {
        orcaice::connectToInterfaceWithString( context_, buttonPrx_, proxyString_ );
    }
    catch ( Ice::Exception &e )
    {
        stringstream ss;
        ss << "ButtonElement::connect(): failed: " << e;
        _stuff.humanManager->showStatusError( ss.str().c_str() );
        return;
    }
    isConnected_ = true;
}

QStringList
ButtonElement::contextMenu()
{
    QStringList s;
    s<<"Press";
    return s;
}

void
ButtonElement::execute( int action )
{
    switch ( action )
    {
    case 0:
        press();
        break;
    default:
        stringstream ss;
        ss << "execute(): What the hell? bad action: " << action;
        throw hydroqgui::Exception( ERROR_INFO, ss.str() );
        break;
    }
}

void
ButtonElement::press()
{
    if ( !isConnected_ ) connect();
    if ( !isConnected_ ) return;

    const int MAX_TRIES = 2;

    for ( int i=0; i < MAX_TRIES; i++ )
    {
        try {
            buttonPrx_->press();
            break;
        }
        catch ( Ice::Exception &e )
        {
            stringstream ss;
            ss << "ButtonElement: failed to press(): " << e;
            _stuff.humanManager->showStatusError( ss.str().c_str() );
        }
    }
}

}

