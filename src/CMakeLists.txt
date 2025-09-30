add_executable(rename main.cpp)

install(TARGETS rename RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
