cmake_minimum_required(VERSION 3.22)
include(FetchContent)

set(TARGET ScDowngrade)

project(${TARGET})

set(SOURCE Source/main.cpp)

FetchContent_Declare(
    SupercellFlash
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellFlash.git
    GIT_TAG main
)
FetchContent_MakeAvailable(SupercellFlash)

add_executable(${TARGET} ${SOURCE})
wk_project_setup(${TARGET}) 

target_link_libraries(${TARGET} PUBLIC
    supercell::flash
)