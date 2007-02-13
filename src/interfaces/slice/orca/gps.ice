/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#ifndef ORCA2_GPS_INTERFACE_ICE
#define ORCA2_GPS_INTERFACE_ICE

#include <orca/orca.ice>
#include <orca/bros1.ice>

module orca
{
/*!
    @ingroup interfaces
    @defgroup orca_interface_gps Gps
    @brief Access to GPS devices.
    @{
*/


//! Gps config structure
struct GpsDescription
{
    //! Time (according to the computer clock) when data was measured.
    Time timeStamp;

    //! Offset of the gps antenna with respect to the robot,
    //! in the robot local coordinate system.
    Frame3d offset; 

    //! Dimensions of the gps antenna.
    Size3d size;
};

//! Gps time structure
struct GpsTimeData
{
    //! Time (according to the computer clock) when data was measured.
    Time timeStamp;
    //! UTC time (according to GPS device), reference is Greenwich.
    TimeOfDay utcTime;
    //! UTC date (according to GPS device)
    Date utcDate;
};

//! Gps Map Grid Australia (MGA) data structure
struct GpsMapGridData
{
    //! Time (according to the computer clock) when data was measured.
    Time timeStamp;
    //! UTC time (according to GPS device), reference is Greenwich.
    TimeOfDay utcTime;

    //! Our current MGA zone
    int zone;
    //! Northing (metres)
    double northing;
    //! Easting (metres)
    double easting;
    //! Altitude (metres above ellipsoid)
    double altitude;
    
    //! Heading/track/course (degrees)
    double heading; 
    //! Horizontal velocity (metres/second)
    double speed;
    //! Vertical velocity (metres/second)
    double climbRate;
    
    //! Position type (Bad (0), Ugly (1), Good (2))
    int positionType;
};


//! Gps data structure
struct GpsData
{
    //! Time (according to the computer clock) when data was measured.
    Time timeStamp;
    //! UTC time (according to GPS device), reference is Greenwich.
    TimeOfDay utcTime;
 
    //! Latitude (degrees)
    double latitude;
    //! Longitude (degrees)
    double longitude;
    //! Altitude (metres above ellipsoid)
    double altitude;
    
    //! Heading/track/course (degrees)
    double heading; 
    //! Horizontal velocity (metres/second)
    double speed;
    //! Vertical velocity (metres/second)
    double climbRate;
    
    //! Number of satellites
    int satellites;
    //! Position type (Bad (0), Ugly (1), Good (2))
    int positionType;
    //! Geoidal Separation (metres)
    double geoidalSeparation;    
};


/*!
 *
 * Data consumer interface
 *
 */
interface GpsConsumer
{
    void setData( GpsData obj );
};

/*!
 * Gps Map information consumer interface
 */
interface GpsMapGridConsumer
{
    void setData( GpsMapGridData obj );
};

/*!
 * Gps Time information consumer interface
 */
interface GpsTimeConsumer
{
    void setData( GpsTimeData obj );
};

/*!
    @brief Access to GPS data.
*/
interface Gps
{
    //! Returns the latest data
    nonmutating GpsData getData()
            throws HardwareFailedException;

    //! Return the latest timestamp information
    nonmutating GpsTimeData getTimeData()
            throws HardwareFailedException;

    //! Return the latest map information
    nonmutating GpsMapGridData getMapGridData()
            throws HardwareFailedException;

    //! Return the gps description
    nonmutating GpsDescription getDescription();

    /*!
     * Mimics IceStorm's subscribe() but without QoS, for now. The
     * implementation may choose to implement the data push internally
     * or use IceStorm. This choice is transparent to the subscriber.
     *
     * @param subscriber The subscriber's proxy.
     *
     * @see unsubscribe
     */
    void subscribe( GpsConsumer* subscriber )
            throws SubscriptionFailedException;

    void subscribeForMapGrid( GpsMapGridConsumer* subscriber )
            throws SubscriptionFailedException;

    void subscribeForTime( GpsTimeConsumer* subscriber )
            throws SubscriptionFailedException;

    // for reference, this is what IceStorm's subscribe function looks like.
    //void subscribe(QoS theQoS, Object* subscriber);

    /*!
     * Unsubscribe the given [subscriber].
     *
     * @param subscriber The proxy of an existing subscriber.
     *
     * @see subscribe
     */
    idempotent void unsubscribe( GpsConsumer* subscriber );
    
    idempotent void unsubscribeForMapGrid( GpsMapGridConsumer* subscriber );
    
    idempotent void unsubscribeForTime( GpsTimeConsumer* subscriber );
};


//!  //@}
}; // module

#endif
