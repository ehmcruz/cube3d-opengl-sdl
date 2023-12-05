set(PACMAN_SOURCE_FILES
	game-object.cpp
	game-world.cpp
	graphics.cpp
	lib.cpp
	events.cpp
	graphics/sdl.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(PACMAN_SOURCE_FILES ${PACMAN_SOURCE_FILES}
		pc/main.cpp)
endif()

if (MSVC)
	set(PACMAN_SOURCE_FILES ${PACMAN_SOURCE_FILES}
		pc/main.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
	set(PACMAN_SOURCE_FILES ${PACMAN_SOURCE_FILES}
		android/main.cpp)
endif()

if (SUPPORT_OPENGL)
	set(PACMAN_SOURCE_FILES ${PACMAN_SOURCE_FILES}
		graphics/opengl.cpp)
endif()

# -------------------------------------

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_executable(pacman ${PACMAN_SOURCE_FILES})
endif()

if (MSVC)
	add_executable(pacman ${PACMAN_SOURCE_FILES})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	add_library(main SHARED ${PACMAN_SOURCE_FILES})
endif()

# -------------------------------------

#set_target_properties(
#	pacman PROPERTIES
#	NO_SYSTEM_FROM_IMPORTED true) # remove -isystem from system libs and use -I to include everything

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(pacman ${SDL2_LIBRARIES} ${Boost_LIBRARIES})
endif()

if (MSVC)
	target_link_libraries(pacman ${SDL2_LIBRARIES} ${my_Boost_LIBRARIES})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	target_link_libraries(main SDL2)
endif()

if (SUPPORT_OPENGL)
	target_link_libraries(pacman ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
endif()
