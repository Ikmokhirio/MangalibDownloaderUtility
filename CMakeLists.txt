cmake_minimum_required(VERSION 3.0)

project(MangalibDownloaderUtility VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${PROJECT_BINARY_DIR}/compile_commands.json
        ${PROJECT_SOURCE_DIR}
    )

if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
set(OPENSSL_PATH "${CMAKE_SOURCE_DIR}/openssl-3/x64")
else()
set(OPENSSL_PATH "${CMAKE_SOURCE_DIR}/openssl-3/x86")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)

# JSON
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("${PROJECT_SOURCE_DIR}/Modules/cppjson")

add_compile_definitions(_WINSOCKAPI_)

add_executable(${PROJECT_NAME} main.cpp MangalibSession.cpp MangaCollection.cpp MangaDownloader.cpp Combiners/Utils.cpp Combiners/HtmlCombiner.cpp Combiners/RawImageCombiner.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC Modules/cppHttplib)
target_include_directories(${PROJECT_NAME} PUBLIC Modules/cppjson/single_include)
target_include_directories(${PROJECT_NAME} PUBLIC Modules/cppBase64)
target_include_directories(${PROJECT_NAME} PUBLIC "${OPENSSL_PATH}/include/")

target_link_directories(${PROJECT_NAME} PRIVATE "${OPENSSL_PATH}/lib/")

if (build_type STREQUAL debug)
message("Debug build")
target_link_libraries(${PROJECT_NAME} libssl nlohmann_json::nlohmann_json libcrypto)
elseif (build_type STREQUAL release)
message("Release build")
target_link_libraries(${PROJECT_NAME} libssl nlohmann_json::nlohmann_json libcrypto)
endif()
