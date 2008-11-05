/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
 
#include <gbxutilacfr/tokenise.h>
#include <hydrocolourtext/colourtext.h>
#include <orcaobj/stringutils.h>
#include "display.h"

using namespace std;
using namespace systemstatusmon;      

// local non-member functions
namespace 
{  

std::string toHorizontalLine( int platformCount, int nameWidth, int stateWidth )
{
    stringstream ss;
    ss << hydroctext::toFixedWidth("-----------------------------------", nameWidth) << " +";
    for ( int i=0; i<platformCount; ++i )
        ss << hydroctext::toFixedWidth("-------------------------------------", stateWidth) << " +";
    return ss.str();
}

vector<string> extractPlatComp( const string &input )
{
    vector<string> tokens = gbxutilacfr::tokenise( input.c_str(), "/");
    if (tokens.size()!=2)
    {
        stringstream ss;
        ss << "extractPlatForm: unexpected <platform/component> string: " << input;
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
    return tokens;
}

string extractPlatform( const string &input )
{
    vector<string> tokens = extractPlatComp( input );
    return tokens[0];
}

string extractComponent( const string &input )
{
    vector<string> tokens = extractPlatComp( input );
    return tokens[1];
}

unsigned int extractMaxNumComponents( const orca::SystemStatusData &data )
{
    unsigned int maxNum=0;
    map<string,vector<orca::ObservedComponentStatus> >::const_iterator it;
    
    for ( it=data.systemStatus.begin(); it!=data.systemStatus.end(); ++it )
    {
        if ( (it->second.size()) > maxNum )
            maxNum = it->second.size();
    }
    
    return maxNum;
}
    
std::string stateToString( const orca::ObservedComponentState &state )
{
    switch (state)
    {
        case orca::ObsCompInactive: 
            return "! ";
        case orca::ObsCompConnecting:
            return "^ ";
        case orca::ObsCompInitialising:
            return "- ";
        case orca::ObsCompActive:
            return " ";
        case orca::ObsCompFinalising:
            return "v ";
        case orca::ObsCompDisconnecting:
            return "x ";
        default:
            assert( false && "unknown state type" );
    }
}
void extractStateAndHealth( const orca::ObservedComponentStatus &compData,
                            hydroctext::Style                   &healthStyle,
                            string                              &stateIcon )
{    
    switch (compData.health)
    {
        case orca::ObsCompOk:
            healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Green );
            stateIcon = hydroctext::emph( stateToString(compData.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Green ) );
            return;
        case orca::ObsCompWarning:  
            healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Yellow );
            stateIcon = hydroctext::emph( stateToString(compData.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Yellow ) );
            return;
        case orca::ObsCompFault:  
            healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Red );
            stateIcon = hydroctext::emph( stateToString(compData.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Red ) );
            return;
        case orca::ObsCompStalled:  
            healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Black );
            stateIcon = hydroctext::emph( stateToString(compData.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Black ) );
            return;
        case orca::ObsCompStale:  
            healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::White );
            stateIcon = hydroctext::emph( stateToString(compData.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Black ) );
            return;
        default:
            assert( false && "unknown health type" );
    }
}

// void extractStateAndHealth( const orca::ComponentStatusData &compData,
//                             hydroctext::Style               &healthStyle,
//                             string                          &stateIcon )
// {
//     if (compData.isDataStale) {
//         healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::White );
//         stateIcon = hydroctext::emph( "  ", hydroctext::Style( hydroctext::Reverse, hydroctext::White ) );
//         return;
//     }
//     
//     const orca::SubsystemStatusDict &subSysSt = compData.data.subsystems;
//     
//     if (subSysSt.size()==0)
//         throw gbxutilacfr::Exception(  ERROR_INFO, "SubsystemStatus field is empty" );
//     
//     map<string,orca::SubsystemStatus>::const_iterator itWorstHealth;
// 
//     orca::SubsystemHealth worstHealth = orca::SubsystemOk;           
//     for (map<string,orca::SubsystemStatus>::const_iterator it=subSysSt.begin(); it!=subSysSt.end(); ++it)
//     {
//         // the >= guarantees that itWorstHealth is set at least once
//         if (it->second.health >= worstHealth ) {
//             worstHealth = it->second.health;
//             itWorstHealth = it;
//         }
//     }
//     
//     switch (worstHealth)
//     {
//         case orca::SubsystemOk:
//             healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Green );
//             stateIcon = hydroctext::emph( stateToString(itWorstHealth->second.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Green ) );
//             return;
//         case orca::SubsystemWarning:  
//             healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Yellow );
//             stateIcon = hydroctext::emph( stateToString(itWorstHealth->second.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Yellow ) );
//             return;
//         case orca::SubsystemFault:  
//             healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Red );
//             stateIcon = hydroctext::emph( stateToString(itWorstHealth->second.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Red ) );
//             return;
//         case orca::SubsystemStalled:  
//             healthStyle = hydroctext::Style( hydroctext::Reverse, hydroctext::Black );
//             stateIcon = hydroctext::emph( stateToString(itWorstHealth->second.state), hydroctext::Style( hydroctext::Reverse, hydroctext::Black ) );
//             return;
//         default:
//             assert( false && "unknown health type" );
//     }
// }

