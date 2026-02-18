cmake_minimum_required(VERSION 4.0.0)
project (project_management LANGUAGES CXX)


add_library(project_management STATIC Project.cpp include/Project.h)


target_include_directories(project_management PUBLIC include/)
target_compile_features(project_management PUBLIC cxx_std_23)

target_link_libraries(project_management PUBLIC glaze::glaze)

# glaze requires this flag on MSVC to enable correct macro expansion
if(MSVC)
    target_compile_options(project_management PRIVATE /Zc:preprocessor)
endif()
