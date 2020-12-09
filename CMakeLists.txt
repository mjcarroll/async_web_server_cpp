cmake_minimum_required(VERSION 2.8.3)
project(async_web_server_cpp)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic) 
endif()

## System dependencies are found with CMake's conventions
find_package(OpenSSL)

###########
## Build ##
###########

## Specify additional locations of header files
include_directories(
  include
  ${OPENSSL_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} 
  src/http_request.cpp
  src/http_request_parser.cpp
  src/http_connection.cpp 
  src/http_reply.cpp 
  src/http_request_handler.cpp
  src/http_server.cpp
  src/websocket_connection.cpp 
  src/websocket_request_handler.cpp
  src/websocket_message.cpp 
  )
  

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
  "-DASIO_STANDALONE=1"
)

target_link_libraries(${PROJECT_NAME}
  ${OPENSSL_LIBRARIES}
  pthread
)

add_subdirectory(test)
