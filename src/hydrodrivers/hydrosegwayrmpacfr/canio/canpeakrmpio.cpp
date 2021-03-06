/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 * Copyright (c) 2004-2009 Duncan Mercer
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
 
//******************************************************************
// Written by Duncan Mercer ACFR March 2007 
//********************************************************************
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <ctime>

#include <libpcan.h>  //stuff to access the peak can drivers
#include <pcan.h>

#include <gbxsickacfr/gbxiceutilacfr/timer.h>
#include <hydrointerfaces/segwayrmp.h> // for Exception
#include <gbxutilacfr/exceptions.h>
#include "canpeakrmpio.h"
#include "peakutil.h"

using namespace std;
using namespace segwayrmpacfr;

namespace {

//***********************************************************************
void 
convertCanPacketToPeak(TPCANMsg *peakCanOut, const CanPacket *pktIn ){

    peakCanOut->ID      =  pktIn->id();
    peakCanOut->LEN     =  CanPacket::CAN_DATA_SIZE;
    memcpy(peakCanOut->DATA, pktIn->msg(), CanPacket::CAN_DATA_SIZE * sizeof(BYTE));

    // Use the peak driver definition for a standard (not extended type message)
    // Note that this does not match that used in the FDTI library (see below)
    peakCanOut->MSGTYPE =  MSGTYPE_STANDARD; 
}

//***********************************************************************
void 
convertPeakToCanPacket(CanPacket *pktOut, const TPCANMsg *peakCanIn )
{
    pktOut->setId( peakCanIn->ID );
    assert( peakCanIn->LEN == CanPacket::CAN_DATA_SIZE );
    memcpy(pktOut->msg(), peakCanIn->DATA, CanPacket::CAN_DATA_SIZE * sizeof(BYTE));

    // NOTE:- We do not change the value in the flags field. It seems the 
    // different drivers peak / FDTI etc have different definitions of how a standard
    // (not extended) can message is denoted.
    
    /* These two never need to change and are set in the default constructor for CanPacket
      pktOut -> flags   =  pktOut -> flags; 
      pktOut -> dlc     =  CAN_DATA_SIZE;
    */

}

//*****************************************************************************

}

//////////////////////////////////////////////////////////////////////

