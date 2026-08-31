add_executable(bike_dashcam_tests
    TestMain.cpp
    ConfigManagerTests.cpp
    DashcamControllerTests.cpp
)

target_include_directories(bike_dashcam_tests PRIVATE "${PROJECT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(bike_dashcam_tests PRIVATE bike_dashcam_core)
bike_dashcam_apply_warnings(bike_dashcam_tests)

add_test(NAME bike_dashcam_tests COMMAND bike_dashcam_tests)
