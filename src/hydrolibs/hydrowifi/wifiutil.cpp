/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2009 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#include <cstring>
#include <stdio.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/types.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>

#include <iostream>
#include <sstream>

#include "wifiutil.h"

// for wireless.h versions older than V19
#ifndef IW_QUAL_DBM
    #define IW_QUAL_DBM 0x08
#endif

using namespace std;

namespace wifiutil {

void getInterfaceNames( vector<string> &interfaceNames )
{
    FILE *wifiInfoFile;
    char buf[128];
    
    int numTries=0;
    while(1)
    {
        // get the wireless device from /proc/net/wireless
        if ((wifiInfoFile = fopen("/proc/net/wireless", "r")) == NULL) 
        {
            numTries++;
            cout << "Warning(getInterfaceNames): Couldn't open for reading. Tried " << numTries << " times." << endl;
            if (numTries==5) throw Exception( "Error:\nCouldn't open /proc/net/wireless for reading.\n" );
        } else {
            break;
        }
    }
        
    // lets skip the header
    for (int i = 0; i < 3; i++) 
    {    
        if (fgets(buf, sizeof(buf),wifiInfoFile) == NULL) {
            fclose(wifiInfoFile);
            throw NoWirelessInterfaceException( "No wireless interface available\n" );
        }
    }
    
    int numEth;
    
    while (true)
    {
        char c;
        sscanf(buf, "  %cth%d:", &c, &numEth);
        stringstream ss; 
        ss << c << "th" << numEth;
        interfaceNames.push_back( ss.str() );
        // read the next line
        if (fgets(buf, sizeof(buf),wifiInfoFile) == NULL) break;
    }
    
    fclose( wifiInfoFile );
}


void readFromProc( vector<ProcData> &wifiData )
{  
    FILE *wifiInfoFile;
    ProcData data;
    char buf[128];
    
    int numTries=0;
    
    while(1)
    {
        // get the wireless device from /proc/net/wireless
        if ((wifiInfoFile = fopen("/proc/net/wireless", "r")) == NULL)
        {
            fclose(wifiInfoFile);
            numTries++;
            cout << "WARNING(readFromProc): Couldn't open for reading. Tried " << numTries << " times." << endl;
            if (numTries==5) throw Exception( "Error:\nCouldn't open /proc/net/wireless for reading.\n" );
        } else {
            break;
        }
    }  
        
    // lets skip the header
    for (int i = 0; i < 3; i++) 
    {    
        if (fgets(buf, sizeof(buf),wifiInfoFile) == NULL) {
            fclose(wifiInfoFile);
            throw NoWirelessInterfaceException( "No wireless interface available\n");
        }
    }
    
    while (true)
    {
        char *bp;
        int numEth;
        bp=buf;
        while(*bp&&isspace(*bp)) bp++;
        
        /* Get interface names */
        char c;
        sscanf(buf, "  %cth%d:", &c, &numEth);
        stringstream ss; 
        ss << c << "th" << numEth;
        data.interfaceName = ss.str();
    
        /* Skip ethX: */
        bp=strchr(bp,':');
        bp++;
        
        /* -- status -- */
        bp = strtok(bp, " ");
        sscanf(bp, "%X", &data.status);
        
        /* -- link quality -- */
        bp = strtok(NULL, " ");
        if(strchr(bp,'.') != NULL)
        sscanf(bp, "%d", &data.linkQuality);
        
        /* -- signal level -- */
        bp = strtok(NULL, " ");
        if(strchr(bp,'.') != NULL)
        sscanf(bp, "%d", &data.signalLevel);
          
        /* -- noise level -- */
        bp = strtok(NULL, " ");
        if(strchr(bp,'.') != NULL)
        sscanf(bp, "%d", &data.noiseLevel);
        
        /* -- discarded packets -- */
        bp = strtok(NULL, " ");
        sscanf(bp, "%d", &data.numInvalidNwid);
        bp = strtok(NULL, " ");
        sscanf(bp, "%d", &data.numInvalidCrypt);
        bp = strtok(NULL, " ");
        sscanf(bp, "%d", &data.numInvalidFrag);
        bp = strtok(NULL, " ");
        sscanf(bp, "%d", &data.numRetries);
        bp = strtok(NULL, " ");
        sscanf(bp, "%d", &data.numInvalidMisc);
        bp = strtok(NULL, " ");
        sscanf(bp, "%d", &data.numMissedBeacons);  
        
        wifiData.push_back( data );
        
        // read the next line
        if (fgets(buf, sizeof(buf),wifiInfoFile) == NULL) break;
    }
    
    fclose(wifiInfoFile);
}


string toString( vector<ProcData> &wifiData )
{    
    stringstream ss;
    
    for ( size_t i=0; i < wifiData.size(); i++ )
    {
        const ProcData &p = wifiData[i]; 
        ss << "Interface name:       " << p.interfaceName << endl;
        ss << "Status:               " << p.status << endl;
        ss << "Link quality:         " << p.linkQuality << endl;
        ss << "Signal level:         " << p.signalLevel << endl;
        ss << "Noise level:          " << p.noiseLevel << endl;
        ss << "Rx invalid nwid:      " << p.numInvalidNwid << endl;
        ss << "Rx invalid crypt:     " << p.numInvalidCrypt << endl;
        ss << "Rx invalid frag:      " << p.numInvalidFrag << endl;
        ss << "Tx excessive retries: " << p.numRetries << endl;   
        ss << "Invalid misc:         " << p.numInvalidMisc << endl;   
        ss << "Missed beacon:        " << p.numMissedBeacons << endl << endl;        
    }
    
    return ss.str();
}

string toString( vector<WirelessConfig> &wifiConfigs )
{    
    stringstream ss;
    
    for ( size_t i=0; i < wifiConfigs.size(); i++ )
    {
        const WirelessConfig &w = wifiConfigs[i]; 
        ss << "Interface name:     " << w.interfaceName << endl;
        ss << "Mode:               " << w.mode << endl;
        ss << "Bitrate:            " << w.bitrate << endl;
        ss << "Access point:       " << w.accessPoint << endl << endl;        
    }
    
    return ss.str();
}

string toString( vector<IoctlData> &wifiDataIoctl )
{   
    stringstream ss; 
    
    for ( size_t i=0; i < wifiDataIoctl.size(); i++ )
    {
        const IoctlData &d = wifiDataIoctl[i]; 
        ss << "Interface name:     " << d.interfaceName << endl;
        ss << "Throughput:         " << d.throughPut << endl;
        ss << "Link quality type:  " << d.linkType << endl;
        ss << "Link quality:       " << d.linkQuality << endl;
        ss << "Max link quality:   " << d.maxLinkQuality << endl;
        ss << "Signal level:       " << d.signalLevel << endl;
        ss << "Max signal level:   " << d.maxSignalLevel << endl;
        ss << "Noise level:        " << d.noiseLevel << endl;
        ss << "Max noise level:    " << d.maxNoiseLevel << endl << endl;    
    }
    
    return ss.str();    
}

char* convertMacHex(char *buf, unsigned char *data)
{
  struct ether_addr * p = (struct ether_addr *)data;
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
      p->ether_addr_octet[0], p->ether_addr_octet[1],
      p->ether_addr_octet[2], p->ether_addr_octet[3],
      p->ether_addr_octet[4], p->ether_addr_octet[5]);

