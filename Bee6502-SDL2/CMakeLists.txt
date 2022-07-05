project(Bee6502-SDL2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL_MAIN_HANDLED")

add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCES})
target_link_libraries(${PROJECT_NAME} libbee6502)

find_package(SDL2 REQUIRED)

if (TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

