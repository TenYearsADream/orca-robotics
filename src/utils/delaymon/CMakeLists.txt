ORCA_SET_COMPONENT_NAME( DelayMon )
set( COMP_INTERFACE_FLAG "orcaice::NoStandardInterfaces" )

set( build TRUE )
GBX_REQUIRE_OPTION( build EXE ${APP_NAME} ON )

set( dep_libs OrcaIfaceImpl )
GBX_REQUIRE_LIBS( build EXE ${APP_NAME} ${dep_libs} )

if( build )

    include( ${ORCA_CMAKE_DIR}/UseComponentRules.cmake )

    file( GLOB srcs *.cpp )
    
    # build stand-alone component  
    ORCA_WRAP_MAINTHREAD_INTO_APP( ${srcs} )
    target_link_libraries( ${APP_NAME} ${dep_libs} )
    
    ORCA_GENERATE_CONFIG_FILE( ${APP_NAME}.def )

endif( build )