std::string toShortString( const orca::ObservedComponentStatus& compData, int stateWidth )
{   
    hydroctext::Style healthStyle;
    string stateIcon;
    extractStateAndHealth( compData, healthStyle, stateIcon );
    
    stringstream ss;
    int stateUsedWidth = 2;
    string compPlat = orcaobj::toString(compData.name);
    ss << stateIcon
            << hydroctext::emph(hydroctext::toFixedWidth(extractComponent(compPlat),stateWidth-stateUsedWidth), healthStyle );
    
    return ss.str();
}

std::string toBoldString( const string &input )
{
    return hydroctext::emph(input, hydroctext::Style( hydroctext::Bold ) );
}

std::string humanErrorMsgString( const orca::SystemStatusData &ssData )
{
    stringstream ss;
    
    map<string,vector<orca::ObservedComponentStatus> >::const_iterator itCs;
    for ( itCs=ssData.systemStatus.begin(); itCs!=ssData.systemStatus.end(); ++itCs )
    {
        const vector<orca::ObservedComponentStatus> &components = itCs->second;
        
        // per component
        for (unsigned int i=0; i<components.size(); i++)
        {
            string platComp = orcaobj::toString(components[i].name);   
            const orca::SubsystemStatusDict &subSysSt = components[i].subsystems;
            map<string,orca::SubsystemStatus>::const_iterator itSs;
            
            // per subsystem
            for ( itSs=subSysSt.begin(); itSs!=subSysSt.end(); ++itSs )
            {
                if (itSs->second.health!=orca::SubsystemOk)
                    ss << toBoldString(platComp + ", " + itSs->first + ": ") << endl
                        << itSs->second.message << endl << endl;
            }
        }
    }
    
    return ss.str();
}

} // end of namespace
   
////////////////////////////////////// 

ColourTextDisplay::ColourTextDisplay( const orcaice::Context     &context,
                                      gbxiceutilacfr::SafeThread *thread)
    : context_(context)
{
    consumer_ = new orcaifaceimpl::BufferedSystemStatusConsumerImpl( -1, gbxiceutilacfr::BufferTypeCircular, context );
    consumer_->subscribeWithTag( "SystemStatus", thread );
}

void 
ColourTextDisplay::refresh()
{
    if (consumer_->buffer().isEmpty()) {
        context_.tracer().debug( "ColourTextDisplay:refresh(): systemstatus buffer is empty", 5 );
        return;
    }
        
    orca::SystemStatusData data;
    consumer_->buffer().getAndPop( data );
    
    try 
    {
        display( data );
    }
    catch (std::exception &e)
    {
        stringstream ss;
        ss << "ColourTextDisplay: Caught unexpected exception: " << e.what();
        context_.tracer().error( ss.str() );
    }
}
    
void
ColourTextDisplay::display( const orca::SystemStatusData &data )
{   
    map<string,vector<orca::ObservedComponentStatus> >::const_iterator it;
    
    const string sep = " |";
    const int compNameWidth = 20;
    const int compStateWidth = 22;   

    // start with "clean" colour settings
    cout << hydroctext::reset();  
    
    // platform names
    cout << hydroctext::toFixedWidth( toBoldString("PLATFORMS"), compNameWidth ) << sep;
    for ( it=data.systemStatus.begin(); it!=data.systemStatus.end(); ++it )
    {
        cout << hydroctext::toFixedWidth( "    " + toBoldString(it->first), compStateWidth ) << sep;
    }
    cout << endl;
    
    // horizontal line
    cout << toHorizontalLine(data.systemStatus.size(),compNameWidth,compStateWidth) << endl;
    
    // find maximum number of components per platform -> number of rows
    unsigned int maxComp = extractMaxNumComponents( data );
    
    for (unsigned int compCounter=0; compCounter<maxComp; compCounter++)
    {

        // row label
        if (compCounter==0)
            cout << hydroctext::toFixedWidth( toBoldString("COMPONENTS"), compNameWidth ) << sep;
        else
            cout << hydroctext::toFixedWidth( " ", compNameWidth ) << sep;
        
        // per platform (column)
        for ( it=data.systemStatus.begin(); it!=data.systemStatus.end(); ++it )
        {
            const vector<orca::ObservedComponentStatus> &components = it->second;
            if ( compCounter >= components.size() )
                continue;
            
            cout << toShortString(components[compCounter],compStateWidth) << sep;
        }
        cout << endl;
    }
    
    // print the human-readable text, TODO: make configurable? or interactive?
    cout << endl;
    cout << humanErrorMsgString( data );
    
    // space between consecutive records
    cout << endl << endl;
}