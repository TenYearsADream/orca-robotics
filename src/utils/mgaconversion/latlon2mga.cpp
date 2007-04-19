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
#include <iomanip>
#include <sstream>

#include <orcagpsutil/latlon2mga.h>

using namespace std;
using namespace orcagpsutil;

#define USAGE_ARGS "<latitude> <longitude>"

int main( int argc, char **argv )
{
    if (argc<3) { 
        cout << "Usage: " << argv[0] << USAGE_ARGS << endl;
        exit(1);
    }
    
    stringstream ss;
    double latitude, longitude;
    ss << argv[1] << " " << argv[2];
    ss >> latitude >> longitude;
    
    double easting, northing;
    int zone;
    LatLon2MGA( latitude, longitude, northing, easting, zone );
    
    cout    << setiosflags(ios::fixed) << setprecision(16)
            << "Latitude:\t" << latitude << endl
            << "Longitude:\t" << longitude << endl
            << "Easting:\t" << easting << endl
            << "Northing:\t" << northing << endl;
    
    return 0;
}