//**************************************************************
// Take the name of the port and attempt to open it, throws an
// exception if fails.
CanPeakRmpIo::CanPeakRmpIo(const string & portName):
    isEnabled_(false), 
    debugLevel_(0) 
{    
    // open the CAN port and use as a chardev type end point
    portHandle_ = LINUX_CAN_Open(portName.c_str(), O_RDWR); 

    if( ! portHandle_ )
    {
        stringstream ss;
        ss << "CanPeakRmpIo::constructor(): Error: "<<
            "Unable to open the can port '" << portName << "': "<<strerror(nGetLastError()) << endl;
// AlexB: I don't understand why, but a
// hydrointerfaces::SegwayRmp::Exception appears to result in a
// seg-fault
//        throw hydrointerfaces::SegwayRmp::Exception( ss.str() );
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
    else
    {
        std::cout << "CAN port opened properly\n";
    }
}



//*************************************************************
CanPeakRmpIo::~CanPeakRmpIo()
{
    cout<<"TRACE(canpeakrmpio.cpp): " << __func__ << endl;

    try {
        DWORD retVal = 0;
        stringstream ss;

        if ( portHandle_ )
        {
            retVal = CAN_Close(portHandle_);  // close access to the CAN port
            if(retVal != 0 ){
                ss << "CanPeakRmpIo::destructor(): Error: "<<
                    "Unable to close can port: " << errorInfo(retVal);
                throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
            }
            portHandle_ = NULL;      // clear the port handle
            isEnabled_ = false;
        }
    }
    catch ( std::exception &e )
    {
        // Don't throw exceptions from destructor
        cout << e.what() << endl;
    }
}


//*************************************************************
// Function to read a CAN packet from the card. Will
// return NO_DATA in the event of a timeout or a failed
// read. Will wait for just over 10msec (=> 100Hz) for data before 
// timing out

RmpIo::RmpIoStatus 
CanPeakRmpIo::readPacket(CanPacket &pkt){

    // Set the default timeout to 100Hz
    const int timeOutMicroSeconds=10000*1.2;

    assert ( isEnabled_ );

    // Using data structs from pcan.h
    TPCANRdMsg canDataReceived;
     
    // Call the peak library code
    DWORD retVal = LINUX_CAN_Read_Timeout(portHandle_, &canDataReceived, timeOutMicroSeconds ); 

    if ( retVal == 0 )
    {
        if ( canDataReceived.Msg.MSGTYPE != MSGTYPE_STANDARD )
        {
            cout<<"TRACE(canpeakrmpio.cpp): Ignoring special message: " << toString(canDataReceived) << endl;
            return NO_DATA;
        }
        convertPeakToCanPacket(&pkt, &canDataReceived.Msg);
        return OK;
    }
    else
    {
//      if( debugLevel_ > 0)
        cout << __func__ << ": LINUX_CAN_Read_Timeout failed (t="<<timeOutMicroSeconds<<"us). "<<errorInfo(retVal)<<endl;
        return NO_DATA;
    }
}





//***********************************************************************
// 
// throws an Exception if the write fails.

void CanPeakRmpIo::writePacket(const CanPacket &pktToSend){

    assert ( isEnabled_ );

    /*  //debug
        double msecs=writeTimer_.elapsed().toMilliSecondsDouble();
        writeTimer_.restart();
        cout << "Sending " << pktToSend->toString()<<"OrcaTimer msec " << msecs <<endl;
    */

    // We are supposed to be able to write data at ~ 100Hz, allow a timeout equivalent to 
    // 50Hz by default
    const int timeOutMicroSeconds = 20000;  
    
    // Using data structs from pcan.h
    TPCANMsg canDataToSend;
    
    convertCanPacketToPeak( &canDataToSend, &pktToSend );

    // Send to the peak device driver code
    DWORD retVal = LINUX_CAN_Write_Timeout(portHandle_,  &canDataToSend, timeOutMicroSeconds);

    if( retVal != 0){
        // The send data failed!
        stringstream ss;
        ss << "LINUX_CAN_Write_Timeout failed.  " << errorInfo(retVal);
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }

}


//***********************************************************************
// Setup the connection to the CAN card, default baud rates etc.
void 
CanPeakRmpIo::enable(int debugLevel){

    debugLevel_ = debugLevel;
        
    if(debugLevel_ > 0){
        std::cout << "TRACE(peakcandriver.cpp): enable()" << endl;
    }

    // Get the version
    cout << __func__ << ": version: " << versionString(portHandle_) << endl;

    // Force the default initialisation state of the CAN card. This should happen anyway
    // But let's be explicit about it. We are set to 500K baud and extended can message type...
    // Note in reality we are only ever dealing with standard not extended message types, but the
    // writes barf if we set ST message type here
    DWORD retVal = CAN_Init(portHandle_, CAN_BAUD_500K, CAN_INIT_TYPE_EX );
    
    if( retVal != 0 ){
        stringstream ss;
        ss << endl << " --> Attempted initialisation of the CAN card failed: " << errorInfo(retVal) << endl;
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );      
    }

 
    // Potentially we could call the CAN_MsgFilter() from libpcan here to filter out some
    // of the un-needed packets (ie heartbeat messages) at a low level. However I haven't
    // been able to get this to work!
    isEnabled_ = true; 

}



//************************************************************************
// This function is just a place holder, and does very little 
void 
CanPeakRmpIo::disable(void)
{
//     DWORD retVal=0;
//     if(debugLevel_ > 0)
//     {
//         retVal = CAN_Status(portHandle_);
//         cout << "TRACE(peakcandriver.cpp): disable() CAN Status:- " <<
//            peakStatusToString(retVal) << endl;
//     }
    
    isEnabled_ = false;
}
