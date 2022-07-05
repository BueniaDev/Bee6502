set(BEE6502_HEADERS
	bee6502.h)

set(BEE6502_SOURCES
	bee6502.cpp)

add_library(bee6502 ${BEE6502_SOURCES} ${BEE6502_HEADERS})
target_include_directories(bee6502 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(bee6502 PRIVATE BEE6502_STATIC=1 _CRT_SECURE_NO_WARNINGS=1)
add_library(libbee6502 ALIAS bee6502)