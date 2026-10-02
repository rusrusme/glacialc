# test/CMakeLists.txt
add_executable(LexerTest LexerTest.cpp)
target_link_libraries(LexerTest PRIVATE bshc_core)

add_executable(PreprocessorTest PreprocessorTest.cpp)
target_link_libraries(PreprocessorTest PRIVATE bshc_core)

add_executable(CompilerTest CompilerTest.cpp)
target_link_libraries(CompilerTest PRIVATE bshc_core)

