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
float cameraRadius = 0.0f;  // Entfernung vom Ursprung


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
void createTetrahedronWithVBOVBA(
	GLuint& VAO, GLuint& VBO, GLuint& EBO,
	const std::vector<glm::vec3>& vertices, const std::vector<GLuint>& indices) {

	// Ein Array von Vertices erstellen
	std::vector<ColoredVertex> coloredVertices(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i) {
		coloredVertices[i].position = vertices[i];
		// Setze eine Farbe, z.B. nach einer festen Palette oder zufällig
		if (i % 4 == 0) {
			coloredVertices[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (i % 4 == 1) {
			coloredVertices[i].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (i % 4 == 2) {
			coloredVertices[i].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else {
			coloredVertices[i].color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
	}
	// Vertex Array Object erzeugen
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Index Buffer erzeugen
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Vertex Buffer Object erzeugen
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ARRAY_BUFFER, EBO);
	glBufferData(GL_ARRAY_BUFFER, coloredVertices.size() * sizeof(ColoredVertex), coloredVertices.data(), GL_STATIC_DRAW);

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
void quaternionToOrientationAndPosition(const glm::quat& q, GLFrame& frame) {
    // Verwende glm::mat3_cast, um eine Rotationsmatrix aus dem Quaternion zu erzeugen
    glm::mat3 rotationMatrix = glm::mat3_cast(q);

    // Der Up-Vektor für die Kamera (Y Richtung)
    glm::vec3 up = rotationMatrix[1];

    // Berechnen der neuen Kameraposition nach der Rotation
    glm::vec3 origin =  rotationMatrix[0];  // Verwende den Right-Vektor, um die Kamera zu positionieren
    frame.SetOrigin(origin.x, origin.y, origin.z);

    // Der Forward-Vektor für die Kamera soll zum Nullpunkt zeigen
    glm::vec3 forward = -origin;  // Richtet die Kamera zum Nullpunkt aus
    forward = glm::normalize(forward);  // Normalisiere den Forward-Vektor

    // Setzen der Vektoren im GLFrame
    frame.SetForwardVector(forward.x, forward.y, forward.z);
    frame.SetUpVector(up.x, up.y, up.z);
}
void DrawCoordinateSystem(float length) {
	int gridLines = 10;  // Anzahl der Gitterlinien pro Richtung
	float step = length / gridLines;  // Schrittgröße zwischen den Gitterlinien
	glBegin(GL_LINES);

	// Zeichnen der X-Achse in Rot
	glColor3f(1.0f, 0.0f, 0.0f); // Rot
	glVertex3f(0.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f); // Rot
	glVertex3f(length, 0.0f, 0.0f);

	// Zeichnen der Y-Achse in Grün
	glColor3f(0.0f, 1.0f, 0.0f); // Grün
	glVertex3f(0.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f); // Grün
	glVertex3f(0.0f, length, 0.0f);

	// Zeichnen der Z-Achse in Blau
	glColor3f(0.0f, 0.0f, 1.0f); // Blau
	glVertex3f(0.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 1.0f); // Blau

	glVertex3f(0.0f, 0.0f, length);

	// Zeichnen des weißen Gitternetzes auf der XZ-Ebene
	glColor3f(1.0f, 1.0f, 1.0f); // Weiß

	// Vertikale Linien entlang der X-Achse
	for (int i = 0; i <= gridLines; i++) {
		glColor3f(1.0f, 1.0f, 1.0f); // Weiß
		glVertex3f(i * step, 0.0f, 0.0f);
		glVertex3f(i * step, 0.0f, length);
		glVertex3f(-i * step, 0.0f, 0.0f);
		glVertex3f(-i * step, 0.0f, length);
	}

	// Horizontale Linien entlang der Z-Achse
	for (int i = 0; i <= gridLines; i++) {
		glColor3f(1.0f, 1.0f, 1.0f); // Weiß
		glVertex3f(0.0f, 0.0f, i * step);
		glVertex3f(length, 0.0f, i * step);
		glVertex3f(0.0f, 0.0f, -i * step);
		glVertex3f(length, 0.0f, -i * step);
	}

	glEnd();
}
void m3dMidPoint(glm::vec3& result, const glm::vec3& a, const glm::vec3& b) {
	result = (a + b) / 2.0f;
}

// Diese Funktion teilt ein Dreieck rekursiv in vier Unterdreiecke, bis die Tiefe 0 erreicht ist
void divideTriangle(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, int depth, GLuint& index) {
	if (depth == 0) {
		for (int i = 0; i < 12; ++i) {
			indices.push_back(tetrahedronIndices[i] + index);
		}
		index += 4;

		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
	}
	else {
		glm::vec3 v01, v12, v20, v03, v13, v23;
		m3dMidPoint(v01, v0, v1);
		m3dMidPoint(v12, v1, v2);
		m3dMidPoint(v20, v2, v0);
		m3dMidPoint(v03, v0, v3);
		m3dMidPoint(v13, v1, v3);
		m3dMidPoint(v23, v2, v3);
		divideTriangle(vertices, indices, v0, v01, v20, v03, depth - 1, index);
		divideTriangle(vertices, indices, v01, v1, v12, v13, depth - 1, index);
		divideTriangle(vertices, indices, v20, v12, v2, v23, depth - 1, index);
		divideTriangle(vertices, indices, v03, v13, v23, v3, depth - 1, index);
	}
}

void generateSierpinskiTetrahedron(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices, int recursionDepth) {
	// Initiale Vertices des Tetraeders hinzufügen
	glm::vec3 v0(0.0f, 0.0f, 1.0f);
	glm::vec3 v1(s_8_9, 0.0f, -1.0f / 3.0f);
	glm::vec3 v2(-s_2_9, s_2_3, -1.0f / 3.0f);
	glm::vec3 v3(-s_2_9, -s_2_3, -1.0f / 3.0f);

	GLuint index = 0;
	divideTriangle(vertices, indices, v0, v1, v2, v3, recursionDepth, index);
}