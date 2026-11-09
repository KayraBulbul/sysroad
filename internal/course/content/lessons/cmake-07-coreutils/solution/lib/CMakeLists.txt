add_library(coreutils io.cpp)
target_include_directories(coreutils PUBLIC include)
target_compile_features(coreutils PUBLIC cxx_std_20)
target_link_libraries(coreutils PRIVATE project_warnings)