  return buf;
}


OperationMode getMode( int socketFd, iwreq* iwRequest )
{
    OperationMode mode;
    
    if (ioctl(socketFd, SIOCGIWMODE, iwRequest) < 0)
        throw IoctlException( "Error:\nIoctl for mode failed.\n" );
        
    switch(iwRequest->u.mode) {
        case IW_MODE_AUTO:
            mode = OperationModeAuto;
            break;
        case IW_MODE_ADHOC:
            mode = OperationModeAdhoc;
            break;
        case IW_MODE_INFRA:
            mode = OperationModeInfrastructure;
            break;
        case IW_MODE_MASTER:
            mode = OperationModeMaster;
            break;
        case IW_MODE_REPEAT:
            mode = OperationModeRepeat;
            break;
        case IW_MODE_SECOND:
            mode = OperationModeSecondRepeater;
            break;
        default:
            mode = OperationModeUnknown;
    }    
    return mode;
}

string getAccessPointMac( int socketFd, iwreq* iwRequest )
{
    char accessPoint[32];
    
    if (ioctl(socketFd, SIOCGIWAP, iwRequest) < 0)
        throw IoctlException( "Error:\nIoctl for access point failed.\n" );
    
    struct sockaddr sa;
    memcpy(&sa, &(iwRequest->u.ap_addr), sizeof(sa));
    convertMacHex(accessPoint, (unsigned char *)sa.sa_data);
    
    return string( accessPoint );
}

int getBitrate( int socketFd, iwreq* iwRequest )
{
    if (ioctl(socketFd, SIOCGIWRATE, iwRequest) < 0) 
        throw IoctlException( "Error:\nIoctl for bitrate failed.\n" );
    
    int bitrate = iwRequest->u.bitrate.value;
    
    return bitrate;
}

