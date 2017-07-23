# to be set before calling find_package:
#
#  PCM_PREFIX         Set this variable to the root installation of
#                     libpcm if the module has problems finding the
#                     proper installation path.
#
# Variables defined by this module:
#
#  PCM_FOUND              System has PCM libraries and headers
#  PCM_LIBRARIES          The PCM library
#  PCM_INCLUDE_DIRS       The location of PCM headers

find_path(PCM_PREFIX
    NAMES cpucounters.h
	HINTS ${PCM_PREFIX}/include ${PCM_PREFIX}
)

find_library(PCM_LIBRARY
    NAMES libPCM.a PCM 
    HINTS ${PCM_PREFIX}/lib ${PCM_PREFIX}/lib64 ${PCM_PREFIX}
)

set(PCM_LIBRARIES ${PCM_LIBRARY})

find_path(PCM_INCLUDE_DIRS
    NAMES cpucounters.h
    HINTS ${PCM_PREFIX}/include  ${PCM_PREFIX}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PCM DEFAULT_MSG
    PCM_LIBRARIES
    PCM_INCLUDE_DIRS
)

mark_as_advanced(
    PCM_PREFIX_DIRS
    PCM_LIBRARIES
    PCM_INCLUDE_DIRS
)