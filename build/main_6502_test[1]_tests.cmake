add_test([=[CPUTest.RunInlineProgram]=]  [==[C:/Users/obliv/Desktop/C Practice/CPUEmulator6502/build/Debug/main_6502_test.exe]==] [==[--gtest_filter=CPUTest.RunInlineProgram]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[CPUTest.RunInlineProgram]=]  PROPERTIES WORKING_DIRECTORY [==[C:/Users/obliv/Desktop/C Practice/CPUEmulator6502/build]==] SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  main_6502_test_TESTS CPUTest.RunInlineProgram)
