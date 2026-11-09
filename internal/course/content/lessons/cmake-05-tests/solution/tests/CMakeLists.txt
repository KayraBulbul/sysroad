add_executable(textutil_tests test_textutil.cpp)
target_link_libraries(textutil_tests PRIVATE textutil project_warnings)
add_test(NAME textutil_tests COMMAND textutil_tests)

# wc itself, end to end.
add_test(NAME wc_poem COMMAND wc ${CMAKE_CURRENT_SOURCE_DIR}/data/poem.txt)
set_tests_properties(wc_poem PROPERTIES PASS_REGULAR_EXPRESSION "^ +5 +14 +72 ")

add_test(NAME wc_missing_file COMMAND wc no-such-file.txt)
set_tests_properties(wc_missing_file PROPERTIES WILL_FAIL TRUE)
