# Locate Player install directory

# This module defines
# PLAYER_HOME where to find include, lib, bin, etc.
# PLAYER_FOUND, If false, don't try to use Ice.

INCLUDE (${ORCA_CMAKE_DIR}/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE )

#    MESSAGE( "Using pkgconfig" )
    
    # Find all the librtk stuff with pkg-config
    PKGCONFIG( "playerc++ >= 2.0" PLAYER_FOUND PLAYER_INCLUDE_DIRS PLAYER_DEFINES PLAYER_LINK_DIRS PLAYER_LIBS )

#     IF( PLAYER_FOUND )
#        MESSAGE("   Includes in: ${PLAYER_INCLUDE_DIRS}")
#        MESSAGE("   Libraries in: ${PLAYER_LINK_DIRS}")
#        MESSAGE("   Libraries: ${PLAYER_LIBS}")
#        MESSAGE("   Defines: ${PLAYER_DEFINES}")
#     ENDIF ( PLAYER_FOUND )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE )

    # Can't find pkg-config -- have to search manually

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE )
