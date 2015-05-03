
project(CXX)

cmake_minimum_required(VERSION 3.1 )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt4 4.4.3 REQUIRED QtGui QtXml)
add_executable(qvimage qvimage.cpp)
target_link_libraries(qvimage Qt4::QtGui Qt4::QtXml)

target_compile_options(qvimage PUBLIC -g -O0 )

install( TARGETS qvimage
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
  BUNDLE  DESTINATION bin
  )



