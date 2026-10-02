# test/CMakeLists.txt
add_executable(Tests LexerTest.cpp)

target_link_libraries(Tests PRIVATE bshc_core)