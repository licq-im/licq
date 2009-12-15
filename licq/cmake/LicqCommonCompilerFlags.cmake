include(TestCXXAcceptsFlag)

check_cxx_accepts_flag(-Wall CXX_ACCEPTS_WALL)
if (CXX_ACCEPTS_WALL)
  add_definitions(-Wall)
endif (CXX_ACCEPTS_WALL)

check_cxx_accepts_flag(-Wextra CXX_ACCEPTS_WEXTRA)
if (CXX_ACCEPTS_WEXTRA)
  add_definitions(-Wextra)
endif (CXX_ACCEPTS_WEXTRA)
