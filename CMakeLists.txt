cmake_minimum_required(VERSION 3.15.4)

project(xion_client)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS -pthread)

set(CLIENT_SOURCES
    src/Client.cpp
    src/Command.cpp
    src/commands/NameCommand.cpp
    src/commands/QuitCommand.cpp
    src/commands/WhisperCommand.cpp
)

add_executable(client ${CLIENT_SOURCES})

target_include_directories(client
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)
