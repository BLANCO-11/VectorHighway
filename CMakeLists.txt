cmake_minimum_required(VERSION 3.10)
project(UAVSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow older subprojects (like nlohmann/json) to configure without errors on newer CMake versions
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

# Fetch Asio (Header-only dependency for WebSocket++)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)

# Fetch WebSocket++
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(websocketpp)

# Fetch nlohmann/json for parsing frontend commands
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

include_directories(include)
include_directories(${asio_SOURCE_DIR}/asio/include)
include_directories(${websocketpp_SOURCE_DIR})
include_directories(${json_SOURCE_DIR}/single_include)

# Main simulation executable
add_executable(uav_sim 
    src/exec.cpp 
    src/websockets.cpp 
)

target_compile_definitions(uav_sim PRIVATE 
    ASIO_STANDALONE 
    _WEBSOCKETPP_CPP11_STL_
    _WEBSOCKETPP_DISABLE_BOOST_THREAD_SUPPORT
    _WEBSOCKETPP_DISABLE_BOOST_ASIO
    _WEBSOCKETPP_CPP11_THREAD_
)

find_package(Threads REQUIRED)
target_link_libraries(uav_sim PRIVATE Threads::Threads)

# Ensure Windows Sockets are linked for MinGW and the Windows version is set
if(WIN32)
    target_compile_definitions(uav_sim PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(uav_sim PRIVATE ws2_32 iphlpapi mswsock)
endif()

# Separate executable for tests
# add_executable(uav_tests src/tests.cpp) # Commented out until tests.cpp has a main() function



