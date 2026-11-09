add_test(NAME echo_words COMMAND echo -n a b)
set_tests_properties(echo_words PROPERTIES PASS_REGULAR_EXPRESSION "^a b")

add_test(NAME cat_numbers COMMAND cat -n ${CMAKE_CURRENT_SOURCE_DIR}/data/hello.txt)
set_tests_properties(cat_numbers PROPERTIES PASS_REGULAR_EXPRESSION "1\thello\n +2\tworld")

add_test(NAME wc_counts COMMAND wc ${CMAKE_CURRENT_SOURCE_DIR}/data/hello.txt)
set_tests_properties(wc_counts PROPERTIES PASS_REGULAR_EXPRESSION "^ +2 +2 +12 ")

add_test(NAME wc_missing_file COMMAND wc no-such-file.txt)
set_tests_properties(wc_missing_file PROPERTIES WILL_FAIL TRUE)
