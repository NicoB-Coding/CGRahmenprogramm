cmake_minimum_required(VERSION 3.10)

# Projektname setzen
SET(PROJECT_NAME A1_FraktaleSzene)

# Aufgabe1.cpp und Aufgabe1.h hinzufügen
SET(SOURCES
    Aufgabe1.cpp
    Aufgabe1.h
)

# Bibliotheken festlegen
set(LIBRARIES GLTools freeglut AntTweakBar Utils)

project(${PROJECT_NAME})

# Utils-Bibliothek hinzufügen (relative Pfadangabe)
add_subdirectory(../Utils ${CMAKE_BINARY_DIR}/Utils_build_${PROJECT_NAME})

# Executable erzeugen
add_executable(${PROJECT_NAME} ${SOURCES})
create_target_launcher(${PROJECT_NAME} WORKING_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR})

# Bibliotheken verlinken
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# Property setzen
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Aufgaben")

# Sample installieren
install_sample(${PROJECT_NAME} ${SOURCES})
