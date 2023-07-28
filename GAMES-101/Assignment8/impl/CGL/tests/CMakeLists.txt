link_libraries(CGL)

include_directories("${PROJECT_SOURCE_DIR}/src")

link_libraries(
  ${GLFW_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${FREETYPE_LIBRARIES}
)

# OSD
add_executable(osd osd.cpp)

# Install tests
install(TARGETS osd DESTINATION bin/tests)
