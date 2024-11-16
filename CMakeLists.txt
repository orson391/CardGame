cmake_minimum_required(VERSION 3.10)

project(CardGame VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL2_INCLUDE_DIR "C:/vcpkg/installed/x64-mingw-dynamic/include")

include_directories(${SDL2_INCLUDE_DIR})

add_executable(CardGame src/main.cpp src/Game.cpp)

# Link libraries depending on configuration
target_link_libraries(CardGame PRIVATE
    $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/lib/libSDL2d.dll.a>
    $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/lib/libSDL2.dll.a>
    $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/lib/libSDL2maind.a>
    $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/lib/libSDL2main.a>
    $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/lib/libSDL2_imaged.dll.a>
    $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/lib/libSDL2_image.dll.a>
    $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/lib/libSDL2_netd.dll.a>
    $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/lib/libSDL2_net.dll.a>
    $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/lib/libSDL2_ttfd.dll.a>
    $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/lib/libSDL2_ttf.dll.a>
)

#adding the required ddls to the directory
add_custom_command(TARGET CardGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/bin/SDL2d.dll>
        $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/bin/SDL2.dll>
        $<TARGET_FILE_DIR:CardGame>
    COMMAND ${CMAKE_COMMAND} -E copy
        $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/bin/SDL2_imaged.dll>
        $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/bin/SDL2_image.dll>
        $<TARGET_FILE_DIR:CardGame>
    COMMAND ${CMAKE_COMMAND} -E copy
        $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/bin/SDL2_netd.dll>
        $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/bin/SDL2_net.dll>
        $<TARGET_FILE_DIR:CardGame>
    COMMAND ${CMAKE_COMMAND} -E copy
        $<$<CONFIG:Debug>:C:/vcpkg/installed/x64-mingw-dynamic/debug/bin/SDL2_ttfd.dll>
        $<$<CONFIG:Release>:C:/vcpkg/installed/x64-mingw-dynamic/bin/SDL2_ttfd.dll>
        $<TARGET_FILE_DIR:CardGame>   
)
