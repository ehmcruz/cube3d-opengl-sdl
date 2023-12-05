set(SOURCE_FILES
	graphics.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(SOURCE_FILES ${SOURCE_FILES}
		main.cpp)
endif()

if (MSVC)
	set(SOURCE_FILES ${SOURCE_FILES}
		main.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
	set(SOURCE_FILES ${SOURCE_FILES}
		android/main.cpp)
endif()

if (SUPPORT_OPENGL)
	set(SOURCE_FILES ${SOURCE_FILES}
		opengl/opengl.cpp)
endif()

# -------------------------------------

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_executable(cube3d ${SOURCE_FILES})
endif()

if (MSVC)
	add_executable(cube3d ${SOURCE_FILES})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	add_library(main SHARED ${SOURCE_FILES})
endif()

# -------------------------------------

#set_target_properties(
#	pacman PROPERTIES
#	NO_SYSTEM_FROM_IMPORTED true) # remove -isystem from system libs and use -I to include everything

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(cube3d ${SDL2_LIBRARIES})
endif()

if (MSVC)
	target_link_libraries(cube3d ${SDL2_LIBRARIES})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	target_link_libraries(main SDL2)
endif()

if (SUPPORT_OPENGL)
	target_link_libraries(cube3d ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
endif()
