cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

project(loaders)

set(CMAKE_CXX_STANDARD 23)

add_library(loaders INTERFACE)
target_include_directories(loaders INTERFACE image_loaders/img_loader.h ../../../../../external/stb_image)


add_executable(img_loader_example examples/img_loader_example/img_loader_ex_main.cpp)


target_include_directories(img_loader_example PUBLIC  image_loaders)
target_link_libraries(img_loader_example loaders)


add_custom_command(TARGET img_loader_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/examples/assets"
        "$<TARGET_FILE_DIR:img_loader_example>/assets")

#-------- Catch2 -------------
include (FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)


# --- test executables -------------
add_executable(img_loader_tests tests/img_loader_tests.cpp)
target_include_directories(img_loader_tests PRIVATE image_loaders)
target_link_libraries(img_loader_tests
            PRIVATE
            loaders
            Catch2::Catch2WithMain)



# copy test assets alongside the test binary
add_custom_command(TARGET img_loader_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/examples/assets"
        "$<TARGET_FILE_DIR:img_loader_tests>/assets")

include(CTest)
include(Catch)
