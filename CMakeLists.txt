cmake_minimum_required(VERSION 3.20)

project(OpenGLUniversityProject
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(opengl_project
    "opengl-cube/main.cpp"
    "includes/glad/glad.c"
)

target_include_directories(opengl_project
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/includes"
        "${CMAKE_CURRENT_SOURCE_DIR}/opengl-cube"
)

target_compile_definitions(opengl_project
    PRIVATE
        PROJECT_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_link_libraries(opengl_project
    PRIVATE
        glfw
        OpenGL::GL
        ${CMAKE_DL_LIBS}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(opengl_project
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

add_custom_target(run
    COMMAND $<TARGET_FILE:opengl_project>
    DEPENDS opengl_project
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Running opengl_project"
)