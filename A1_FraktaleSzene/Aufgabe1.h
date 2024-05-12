#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLFrustum.h>
#include <math.h>
#include <math3d.h>
#include <GL/freeglut.h>
#include <AntTweakBar.h>
#include <vector>
//Für interleaved vertex buffer objects ist es übersichtlicher eine einfache Struktur zu erzeugen.
//Hier besitzt ein Vertex eine Position und ein Farbwert
struct ColoredVertex
{
	glm::vec3 position;
	glm::vec4 color;
};

GLfloat cube_strip[14][3] = {
	{-1.f, 1.f, 1.f},     // Front-top-left
	{1.f, 1.f, 1.f},      // Front-top-right
	{-1.f, -1.f, 1.f},    // Front-bottom-left
	{1.f, -1.f, 1.f},     // Front-bottom-right
	{1.f, -1.f, -1.f },    // Back-bottom-right
	{1.f, 1.f, 1.f},      // Front-top-right
	{1.f, 1.f, -1.f},     // Back-top-right
	{-1.f, 1.f, 1.f},     // Front-top-left
	{-1.f, 1.f, -1.f},    // Back-top-left
	{-1.f, -1.f, 1.f},    // Front-bottom-left
	{-1.f, -1.f, -1.f},   // Back-bottom-left
	{1.f, -1.f, -1.f},    // Back-bottom-right
	{-1.f, 1.f, -1.f},    // Back-top-left
	{1.f, 1.f, -1.f}      // Back-top-right
};
GLuint cubeIndices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

const float s_8_9 = sqrt(0.8f / 0.9f); // = 0.9428f
const float s_2_9 = sqrt(0.2f / 0.9f); // = 0.4714f
const float s_2_3 = sqrt(0.2f / 0.3f); // = 0.8165f

float tetrahedron_coords[4][3] = {
	{ 0.0f,   0.0f,   1.0f },
	{ s_8_9,  0.0f,  -1.0f / 3.0f},
	{-s_2_9,  s_2_3, -1.0f / 3.0f},
	{-s_2_9, -s_2_3, -1.0f / 3.0f},
};


int tetrahedron_indices[4][3] = { {0, 2, 1},  {0, 3, 2},  {1, 3, 0}, {1, 2, 3} };
GLuint tetrahedronIndices[] = { 0, 2, 1, 0 , 3, 2, 1, 3, 0, 1, 2, 3 };

glm::vec3 tetravec3[4] = {
	glm::vec3(0.0f,   0.0f,   1.0f),
	glm::vec3(s_8_9,  0.0f,  -1.0f / 3.0f),
	glm::vec3(-s_2_9,  s_2_3, -1.0f / 3.0f),
	glm::vec3(-s_2_9, -s_2_3, -1.0f / 3.0f),
};

void createTetrahedronWithVBOVBA(GLuint& VAOtetrahedron, GLuint& indexBufferTetrahedron, GLuint vBufferIdTetrahedron) {
	// Ein Array von 4 Vertices
	ColoredVertex tetrahedronVertices[4];
	for (int i = 0; i < 4; i++) {
		tetrahedronVertices[i].position = tetravec3[i];
		// i = 1->red, i = 2 -> blue, i = 3 -> green, i = 4 -> yellow
		switch (i) {
		case 0:
			tetrahedronVertices[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 1:
			tetrahedronVertices[i].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			break;
		case 2:
			tetrahedronVertices[i].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 3:
			tetrahedronVertices[i].color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
			break;
		}
	}
	// Vertex Array Object erzeugen
	glGenVertexArrays(1, &VAOtetrahedron);
	glBindVertexArray(VAOtetrahedron);
	// Index Buffer erzeugen
	glGenBuffers(1, &indexBufferTetrahedron);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferTetrahedron);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 12, tetrahedronIndices, GL_STATIC_DRAW);

	// Vertex Buffer Object erzeugen
	glGenBuffers(1, &vBufferIdTetrahedron);
	glBindBuffer(GL_ARRAY_BUFFER, vBufferIdTetrahedron);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ColoredVertex) * 4, tetrahedronVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (const GLvoid*)(sizeof(glm::vec3)));
	glBindVertexArray(0);
}

