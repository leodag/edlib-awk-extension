cmake_minimum_required(VERSION 3.10)
project(edlib-awk VERSION 1.0.0)

add_library(edlib-awk SHARED edlib-awk.c edlib.cpp)

# Filename must be edlib.so so it can be loaded as @load "edlib"
set_target_properties(edlib-awk PROPERTIES PREFIX "")
set_target_properties(edlib-awk PROPERTIES OUTPUT_NAME "edlib")
set_target_properties(edlib-awk PROPERTIES SUFFIX ".so")

# Usually gawk will not scan for extensions in /usr/local/lib/gawk,
# so we want to change the default prefix to /usr
include(GNUInstallDirs)
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Installation prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

install(TARGETS edlib-awk DESTINATION ${CMAKE_INSTALL_LIBDIR}/gawk)
install(PROGRAMS edlib.awk DESTINATION ${CMAKE_INSTALL_DATADIR}/awk)
