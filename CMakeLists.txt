cmake_minimum_required(VERSION 3.5)

project(TrignoWireless LANGUAGES CXX VERSION 1.0.0)

include(Findliblsl.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

if(WIN32)
	SET(Boost_USE_STATIC_LIBS OFF)
	SET(Boost_USE_MULTITHREAD ON)

	find_package(Boost REQUIRED)

	IF(Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
	ENDIF(Boost_FOUND)

	SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})
else()
	find_package(Boost REQUIRED COMPONENTS thread)
endif()

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
	src/main.cpp
	src/mainwindow.cpp
	src/recording.cpp
	include/mainwindow.hpp
	include/recording.hpp
	window/mainwindow.ui
)

if(WIN32)
	target_link_libraries(${PROJECT_NAME}
		PRIVATE
		Qt5::Widgets
		LSL::lsl
		ws2_32
		${USED_LIBS}
	)
else()
	target_link_libraries(${PROJECT_NAME}
		PRIVATE
		Qt5::Widgets
		LSL::lsl
		Boost::thread
	)
endif()

installLSLApp(${PROJECT_NAME})

LSLGenerateCPackConfig()