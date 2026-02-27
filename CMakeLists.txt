cmake_minimum_required(VERSION 3.21)

# 프로젝트 이름 설정
project(SisyphusRenderer VERSION 1.0)

# C++17 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# IntelliSense를 위한 compile_commands.json 생성
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 소스 파일 수집
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*.h"
    "src/*.hlsl"
    "src/*.hlsli"
)

# 실행 파일 생성
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# vcpkg 패키지 불러오기
find_package(imgui CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# 헤더 포함 경로 설정
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/System"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Utils"
)

# 5. 라이브러리 연결 (링커 설정)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    # 기본 시스템 라이브러리
    d3d11
    dxgi
    d3dcompiler
    dinput8
    dxguid
    # vcpkg로 설치한 라이브러리 타겟
    imgui::imgui
    Microsoft::DirectXTK
    spdlog::spdlog
    assimp::assimp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/Pch.h")

# HLSL 컴파일 설정 (Visual Studio용)
foreach(FILE ${SOURCES})
    if(FILE MATCHES "\\.hlsl$")
        set_source_files_properties(${FILE} PROPERTIES VS_TOOL_OVERRIDE "FXCompile")
        set_source_files_properties(${FILE} PROPERTIES VS_SHADER_MODEL "5.0")
        
        if(FILE MATCHES "VS" OR FILE MATCHES "Vertex")
            set_source_files_properties(${FILE} PROPERTIES VS_SHADER_TYPE "Vertex")
        elseif(FILE MATCHES "PS" OR FILE MATCHES "Pixel")
            set_source_files_properties(${FILE} PROPERTIES VS_SHADER_TYPE "Pixel")
        endif()
    elseif(FILE MATCHES "\\.hlsli$")
        set_source_files_properties(${FILE} PROPERTIES VS_TOOL_OVERRIDE "None")
    endif()
endforeach()

set(HLSL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/HLSL")
set(HLSL_OUTPUT_DIR "${CMAKE_BINARY_DIR}/src/Graphics/HLSL")
set(ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assets")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HLSL_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${HLSL_SOURCE_DIR}" "${HLSL_OUTPUT_DIR}"
    
    # 에셋 폴더 복사
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSETS_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_SOURCE_DIR}" "${ASSETS_OUTPUT_DIR}"
    
    COMMENT "Copying HLSL shaders and Assets to build directory..."
)
