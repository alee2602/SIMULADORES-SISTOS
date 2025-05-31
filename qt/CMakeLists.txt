cmake_minimum_required(VERSION 3.16)
project(ProcessSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Source files
set(SOURCES
    main.cpp
    synchronizer.cpp
    loader.cpp
    scheduler.cpp
)

# Header files
set(HEADERS
    utils.h
    loader.h
    scheduler.h
    synchronizer.h
)

# Create executable
add_executable(ProcessSimulator ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(ProcessSimulator Qt6::Core Qt6::Widgets)

# Set output directory
set_target_properties(ProcessSimulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy data files to build directory
file(GLOB DATA_FILES "data/*.txt")
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/data/)

# Installation
install(TARGETS ProcessSimulator DESTINATION bin)
install(FILES ${DATA_FILES} DESTINATION data/)