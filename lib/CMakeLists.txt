cmake_minimum_required(VERSION 3.21)
project(libac_dat
        LANGUAGES C
        DESCRIPTION "Aho-Corasick with Double Array Trie"
        HOMEPAGE_URL https://github.com/Izolex/ac_dat)

set(HEADERS
    ../include/ac.h
    ../include/dat.h
    ../include/file.h
    ../include/list.h
    ../include/needle.h
    ../include/print.h
    ../include/socket.h
    ../include/socket_ac.h
    ../include/tail.h
    ../include/thread.h
    ../include/user_data.h
)

set(ARCHIVE_OUTPUT_NAME ${PROJECT_NAME})
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/../src SOURCES)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::EVENT)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ac_dat PUBLIC_HEADER "${HEADERS}")

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../include)


set(PC_REQUIREMENTS libevent)
configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)


include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        EXPORT  ${TARGETS_EXPORT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ac_dat)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