void createTetrahedronWithVertexArrays(GLBatch& batch) {
	// difference here, we use CopyVertexData3f instead of Vertex3f(v)
	batch.Begin(GL_TRIANGLES, 12);
	glm::vec3 tetrahedronVertices[12];
	glm::vec4 tetrahedronColors[12];
	for (int i = 0; i < 3; i++) {
		tetrahedronVertices[i] = glm::make_vec3(tetrahedron_coords[tetrahedron_indices[0][i]]);
		tetrahedronColors[i] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	// blue
	for (int i = 0; i < 3; i++) {
		tetrahedronVertices[i + 3] = glm::make_vec3(tetrahedron_coords[tetrahedron_indices[1][i]]);
		tetrahedronColors[i + 3] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	// green
	for (int i = 0; i < 3; i++) {
		tetrahedronVertices[i + 6] = glm::make_vec3(tetrahedron_coords[tetrahedron_indices[2][i]]);
		tetrahedronColors[i + 6] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	// yellow
	for (int i = 0; i < 3; i++) {
		tetrahedronVertices[i + 9] = glm::make_vec3(tetrahedron_coords[tetrahedron_indices[3][i]]);
		tetrahedronColors[i + 9] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	batch.CopyVertexData3f(glm::value_ptr(tetrahedronVertices[0]));
	batch.CopyColorData4f(glm::value_ptr(tetrahedronColors[0]));
	batch.End();
}

void createCubeWithVertexArrays(GLBatch& batch) {
	batch.Begin(GL_TRIANGLE_STRIP, 14);
	glm::vec3 cubeVertices[14];
	glm::vec4 cubeColors[14];
	// each side of the cube is drawn with a different color
	// red
	for (int i = 0; i < 14; i++) {
		// färbe den Vertex mit rot, grün oder blau ein
		if (i < 4) {
			cubeColors[i] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (i < 8) {
			cubeColors[i] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (i < 12) {
			cubeColors[i] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else {
			cubeColors[i] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		//berechne die normale
		cubeVertices[i] = glm::make_vec3(cube_strip[i]);
	}
	batch.CopyVertexData3f(glm::value_ptr(cubeVertices[0]));
	batch.CopyColorData4f(glm::value_ptr(cubeColors[0]));
	batch.End();
}

void createTetrahedron(GLBatch& batch) {
	batch.Begin(GL_TRIANGLES, 12);
	// red color
	for (int i = 0; i < 3; i++) {
		batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		batch.Vertex3fv(tetrahedron_coords[tetrahedron_indices[0][i]]);
	}
	// blue
	for (int i = 0; i < 3; i++) {
		batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		batch.Vertex3fv(tetrahedron_coords[tetrahedron_indices[1][i]]);
	}
	// green
	for (int i = 0; i < 3; i++) {
		batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		batch.Vertex3fv(tetrahedron_coords[tetrahedron_indices[2][i]]);
	}
	// yellow
	for (int i = 0; i < 3; i++) {
		batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		batch.Vertex3fv(tetrahedron_coords[tetrahedron_indices[3][i]]);
	}
	batch.End();
}

void createCube(GLBatch& batch) {
	batch.Begin(GL_TRIANGLE_STRIP, 14);
	// each side of the cube is drawn with a different color
	// red

	for (int i = 0; i < 14; i++) {
		// färbe den Vertex mit rot, grün oder blau ein
		if (i < 4) {
			batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (i < 8) {
			batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (i < 12) {
			batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else {
			batch.Color4f(1.0f, 1.0f, 0.0f, 1.0f);
		}
		batch.Vertex3fv(cube_strip[i]);
	}
	batch.End();
}
