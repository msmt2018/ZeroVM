cmake_minimum_required(VERSION 3.22)
project(zerodroid_native LANGUAGES C CXX)

option(ZEROVM_BUILD_TESTS "Build host-side native contract tests" ON)

add_library(zerodroid_core STATIC
    src/status.cpp
    src/runtime.cpp
)
target_include_directories(zerodroid_core PUBLIC include)
target_compile_features(zerodroid_core PUBLIC cxx_std_20)
target_compile_definitions(zerodroid_core PRIVATE ZEROVM_BUILDING_LIBRARY)
set_target_properties(zerodroid_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    POSITION_INDEPENDENT_CODE YES
)

if(ANDROID)
    add_library(zerodroid_jni SHARED src/jni_bridge.cpp)
    target_link_libraries(zerodroid_jni PRIVATE zerodroid_core log)
    target_compile_features(zerodroid_jni PRIVATE cxx_std_20)
endif()

if(ZEROVM_BUILD_TESTS AND NOT ANDROID)
    enable_testing()
    add_executable(zerodroid_native_contract_tests tests/native_contract_tests.cpp)
    target_link_libraries(zerodroid_native_contract_tests PRIVATE zerodroid_core)
    add_test(NAME zerodroid_native_contract_tests COMMAND zerodroid_native_contract_tests)
endif()
