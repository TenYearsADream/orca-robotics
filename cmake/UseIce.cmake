#
# Links to libIce only.
# see also: UseIceBasics.cmake
#

INCLUDE( ${ORCA_CMAKE_DIR}/UseZeroC.cmake )

IF ( NOT OS_WIN )

    LINK_LIBRARIES( Ice )

ELSE ( NOT OS_WIN )

    # windows... have to link to different libs depending on build type
    LINK_LIBRARIES( optimized Ice       debug Iced)

ENDIF ( NOT OS_WIN )