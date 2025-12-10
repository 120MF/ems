find_package(miniaudio QUIET)

if (NOT miniaudio_FOUND)
    FetchContent_Declare(miniaudio
            GIT_REPOSITORY https://gitee.com/mirrors/miniaudio
            GIT_TAG master
            GIT_SHALLOW true
    )
    FetchContent_MakeAvailable(miniaudio)
endif ()