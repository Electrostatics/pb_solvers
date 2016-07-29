# Required for static linking and exported to PBSAM.cmake
set(PBSAM_LINKER_LIBS "")

# ---[ BLAS
if(NOT APPLE)
  set(BLAS "MKL" CACHE STRING "Selected BLAS library")
  set_property(CACHE BLAS PROPERTY STRINGS "Atlas;Open;MKL")

  if(BLAS STREQUAL "Atlas" OR BLAS STREQUAL "atlas")
    find_package(Atlas REQUIRED)
   #include_directories(SYSTEM ${Atlas_INCLUDE_DIR})
    include_directories(${Atlas_INCLUDE_DIR})
    list(APPEND PBSAM_LINKER_LIBS ${Atlas_LIBRARIES})
  elseif(BLAS STREQUAL "Open" OR BLAS STREQUAL "open")
    find_package(OpenBLAS REQUIRED)
    include_directories(${OpenBLAS_INCLUDE_DIR})
    list(APPEND PBSAM_LINKER_LIBS ${OpenBLAS_LIB})
  elseif(BLAS STREQUAL "MKL" OR BLAS STREQUAL "mkl")
    find_package(MKL REQUIRED)
    include_directories(${MKL_INCLUDE_DIR})
    list(APPEND PBSAM_LINKER_LIBS ${MKL_LIBRARIES})
    add_definitions(-D__MKL)
  endif()
  add_definitions(-D__LAU)
elseif(APPLE)
  find_package(vecLib REQUIRED)
  include_directories(SYSTEM ${vecLib_INCLUDE_DIR})
  list(APPEND PBSAM_LINKER_LIBS ${vecLib_LINKER_LIBS})
endif()