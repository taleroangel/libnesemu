# Reference:
# https://www.nesdev.org/wiki/Emulator_tests
message(NOTICE "NESEMU: tests will be build!.")

# nestest by Kevin Horton
add_executable(TestNestest "src/nestest.c")
target_link_libraries(TestNestest PUBLIC nesemu)
add_test(
    NAME TestNestest
    COMMAND $<TARGET_FILE:TestNestest>
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/resources"
)
