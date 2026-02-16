cmake_minimum_required(VERSION 3.18...3.27) # FetchContent is available in 3.11+
set(CMAKE_POLICY_VERSION_MINIMUM 3.18...3.27) # Makes sndfile happy

# https://www.programmersought.com/article/68945253024/

project(lillaspel VERSION 0.0.31 LANGUAGES CXX)
message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

# C++ standard settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(ENABLE_SHADERS "Build HLSL shaders" ON)
option(ENABLE_IMGUI_DEMO "Include ImGui demo source" OFF)
option(ENABLE_SCCACHE "Use sccache compiler launcher" ON)
option(ENABLE_DEPENDENCY_TOOLS "Build third-party tests/examples/tools" OFF)

# Disable third-party tests/examples by default
if(NOT ENABLE_DEPENDENCY_TOOLS)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)

  set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
  set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)

  set(LIBSNDFILE_TESTS OFF CACHE BOOL "" FORCE)
  set(LIBSNDFILE_EXAMPLES OFF CACHE BOOL "" FORCE)

  set(FASTGLTF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
endif()

# Enable sccache if available
if(ENABLE_SCCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    message(STATUS "sccache enabled: ${SCCACHE_PROGRAM}")
  else()
    message(STATUS "sccache not found, disabled")
  endif()
endif()

# Output directories for build artifacts
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include(FetchContent)

# Avoid auto-updating dependencies every configure
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Fetch nlohmann::json single-header into the build include dir
set(JSON_SINGLE_INCLUDE "${CMAKE_BINARY_DIR}/include/nlohmann/json.hpp")
if(NOT EXISTS "${JSON_SINGLE_INCLUDE}")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include")
  message(STATUS "Downloading nlohmann/json single header to ${JSON_SINGLE_INCLUDE}")
  file(DOWNLOAD
    "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"
    "${JSON_SINGLE_INCLUDE}"
    TLS_VERIFY ON
    SHOW_PROGRESS
  )
else()
  message(STATUS "Using existing nlohmann/json header at ${JSON_SINGLE_INCLUDE}")
endif()

# Dependencies (FetchContent)
FetchContent_Declare(
  DirectXTK
  GIT_REPOSITORY https://github.com/microsoft/DirectXTK.git
  GIT_TAG oct2025
)
FetchContent_MakeAvailable(DirectXTK)

FetchContent_Declare(
  OpenAL
  GIT_REPOSITORY https://github.com/kcat/openal-soft
  GIT_TAG 1.25.1
)
FetchContent_MakeAvailable(OpenAL)

FetchContent_Declare(
  sndfile
  GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
  GIT_TAG 1.2.2
)
FetchContent_MakeAvailable(sndfile)

FetchContent_Declare(
  fastgltf
  GIT_REPOSITORY https://github.com/spnda/fastgltf/
  GIT_TAG v0.9.0
)
FetchContent_MakeAvailable(fastgltf)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.82
)
FetchContent_MakeAvailable(imgui)

# Project sources
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/headers/*.h" "${CMAKE_SOURCE_DIR}/headers/*.hpp")

add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Target-scoped Windows Unicode defines
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# Unity build (optional)
option(ENABLE_UNITY_BUILD "Enable Unity build for faster compiles" ON)
if(ENABLE_UNITY_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        UNITY_BUILD ON
    )
endif()

# Project include paths
target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_SOURCE_DIR}/headers"
  "${CMAKE_BINARY_DIR}/include"
)

# ImGui sources and include paths
if(imgui_SOURCE_DIR)
  target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
  )
  if(ENABLE_IMGUI_DEMO)
    target_sources(${PROJECT_NAME} PRIVATE
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
    )
  endif()
  target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
  )
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  d3d11
  DirectXTK
  OpenAL::OpenAL
  sndfile
  fastgltf
  xinput
)

# Shader compilation (fxc)
if(ENABLE_SHADERS)
  set(SHADER_PATH "./src/shaders")
  set(PS_SHADER "${SHADER_PATH}/pixelshaders/PSTest.hlsl")
  set(PS_SHADER_UNLIT "${SHADER_PATH}/pixelshaders/psUnlit.hlsl")
  set(PS_SHADER_LIT "${SHADER_PATH}/pixelshaders/psLit.hlsl")
  set(PS_SKYBOX "${SHADER_PATH}/pixelshaders/psSkybox.hlsl")
  set(VS_SHADER "${SHADER_PATH}/vertexshaders/VSTest.hlsl")
  set(VS_SKYBOX "${SHADER_PATH}/vertexshaders/vsSkybox.hlsl")
  set(HLSL_SHADER_FILES ${PS_SHADER} ${PS_SHADER_UNLIT} ${PS_SHADER_LIT} ${PS_SKYBOX} ${VS_SHADER} ${VS_SKYBOX})

  set_source_files_properties(${VS_SHADER} PROPERTIES ShaderType "vs")
  set_source_files_properties(${VS_SKYBOX} PROPERTIES ShaderType "vs")
  set_source_files_properties(${PS_SHADER} PROPERTIES ShaderType "ps")
  set_source_files_properties(${PS_SHADER_UNLIT} PROPERTIES ShaderType "ps")
  set_source_files_properties(${PS_SHADER_LIT} PROPERTIES ShaderType "ps")
  set_source_files_properties(${PS_SKYBOX} PROPERTIES ShaderType "ps")
  set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES ShaderModel "5_0")

  set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
  file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")

  foreach(FILE ${HLSL_SHADER_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)
    set(CSO_FILE "${SHADER_OUTPUT_DIR}/${FILE_WE}.cso")
    set(PDB_FILE "${SHADER_OUTPUT_DIR}/${FILE_WE}.pdb")
    list(APPEND CSO_SHADER_FILES ${CSO_FILE})

    get_source_file_property(shadertype ${FILE} ShaderType)
    get_source_file_property(shadermodel ${FILE} ShaderModel)

    add_custom_command(
      OUTPUT ${CSO_FILE} ${PDB_FILE}
      COMMAND fxc.exe /nologo /Emain /T${shadertype}_${shadermodel} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi
              /Fo ${CSO_FILE} /Fd ${PDB_FILE} ${FILE}
      DEPENDS ${FILE}
      COMMENT "HLSL ${FILE}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
  endforeach()

  add_custom_target(Shaders DEPENDS ${CSO_SHADER_FILES})
  add_dependencies(${PROJECT_NAME} Shaders)

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CSO_SHADER_FILES} $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS)
endif()

# Prevent PDB collisions with MSVC + Ninja
if(MSVC)
  set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_DEBUG_INFORMATION_FORMAT Embedded)
endif()

message(STATUS "Configured ${PROJECT_NAME} with ${CMAKE_CXX_STANDARD}-standard and ${PROJECT_SOURCES} source files")
