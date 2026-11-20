cmake_minimum_required(VERSION 3.20)

project(OpenGLUniversityProject VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(opengl_project
    opengl-cube/main.cpp
    opengl-cube/scene.cpp
    opengl-cube/scene_input.cpp
    opengl-cube/letters.cpp
    opengl-cube/mesh.cpp
    opengl-cube/rubiks_cube.cpp
    opengl-cube/shader.cpp
    opengl-cube/texture.cpp
    opengl-cube/camera.cpp
)
target_compile_features(opengl_project PRIVATE cxx_std_17)
set_target_properties(opengl_project PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(opengl_project SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/includes")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
    target_compile_options(opengl_project PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>)
endif()

if(EMSCRIPTEN)
    target_sources(opengl_project PRIVATE
        opengl-cube/landscape.cpp
        opengl-cube/scene_walk.cpp
        opengl-cube/walker.cpp
    )
    set(web_shell "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html")
    set_target_properties(opengl_project PROPERTIES SUFFIX ".html")
    target_compile_definitions(opengl_project PRIVATE PROJECT_ASSET_DIR="/assets")
    target_compile_options(opengl_project PRIVATE -fexceptions)
    target_link_options(opengl_project PRIVATE
        -fexceptions
        -sUSE_GLFW=3
        -sMIN_WEBGL_VERSION=2
        -sMAX_WEBGL_VERSION=2
        -sALLOW_MEMORY_GROWTH=1
        "SHELL:--shell-file \"${web_shell}\""
        "SHELL:--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets\""
    )
    file(GLOB_RECURSE web_assets CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
    set_property(TARGET opengl_project APPEND PROPERTY LINK_DEPENDS "${web_shell}" "${web_assets}")
else()
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 QUIET)
    if(TARGET glfw)
        set(glfw_target glfw)
    elseif(MSVC AND CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
        add_library(bundled_glfw STATIC IMPORTED)
        set_target_properties(bundled_glfw PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib"
            INTERFACE_LINK_LIBRARIES "gdi32;user32;shell32"
            # The bundled archive requests the debug CRT. Use the project's CRT in release builds.
            INTERFACE_LINK_OPTIONS "$<$<NOT:$<CONFIG:Debug>>:/NODEFAULTLIB:MSVCRTD>"
        )
        set(glfw_target bundled_glfw)
    else()
        message(FATAL_ERROR "Install GLFW 3.3 or newer and set glfw3_DIR if needed.")
    endif()
    target_sources(opengl_project PRIVATE includes/glad/glad.c)
    target_compile_definitions(opengl_project PRIVATE PROJECT_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
    target_link_libraries(opengl_project PRIVATE ${glfw_target} OpenGL::GL ${CMAKE_DL_LIBS})

    include(CTest)
    if(BUILD_TESTING)
        add_executable(rubiks_cube_test tests/rubiks_cube_test.cpp opengl-cube/rubiks_cube.cpp)
        target_compile_features(rubiks_cube_test PRIVATE cxx_std_17)
        target_include_directories(rubiks_cube_test PRIVATE opengl-cube)
        target_include_directories(rubiks_cube_test SYSTEM PRIVATE includes)
        add_test(NAME rubiks_cube_turns COMMAND rubiks_cube_test)

        add_executable(walker_test tests/walker_test.cpp opengl-cube/walker.cpp)
        target_compile_features(walker_test PRIVATE cxx_std_17)
        target_include_directories(walker_test PRIVATE opengl-cube)
        target_include_directories(walker_test SYSTEM PRIVATE includes)
        add_test(NAME walker_collision COMMAND walker_test)
    endif()

    add_custom_target(run
        COMMAND $<TARGET_FILE:opengl_project>
        DEPENDS opengl_project
        USES_TERMINAL
    )
endif()
