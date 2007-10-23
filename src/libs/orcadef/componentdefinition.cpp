/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <sstream>
#include <assert.h>

#include <orcaobj/orcaobj.h>
#include <orcaice/orcaice.h>

#include "componentdefinition.h"
#include "exceptions.h"

using namespace std;

namespace orcadef {

const std::string
toDefText( const ComponentDef &def )
{
    std::stringstream ss;
    //assert( !def.tag.empty() );
    if ( def.tag.empty() ) {
        throw ParseException( ERROR_INFO, "Component tag is empty" );
    }

    ss << "DefaultComponentTag=" << def.tag << endl;
    ss << "DefaultComponentEndpoints=" << def.endpoints << endl;

    for ( unsigned int i=0; i<def.comments.size(); ++i ) {
        ss << "Component.Comment=" << def.comments[i] << endl;
    }
    ss << endl;

    if ( def.provided.size() > 0 )
    {
        ss << "# Provided Interfaces" << endl;
        for ( unsigned int i=0; i < def.provided.size(); i++ )
        {
            //assert( !cfg.provided[i].name.empty() );
            if ( def.provided[i].name.empty() ) {
                throw ParseException( ERROR_INFO, "Provided interface name is empty." );
            }
            ss << "Provides." << def.provided[i].tag
                   << ".Type=" << def.provided[i].type << endl;
            ss << "Provides." << def.provided[i].tag
                   << ".DefaultName=" << def.provided[i].name << endl;
            ss << endl;
        }
    }
    
    if ( def.required.size() > 0 )
    {
        ss << "# Required Interfaces" << endl;
        for ( unsigned int i=0; i < def.required.size(); ++i )
        {
            ss << "Requires." << def.required[i].tag << ".Type="
               << def.required[i].type << endl;
            ss << "Requires." << def.required[i].tag << ".DefaultProxy="
               << def.required[i].proxy << endl;
            ss << endl;
        }
    }
    

    ss << "# Configuration Options" << endl;
    // alexm: what about the configs? did it ever worked?
    ss << endl;

    return ss.str();
}

ComponentCfg
toCfg( const ComponentDef & def )
{
    if ( def.tag.empty() ) {
        throw ParseException( ERROR_INFO, "Component tag is empty" );
    }
    
    ComponentCfg cfg;

    cfg.fqname.platform     = "local";
    cfg.fqname.component    = hydroutil::toLowerCase(def.tag);
    cfg.endpoints           = def.endpoints;

    //
    // Provided Interfaces
    //
    ProvidedCfg p;
    for ( unsigned int i=0; i < def.provided.size(); ++i )
    {
        p.name = def.provided[i].name;
        p.tag = def.provided[i].tag;
        
        //assert( !p.tag.empty() );
        if ( p.tag.empty() ) {
            throw ParseException( ERROR_INFO, "Provided interface tag is empty" );
        }
    
        // default
        if ( p.name.empty() ) {
            p.name = hydroutil::toLowerCase(p.tag);
        }
        
        cfg.provided.push_back(p);
    }
    
    //
    // Required Interfaces
    //
    RequiredCfg r;
    for ( unsigned int i=0; i < def.required.size(); ++i )
    {
        r.tag = def.required[i].tag;
        r.proxy = def.required[i].proxy;

        // apply defaults: direct proxy
        if ( r.proxy.empty() ) {
            orca::FQInterfaceName fqname;
            fqname.platform = std::string("local");
            fqname.component = hydroutil::toLowerCase(r.tag);
            fqname.iface = hydroutil::toLowerCase(r.tag);
            
            r.proxy = orcaice::toString( fqname );
        }

        cfg.required.push_back(r);
    }
    
    //
    // Configuration Parameters
    //
    ConfigCfg c;
    // component configs
    for ( unsigned int i=0; i < def.configs.size(); ++i )
    {
        c.tag = def.configs[i].tag;
        c.value = def.configs[i].value;
        cfg.configs.push_back(c);
    }
    // external configs
    for ( unsigned int i=0; i < def.externalConfigs.size(); ++i )
    {
        c.tag = def.externalConfigs[i].tag;
        c.value = def.externalConfigs[i].value;
        cfg.externalConfigs.push_back(c);
    }

    return cfg;
}

const std::string
toCfgText( const ComponentDef &def )
{
    ComponentCfg cfg = toCfg( def );

    std::stringstream ss;
    //assert( !def.tag.empty() );
    if ( def.tag.empty() ) {
        throw ParseException( ERROR_INFO, "Component tag is empty" );
    }

    ss << "# Orca version " << hydroutil::orcaVersion() << endl;
//     if ( !def.filename.empty() ) {
//         ss << "# Autogenerated from " << def.filename << endl;
//     }
//     else {
//         ss << "# Autogenerated from a .def file" << endl;
//     }
    ss << endl;

    ss << "# Component" << endl;
    ss << def.tag << ".Platform=" << cfg.fqname.platform << endl;
    ss << def.tag << ".Component=" << cfg.fqname.component << endl;
    // only print out endpoint info if special endpoints were specified
    if ( def.isSpecialEndpoints ) {
        ss << def.tag << ".Endpoints=" << cfg.endpoints << endl;
    }
    ss << endl;

    //assert( def.provided.size()==cfg.provided.size() );
    if ( def.provided.size()!=cfg.provided.size() ) {
        throw ParseException( ERROR_INFO, "Number of provided interfaces in def and cfg sturctures do not match." );
    }
    if ( cfg.provided.size() > 0 )
    {
        ss << "# Provided Interfaces" << endl;
        for ( unsigned int i=0; i < cfg.provided.size(); i++ )
        {
            //assert( !cfg.provided[i].name.empty() );
            if ( cfg.provided[i].name.empty() ) {
                throw ParseException( ERROR_INFO, "Provided interface name is empty." );
            }
            ss << def.tag 
               << ".Provides." << cfg.provided[i].tag
               << ".Name=" << cfg.provided[i].name << endl;
        }
        ss << endl;
    }
    
    //assert( def.required.size()==cfg.required.size() );
    if ( def.required.size()!=cfg.required.size() ) {
        throw ParseException( ERROR_INFO, "Number of required interfaces in def and cfg sturctures do not match." );
    }
    if ( cfg.required.size() > 0 )
    {
        ss << "# Required Interfaces" << endl;
        for ( unsigned int i=0; i < cfg.required.size(); ++i )
        {
            ss << def.tag 
               << ".Requires." << cfg.required[i].tag 
               << ".Proxy=" << cfg.required[i].proxy << endl;
        }
        ss << endl;
    }
    
    //assert( def.configs.size()==cfg.configs.size() );
    if ( def.configs.size()!=cfg.configs.size() ) {
        throw ParseException( ERROR_INFO, "Number of component config entries in def and cfg structs do not match." );
    }
    if ( !cfg.configs.empty() )
    {
        ss << "# Component Configuration Options" << endl;
        for ( unsigned int i=0; i < cfg.configs.size(); ++i )
        {
            if ( def.configs[i].comment != "" )
            {
                ss << "# " << def.configs[i].comment << endl;
            }
            //
            // This behavior is OBSOLETE and will be removed soon.
            // Use ExternalConfig tag to specify external configs.
            //
            // 2 cases: internal and external configs.
            if ( def.configs[i].isExternal ) {
                // External ones are printed as is
                ss << cfg.configs[i].tag << "=" << cfg.configs[i].value << endl;
            }
            else {
                // Internal ones are preceded by component tag.
                ss << def.tag << ".Config." << cfg.configs[i].tag << "=" << cfg.configs[i].value << endl;
            }
        }
        ss << endl;
    }

    //assert( def.externalConfigs.size()==cfg.externalConfigs.size() );
    if ( def.externalConfigs.size()!=cfg.externalConfigs.size() ) {
        throw ParseException( ERROR_INFO, "Number of external config entries in def and cfg structs do not match." );
    }
    if ( !cfg.externalConfigs.empty() )
    {
        ss << "# External Configuration Options" << endl;
        for ( unsigned int i=0; i < cfg.externalConfigs.size(); ++i )
        {
            if ( !def.externalConfigs[i].comment.empty() )
            {
                ss << "# " << def.externalConfigs[i].comment << endl;
            }
            // External configs are printed as is!
            ss << cfg.externalConfigs[i].tag << "=" << cfg.externalConfigs[i].value << endl;
        }
        ss << endl;
    }

    return ss.str();
}

const std::string 
toCfgTextWithHeaders( const ComponentDef & def, const std::vector<std::string> & headers  )
{
    std::stringstream ss;

    // Place extra headers, one line at a time
    for ( unsigned int i=0; i<headers.size(); ++i ) {
        ss << headers[i] << endl;
    }

    ss << toCfgText( def );

    return ss.str();
}

const std::string
toXmlText( const ComponentDef &def, bool withTemplate )
{
    ComponentCfg cfg = toCfg( def );

    std::stringstream ss;
    //assert( !def.tag.empty() );
    if ( def.tag.empty() ) {
        throw ParseException( ERROR_INFO, "Component tag is empty" );
    }
    // define tab to be 4 spaces.
    std::string tab = "    ";

    ss << "<!-- Orca version " << hydroutil::orcaVersion() << " -->" << endl;
    ss << endl;

    ss << "<icegrid>" << endl;

    ss <<tab
        <<"<application "
        <<"name=\"OrcaApp\">" << endl;

    if ( withTemplate )
    {
        ss << toXmlTemplateText( def );
    }

    ss <<tab<<tab
        <<"<node "
        <<"name=\"Node1\">" << endl;

    ss <<tab<<tab<<tab
        <<"<server "
        <<"id=\""<<cfg.fqname.platform<<"."<<cfg.fqname.component<<"\" "
        <<"exe=\""<<cfg.fqname.component<<"\" "
        <<"activation=\"on-demand\">" << endl;

    ss <<tab<<tab<<tab<<tab
        <<"<adapter "
        <<"name=\""<<def.tag<<"\" "
        // it's enough to state special endpoints under the adapter name below
//         <<"endpoints=\""<<def.endpoints<<"\" "
        <<"register-process=\"true\" "
        <<"id=\""<<cfg.fqname.platform<<"/"<<cfg.fqname.component<<"\"/>" << endl;

    // We can register Home as a well known objects easily from here
    // but we do it programatically from libOrcaIce so it's done the same way with and 
    // without IceGrid
    // Well-known objects
//     ss <<tab<<tab<<tab<<tab<<tab
//         <<"<object "
//         <<"identity=\"orca."<<cfg.fqname.platform<<"."<<cfg.fqname.component<<"/Home\" "
//         << "type=\"::orca::Home\"/>" << endl;
// 
//     ss <<tab<<tab<<tab<<tab<<"</adapter>" << endl;

    // special parameters
//     ss <<tab<<tab<<tab<<tab<<"<!-- Component properties -->" << endl;
    ss <<tab<<tab<<tab<<tab<<"<property name=\""<<def.tag<<".Platform\" value=\"local\"/>" << endl;
    ss <<tab<<tab<<tab<<tab<<"<property name=\""<<def.tag<<".Component\" value=\""<<cfg.fqname.component<<"\"/>" << endl;
    if ( def.isSpecialEndpoints ) {
        ss <<tab<<tab<<tab<<tab<<"<property name=\""<<def.tag<<".Endpoints\" value=\""<<def.endpoints<<"\"/>" << endl;
    }

    // Provided Interfaces
    if ( def.provided.size()!=cfg.provided.size() ) {
        throw ParseException( ERROR_INFO, "Number of provided interfaces in def and cfg sturctures do not match." );
    }
//     ss <<tab<<tab<<tab<<tab<<"<!-- Provided interfaces -->" << endl;
    for ( unsigned int i=0; i < cfg.provided.size(); ++i ) {
        ss <<tab<<tab<<tab<<tab
            <<"<property "
            <<"name=\""<<def.tag<<".Provides."<<cfg.provided[i].tag<<".Name\" "
            <<"value=\""<<cfg.provided[i].name<<"\"/>" << endl;
    }

    // Required Interfaces
    if ( def.required.size()!=cfg.required.size() ) {
        throw ParseException( ERROR_INFO, "Number of required interfaces in def and cfg sturctures do not match." );
    }
//     ss <<tab<<tab<<tab<<tab<<"<!-- Required interfaces -->" << endl;
    for ( unsigned int i=0; i < cfg.required.size(); ++i )
    {
        ss <<tab<<tab<<tab<<tab
            <<"<property "
            <<"name=\""<<def.tag<<".Requires."<<cfg.required[i].tag<<".Proxy\" "
            <<"value=\""<<cfg.required[i].proxy<<"\"/>" << endl;
    }

    //
    // This behavior is OBSOLETE and will be removed soon.
    // Use ExternalConfig tag to specify external configs.
    //
    // component configs
//     ss <<tab<<tab<<tab<<tab<<"<!-- Component configuration parameters -->" << endl;
    for ( unsigned int i=0; i < def.configs.size(); ++i ) {
        // 2 cases: internal and external configs.
        if ( def.configs[i].isExternal ) {
            // External ones are printed as is
            ss <<tab<<tab<<tab<<tab
                <<"<property "
                <<"name=\""<<def.configs[i].tag<<"\" "
                <<"value=\""<<def.configs[i].value<<"\"/>" << endl;
        }
        else {
            // Internal ones are preceded by component tag.
            ss <<tab<<tab<<tab<<tab
                <<"<property "
                <<"name=\""<<def.tag<<".Config."<<def.configs[i].tag<<"\" "
                <<"value=\""<<def.configs[i].value<<"\"/>" << endl;
        }
    }

    // external configs
//     ss <<tab<<tab<<tab<<tab<<"<!-- External configuration parameters -->" << endl;
    for ( unsigned int i=0; i < def.externalConfigs.size(); ++i ) {
        // External configs are printed as is
        ss <<tab<<tab<<tab<<tab
            <<"<property "
            <<"name=\""<<def.externalConfigs[i].tag<<"\" "
            <<"value=\""<<def.externalConfigs[i].value<<"\"/>" << endl;
    }

    ss <<tab<<tab<<tab<<"</server>" << endl;
    ss <<tab<<tab<<"</node>" << endl;
    ss <<tab<<"</application>" << endl;
    ss <<"</icegrid>" << endl;

    return ss.str();
}

const std::string
toXmlTemplateText( const ComponentDef &def )
{
    ComponentCfg cfg = toCfg( def );

    std::stringstream ss;
    //assert( !def.tag.empty() );
    if ( def.tag.empty() ) {
        throw ParseException( ERROR_INFO, "Component tag is empty" );
    }
    // define tab to be 4 spaces.
    std::string tab = "    ";

//     ss << "<!-- Orca version " << hydroutil::orcaVersion() << " -->" << endl;
//     ss << endl;
// 
//     ss << "<icegrid>" << endl;
// 
//     ss <<tab
//         <<"<application "
//         <<"name=\"OrcaApp\">" << endl;
// 
//     ss <<tab<<tab
//         <<"<distrib/>" << endl;

    ss <<tab<<tab
        <<"<server-template "
        <<"id=\""<<cfg.fqname.component<<"Template\">" << endl;

    ss <<tab<<tab<<tab
        <<"<parameter name=\"platform\" default=\"local\"/>" << endl;

    ss <<tab<<tab<<tab
        <<"<server "
        <<"id=\"${platform}."<<cfg.fqname.component<<"\" "
        <<"exe=\"${application.distrib}/orca2/bin/"<<cfg.fqname.component<<"\" "
        <<"activation=\"on-demand\">" << endl;

    ss <<tab<<tab<<tab<<tab
        <<"<adapter "
        <<"name=\""<<def.tag<<"\" "
        // it's enough to state special endpoints under the adapter name below
//         <<"endpoints=\""<<def.endpoints<<"\" "
        <<"register-process=\"true\" "
        <<"id=\"${platform}/"<<cfg.fqname.component<<"\"/>" << endl;

    // We can register Home as a well known objects easily from here
    // but we do it programatically from libOrcaIce so it's done the same way with and 
    // without IceGrid
//     ss <<tab<<tab<<tab<<tab<<tab
//         <<"<object "
//         <<"identity=\"${platform}."<<cfg.fqname.component<<".home\" "
//         << "type=\"::orca::Home\"/>" << endl;
// 
//     ss <<tab<<tab<<tab<<tab<<"</adapter>" << endl;

    // special parameters
//     ss <<tab<<tab<<tab<<tab<<"<!-- Component properties -->" << endl;
    ss <<tab<<tab<<tab<<tab<<"<property name=\""<<def.tag<<".Platform\" value=\"local\"/>" << endl;
    ss <<tab<<tab<<tab<<tab<<"<property name=\""<<def.tag<<".Component\" value=\""<<cfg.fqname.component<<"\"/>" << endl;
    if ( def.isSpecialEndpoints ) {
        ss <<tab<<tab<<tab<<tab<<"<property name=\""<<def.tag<<".Endpoints\" value=\""<<def.endpoints<<"\"/>" << endl;
    }

    // Provided Interfaces
    if ( def.provided.size()!=cfg.provided.size() ) {
        throw ParseException( ERROR_INFO, "Number of provided interfaces in def and cfg sturctures do not match." );
    }
//     ss <<tab<<tab<<tab<<tab<<"<!-- Provided interfaces -->" << endl;
    for ( unsigned int i=0; i < cfg.provided.size(); ++i ) {
        ss <<tab<<tab<<tab<<tab
            <<"<property "
            <<"name=\""<<def.tag<<".Provides."<<cfg.provided[i].tag<<".Name\" "
            <<"value=\""<<cfg.provided[i].name<<"\"/>" << endl;
    }

    // Required Interfaces
    if ( def.required.size()!=cfg.required.size() ) {
        throw ParseException( ERROR_INFO, "Number of required interfaces in def and cfg sturctures do not match." );
    }
//     ss <<tab<<tab<<tab<<tab<<"<!-- Required interfaces -->" << endl;
    for ( unsigned int i=0; i < cfg.required.size(); ++i )
    {
        ss <<tab<<tab<<tab<<tab
            <<"<property "
            <<"name=\""<<def.tag<<".Requires."<<cfg.required[i].tag<<".Proxy\" "
            <<"value=\""<<cfg.required[i].proxy<<"\"/>" << endl;
    }

    //
    // This behavior is OBSOLETE and will be removed soon.
    // Use ExternalConfig tag to specify external configs.
    //
    // component configs
//     ss <<tab<<tab<<tab<<tab<<"<!-- Component configuration parameters -->" << endl;
    for ( unsigned int i=0; i < def.configs.size(); ++i ) {
        // 2 cases: internal and external configs.
        if ( def.configs[i].isExternal ) {
            // External ones are printed as is
            ss <<tab<<tab<<tab<<tab
                <<"<property "
                <<"name=\""<<def.configs[i].tag<<"\" "
                <<"value=\""<<def.configs[i].value<<"\"/>" << endl;
        }
        else {
            // Internal ones are preceded by component tag.
            ss <<tab<<tab<<tab<<tab
                <<"<property "
                <<"name=\""<<def.tag<<".Config."<<def.configs[i].tag<<"\" "
                <<"value=\""<<def.configs[i].value<<"\"/>" << endl;
        }
    }

    // external configs
//     ss <<tab<<tab<<tab<<tab<<"<!-- External configuration parameters -->" << endl;
    for ( unsigned int i=0; i < def.externalConfigs.size(); ++i ) {
        // External configs are printed as is
        ss <<tab<<tab<<tab<<tab
            <<"<property "
            <<"name=\""<<def.externalConfigs[i].tag<<"\" "
            <<"value=\""<<def.externalConfigs[i].value<<"\"/>" << endl;
    }

    ss <<tab<<tab<<tab<<"</server>" << endl;
    ss <<tab<<tab<<"</server-template>" << endl;

//     // sample use of the server template
//     ss <<tab<<tab<<"<node name=\"Localhost\">" << endl;
//     ss <<tab<<tab<<tab
//         <<"<server-instance "
//         <<"template=\""<<cfg.fqname.component<<"Template\" "
//         <<"platform=\"localhost\"/>" << endl;
//     ss <<tab<<tab<<"</node>" << endl;
// 
//     ss <<tab<<"</application>" << endl;
//     ss <<"</icegrid>" << endl;

    return ss.str();
}

ComponentDef
invertDef( const ComponentDef & def, const std::string & invTag )
{
    ComponentDef invDef;

    invDef.tag = invTag;
    // NOTE: this only works with indirect proxies
    invDef.endpoints = "tcp -t 5000";
    invDef.comments.push_back( "autogenerated by inverting "+def.tag );
    
    // output provided interfaces are determined by input required ones
    ProvidedDef p;
    for ( unsigned int i=0; i < def.required.size(); ++i )
    {
        p.tag = def.required[i].tag;
        p.type = def.required[i].type;
        // NOTE: this works only for indirect proxies
        p.name = orcaice::toInterfaceName( def.required[i].proxy ).iface;

        invDef.provided.push_back(p);
    }

    // output required interfaces are determined by input provided ones
    RequiredDef r;
    for ( unsigned int i=0; i < def.provided.size(); ++i )
    {
        r.tag = def.provided[i].tag;
        r.type = def.provided[i].type;

        orca::FQInterfaceName fqname;
        fqname.platform = std::string("local");
        fqname.component = hydroutil::toLowerCase(def.tag);
        fqname.iface = def.provided[i].name;
        r.proxy = orcaice::toString( fqname );

        invDef.required.push_back(r);
    }

    return invDef;
}

const std::string
toString( const ProvidedDef &d )
{
    std::stringstream ss;
    ss<<"    Tag:    " << d.tag << endl;
    ss<<"    Type:   " << d.type << endl;
    ss<<"    Name:   " << d.name << endl;
    return ss.str();
}

const std::string
toString( const RequiredDef &d )
{
    std::stringstream ss;
    ss<<"    Tag:    " << d.tag << endl;
    ss<<"    Type:   " << d.type << endl;
    ss<<"    Proxy:  " << d.proxy << endl;
    return ss.str();
}

const std::string
toString( const ConfigDef &d )
{
    std::stringstream ss;
    ss<<"    Tag:     " << d.tag << endl;
    ss<<"    Type:    " << d.type << endl;
    ss<<"    Comment: " << d.comment << endl;
    ss<<"    Value:   " << d.value << endl;
    ss<<"    External:" << (int)d.isExternal << endl;
    return ss.str();
}

const std::string
toString( const ComponentDef &d )
{
    std::stringstream ss;

    ss<<"  tag: " << d.tag << endl;
    ss<<"  endpoints: " << d.endpoints << endl;
    for ( unsigned int i=0; i < d.comments.size(); i++ )
    {
        ss<<"  comments[" << i << "]: " << d.comments[i] << endl;
    }
    
    if ( d.provided.size() > 0 )
    {
        ss<<"  Provided Interfaces: " << endl;
        for ( unsigned int i=0; i < d.provided.size(); i++ )
        {
            ss << toString( d.provided[i] );
        }
    }
    if ( d.required.size() > 0 )
    {
        ss<<"  Required Interfaces:" << endl;
        for ( unsigned int i=0; i < d.required.size(); i++ )
        {
            ss << toString( d.required[i] );
        }
    }
    if ( d.configs.size() > 0 )
    {
        ss<<"  Component Configuration Parameters:" << endl;
        for ( unsigned int i=0; i < d.configs.size(); i++ )
        {
            ss << toString( d.configs[i] );
        }
    }
    if ( d.externalConfigs.size() > 0 )
    {
        ss<<"  External Configuration Parameters:" << endl;
        for ( unsigned int i=0; i < d.externalConfigs.size(); i++ )
        {
            ss << toString( d.externalConfigs[i] );
        }
    }
    return ss.str();
}

const std::string
toString( const ProvidedCfg &c )
{
    stringstream ss;
    
    ss<<"    Tag:   " << c.tag << endl;
    ss<<"    Name:  " << c.name << endl;

    return ss.str();
}

const std::string
toString( const RequiredCfg &c )
{
    stringstream ss;

    ss<<"    Tag:   " << c.tag << endl;
    ss<<"    Proxy: " << c.proxy << endl;

    return ss.str();
}

const std::string
toString( const ConfigCfg &c )
{
    stringstream ss;

    ss<<"    Tag:   " << c.tag << endl;
    ss<<"    Value: " << c.value << endl;

    return ss.str();
}

const std::string
toString( const ComponentCfg &c )
{
    stringstream ss;

    ss<<"  fqName:       " << orcaice::toString(c.fqname) << endl;
    ss<<"  fqExecutable: " << orcaice::toString(c.fqexecutable) << endl;
    ss<<"  Endpoints:    " << c.endpoints << endl;

    if ( c.provided.size() > 0 )
    {
        ss<<"  Provided interfaces:" << endl;
        for ( unsigned int i=0; i < c.provided.size(); i++ )
        {
            ss << toString(c.provided[i]);
        }
    }
    if ( c.required.size() > 0 )
    {
        ss<<"  Required interfaces:" << endl;
        for ( unsigned int i=0; i < c.required.size(); i++ )
        {
            ss << toString(c.required[i]);
        }
    }
    if ( c.configs.size() > 0 )
    {
        ss<<"  Component Configuration Parameters:" << endl;
        for ( unsigned int i=0; i < c.configs.size(); i++ )
        {
            ss << toString(c.configs[i]);
        }
    }
    if ( c.externalConfigs.size() > 0 )
    {
        ss<<"  External Configuration Parameters:" << endl;
        for ( unsigned int i=0; i < c.externalConfigs.size(); i++ )
        {
            ss << toString(c.externalConfigs[i]);
        }
    }

    return ss.str();
}

} // namespace
