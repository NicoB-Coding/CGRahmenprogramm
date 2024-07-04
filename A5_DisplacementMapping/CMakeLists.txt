SET(PROJECT_NAME A5_DisplacementMapping)

set(SOURCES DisplacementMapping.cpp
	    VertexShader.glsl
	    TessControlShader.glsl
	    TessEvalShader.glsl
	    FragmentShader.glsl
)
set(LIBRARIES GLTools ObjLoader ImageLoader AntTweakBar freeglut)

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} ${SOURCES})
create_target_launcher(${PROJECT_NAME}  WORKING_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Aufgaben")
install_sample(${PROJECT_NAME} ${SOURCES})
