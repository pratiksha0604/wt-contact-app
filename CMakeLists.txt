cmake_minimum_required(VERSION 3.16)
project(wt_contact_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Wt REQUIRED)

add_executable(wt_app src/main.cpp)
target_link_libraries(wt_app PRIVATE Wt::Wt Wt::HTTP)