void readConfig( vector<WirelessConfig> &wifiConfigs )
{
    // get a list of all current wireless interfaces
    vector<string> interfaceNames;
    getInterfaceNames(interfaceNames);  
    
    struct iwreq* iwRequest = new struct iwreq;
    
    // this is the socket to use for wireless info
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0)
    {
        cout << "Didn't get a socket!" << endl;
        close(socketFd);
        throw IoctlException( "Error:\nUnable to get a socket.\n" );
    }
    
    WirelessConfig cfg;
    
    for (size_t i=0; i<interfaceNames.size(); i++)
    {
        cfg.interfaceName = interfaceNames[i];
        
        char interface_name[32]; // name of wireless device
        strncpy(interface_name,interfaceNames[i].c_str(),sizeof(interface_name));
        
        // IMPORTANT: copy this name into the iwreq struct for use in ioctls
        strncpy(iwRequest->ifr_name, interface_name, IFNAMSIZ);
        
        // Request for mode 
        cfg.mode = getMode( socketFd, iwRequest );
        
        // Request for access point address
        cfg.accessPoint = getAccessPointMac( socketFd, iwRequest );
         
        // Request for bitrate
        cfg.bitrate = getBitrate( socketFd, iwRequest );
        
        wifiConfigs.push_back( cfg );
    }
    
    delete iwRequest;
    close(socketFd);
}

//
// Some of this code is adapted from wireless tools: iwlib.c
//
void readUsingIoctl( vector<IoctlData> &wifiDataIoctl )
{    
    // this is the socket to use for wireless info
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0)
    {
        cout << "Didn't get a socket!" << endl;
        close(socketFd);
        throw IoctlException( "Error:\nUnable to get a socket.\n" );
    }
    
    struct iwreq* iwRequest = new struct iwreq;
    struct iw_range* iwRange = new struct iw_range;
    struct iw_statistics* iwStats = new struct iw_statistics; 
    
    // get a list of all current wireless interfaces
    vector<string> interfaceNames;
    getInterfaceNames(interfaceNames);
    
    IoctlData data;
    
    for (size_t i=0; i<interfaceNames.size(); i++)
    {
        data.interfaceName = interfaceNames[i];
        
        char interface_name[32]; // name of wireless device
        strncpy(interface_name,interfaceNames[i].c_str(),sizeof(interface_name));
        
        // copy this name into the iwreq struct for use in ioctls
        strncpy(iwRequest->ifr_name, interface_name, IFNAMSIZ);
        
        // ========== Request for range ===============
        struct iw_range requestBuffer;
        iwRequest->u.data.pointer = (caddr_t)&requestBuffer;
        iwRequest->u.data.length = sizeof(struct iw_range);
        iwRequest->u.data.flags = 0;
        
        if(ioctl(socketFd, SIOCGIWRANGE, iwRequest) >= 0) 
        {
            memcpy((char *) iwRange, &requestBuffer, sizeof(struct iw_range));
        }
        else
        {
            cout << "Ioctl for range failed" << endl;
            close(socketFd);
            throw IoctlException( "Error:\nIoctl for range failed.\n" );    
        }
        
        // ============= Request for stats ================
        struct iw_quality *qual;
        iwRequest->u.data.pointer = (caddr_t) iwStats;
        iwRequest->u.data.length = sizeof(struct iw_statistics);
        iwRequest->u.data.flags = 1;  // clear updated flag
        if(ioctl(socketFd, SIOCGIWSTATS, iwRequest) >= 0) 
        {
            qual = &iwStats->qual;
            data.throughPut = iwRange->throughput;
            
            data.linkQuality = qual->qual;
            data.maxLinkQuality = iwRange->max_qual.qual;
            
            if((qual->updated & IW_QUAL_DBM) || (qual->level > iwRange->max_qual.level))
            {
                // dbm
                data.linkType = LinkQualityTypeDbm;
                data.signalLevel = qual->level - 0x100;
                data.maxSignalLevel = iwRange->max_qual.level;
                data.noiseLevel = qual->noise - 0x100;
                data.maxNoiseLevel = iwRange->max_qual.noise;
                
            }
            else
            {
                // relative
                data.linkType = LinkQualityTypeRelative;
                data.signalLevel = qual->level;
                data.maxSignalLevel = iwRange->max_qual.level;
                data.noiseLevel = qual->noise;
                data.maxNoiseLevel = iwRange->max_qual.noise;
            }
        }
        else
        {
            cout << "Ioctl for stats failed" << endl;
            close(socketFd);
            throw IoctlException( "Error:\nIoctl for stats failed.\n" );    
        }
    
        wifiDataIoctl.push_back( data );
    }

    delete iwRequest;
    delete iwRange;
    delete iwStats;

    close(socketFd);
}

}

