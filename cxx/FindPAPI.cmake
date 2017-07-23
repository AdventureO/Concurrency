# to be set before calling find_package:
#
#  PAPI_PREFIX         Set this variable to the root installation of
#                      libpapi if the module has problems finding the
#                      proper installation path.
#
# Variables defined by this module:
#
#  PAPI_FOUND              System has PAPI libraries and headers
#  PAPI_LIBRARIES          The PAPI library
#  PAPI_INCLUDE_DIRS       The location of PAPI headers
#
# https://stackoverflow.com/questions/28711492/include-these-variables-into-cmake

find_path(PAPI_PREFIX
    NAMES include/papi.h
)

find_library(PAPI_LIBRARY
    NAMES libpapi.a papi 
    HINTS ${PAPI_PREFIX}/lib ${PAPI_PREFIX}/lib64
)

find_library(PFM_LIBRARY
    NAMES libpfm.a  pfm
    HINTS ${PAPI_PREFIX}/lib ${PAPI_PREFIX}/lib64
)

set(PAPI_LIBRARIES ${PAPI_LIBRARY} ${PFM_LIBRARY})

find_path(PAPI_INCLUDE_DIRS
    NAMES papi.h
    HINTS ${PAPI_PREFIX}/include 
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAPI DEFAULT_MSG
    PAPI_LIBRARIES
    PAPI_INCLUDE_DIRS
)

mark_as_advanced(
    PAPI_PREFIX_DIRS
    PAPI_LIBRARIES
    PAPI_INCLUDE_DIRS
)