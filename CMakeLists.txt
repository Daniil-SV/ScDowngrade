cmake_minimum_required(VERSION 3.22)
include(FetchContent)

# Yeah, avx2 would be much faster but its may cause many problems, so use sse here (for now)
set(WK_PREFERRED_CPU_FEATURES SSE41)
FetchContent_Declare(
    SupercellFlash
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellFlash.git
    GIT_TAG main
)
FetchContent_MakeAvailable(SupercellFlash)

include(WkArgsparse)

set(TARGET ScDowngrade)
project(${TARGET})

set(SOURCE source/main.cpp)
add_executable(${TARGET} ${SOURCE})
wk_project_setup(${TARGET}) 
wk_include_argparse()

target_link_libraries(${TARGET} PUBLIC
    supercell::flash
)