cmake_minimum_required(VERSION 3.10)
project(snowing)

set(CMAKE_CXX_STANDARD 17)

# 寻找OpenGL、GLEW和GLFW3包
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# 设置包含目录
include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 添加可执行文件
add_executable(snowing src/main.cpp)

# 链接库
target_link_libraries(snowing
    OpenGL::GL
    GLEW::GLEW
    glfw
) 

# 将着色器文件和纹理复制到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})