/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp, Ben Upcroft
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef ORCAGUI_DEFAULT_FACTORY_H
#define ORCAGUI_DEFAULT_FACTORY_H

#include <orcaqgui/iguielementfactory.h>
#include <orcaice/context.h>

namespace orcaqgui2d
{

    //!
    //! Generates 2D Gui Elements based on its type.
    //!
    class DefaultFactory : public orcaqgui::IGuiElementFactory    
    {
    public:

        DefaultFactory();

        void setContext( const orcaice::Context &context )
            { context_ = context; isContextSet_ = true; }

        // returns a NULL pointer if something goes wrong
        virtual hydroqguielementutil::GuiElement* create(  const hydroqguielementutil::GuiElementInfo &guiElementInfo ) const;
                
        bool lookupElementType( const QStringList &ids, QString &elementType ) const;

    private:
        
        bool                      isContextSet_;
        orcaice::Context          context_;

    };

}


//
// Hook for dynamic loading
//
extern "C" {
    hydroqgui::IGuiElementFactory *createFactory();
}

#endif
