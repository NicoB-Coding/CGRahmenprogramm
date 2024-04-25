/**
* Ein leeres Projekt für Aufgabe 1 und 2
* 1.2 a) Programmieren Sie Ihre eigene „fraktale Szene“, indem Sie einfache Grundbausteine 
(z.B. Tetraeder, Würfel, Dodekaeder, usw.) über mehrere Rekursionsstufen zu komplexeren Objekten 
zusammensetzen (z.B. Sierpinski-Tetraeder (siehe unten), Menger-Schwamm (siehe unten), Farne, Bäume, 
fraktale Landschaften usw., weitere Erläuterungen dazu finden Sie unter Lindenmayer-Systemen  und Beispielen ).
(Benutzen Sie die Modell-Transformationen Translation, Rotation und, falls nötig Skalierung). 
Setzen Sie dabei die Technik der "Matrizen Stapel" ein, wie bei dem Beispiel mit dem Automobil 
(Koordinatensysteme und Transformationen » Beispiel für Matrizen-Stapel). Gestalten Sie Ihre Szene so, 
dass Sie zwischen den Rekursionsstufen hin- und herschalten können und die Objekte später animieren (d.h. bewegen) können. 
Realisieren Sie Ihre "fraktale Szene" wie in "A1_Versuch1a" dargestellt, 
d.h. für OpenGL mit Hilfe der GLBatch-Klasse aus der GLTools-Library und den Methoden 
".Begin/.End", sowie .Vertex3f und Color4f.
*/
#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#define GLM_FORCE_RADIANS
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

GLShaderManager shaderManager;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;
GLBatch pyramidBatch;
// create GLBatch for the menger sponge
GLBatch cubeBatch;

GLuint VAOcube;
GLuint indexBufferCube;
GLuint vBufferIdCube;

GLuint VAOtetrahedron;
GLuint indexBufferTetrahedron;
GLuint vBufferIdTetrahedron;

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
GLuint tetrahedronIndices[] = { 0, 2, 1, 0 , 3, 2, 1, 3, 0, 1, 2, 3};

const float s_8_9 = sqrt(0.8f / 0.9f); // = 0.9428f
const float s_2_9 = sqrt(0.2f / 0.9f); // = 0.4714f
const float s_2_3 = sqrt(0.2f / 0.3f); // = 0.8165f

float tetrahedron_coords[4][3] = {
	{ 0.0f,   0.0f,   1.0f },
	{ s_8_9,  0.0f,  -1.0f / 3.0f},
	{-s_2_9,  s_2_3, -1.0f / 3.0f},
	{-s_2_9, -s_2_3, -1.0f / 3.0f},
};
glm::vec3 tetravec3[4] = {
	glm::vec3( 0.0f,   0.0f,   1.0f),
	glm::vec3( s_8_9,  0.0f,  -1.0f / 3.0f),
	glm::vec3( - s_2_9,  s_2_3, -1.0f / 3.0f),
	glm::vec3( - s_2_9, -s_2_3, -1.0f / 3.0f),
};
//Für interleaved vertex buffer objects ist es übersichtlicher eine einfache Struktur zu erzeugen.
//Hier besitzt ein Vertex eine Position und ein Farbwert
struct ColoredVertex
{
	glm::vec3 position;
	glm::vec4 color;
};
int tetrahedron_indices[4][3] = { {0, 2, 1},  {0, 3, 2},  {1, 3, 0}, {1, 2, 3} };

// Rotationsgroessen
glm::quat rotation = glm::quat(0, 0, 0, 1);

// bool for levers
bool bSierpinski = false;
bool bMengerSponge = false;
// depth of recursion for sierpinski and menger sponge
int depth = 0;
int prevDepth = 0;
GLfloat globalScale = 0.7f;
bool bDepth = true;

// length of the cube
float currentLength = 2.0f;
float mengerScaleFactor = 1.0f;

// Kamera Translation
static float xTrans = 0.0f;
static float yTrans = 0.0f;
static float zTrans = 0.0f;

void createTetrahedronWithVBOVBA() {
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(ColoredVertex) * 4 , tetrahedronVertices, GL_STATIC_DRAW);

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
		batch.Color4f(0.0f, 0.0f, 1.0f, 1.0f);
		batch.Vertex3fv(tetrahedron_coords[tetrahedron_indices[1][i]]);
	}
	// green
	for (int i = 0; i < 3; i++) {
		batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		batch.Vertex3fv(tetrahedron_coords[tetrahedron_indices[2][i]]);
	}
	// yellow
	for (int i = 0; i < 3; i++) {
		batch.Color4f(1.0f, 1.0f, 0.0f, 1.0f);
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
		else {
			batch.Color4f(1.0f, 1.0f, 0.0f, 1.0f);
		}
		batch.Vertex3fv(cube_strip[i]);
	}
	batch.End();
}
void drawSierpinski(int sierpinskiDepth) {
	if (sierpinskiDepth == 0) {
		// Grundfall: Zeichne ein einzelnes Tetraeder
		shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix());
		//pyramidBatch.Draw();
		glBindVertexArray(VAOtetrahedron);
		glBindBuffer(GL_ARRAY_BUFFER, vBufferIdTetrahedron);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferTetrahedron);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	else {
		// Rekursiver Fall: Erzeuge 4 kleinere Tetraeder
		for (int i = 0; i < 4; i++) {
			modelViewMatrix.PushMatrix();
			// Skalierungsfaktor ist 1/2^tiefe
			// Translation Faktor
			float scale = 1 * pow(2, sierpinskiDepth-1);
			modelViewMatrix.Translate(tetrahedron_coords[i][0]*scale, tetrahedron_coords[i][1] * scale, tetrahedron_coords[i][2] * scale);
			// Rekursiver Aufruf für jedes Tetraeder
			drawSierpinski(sierpinskiDepth - 1);
			modelViewMatrix.PopMatrix();
		}
	}
}

void drawMenger(int mengerDepth) {
	if (mengerDepth == 0) {
		// Grundfall: Zeichne einen einzelnen Würfel
		shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix());
		cubeBatch.Draw();
	}
	else {
		// Rekursiver Fall: Erzeuge 20 kleinere Würfel
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				for (int l = -1; l <= 1; l++) {
					// Korrektur: Entferne nur den zentralen Würfel jeder Schicht
					if ((j == 0 && k == 0) || (j == 0 && l == 0) || (k == 0 && l == 0)) {
						continue;
					}
					modelViewMatrix.PushMatrix();
					float scale = 1.64f * pow(3, mengerDepth-1);
					modelViewMatrix.Translate(j*scale, k*scale, l*scale);
					// Rekursiver Aufruf für jeden Würfel
					drawMenger(mengerDepth - 1);
					modelViewMatrix.PopMatrix();
				}
			}
		}
	}
}

//GUI
TwBar *bar;
void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'");
	TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F, &rotation, "");
	//Hier weitere GUI Variablen anlegen. Für Farbe z.B. den Typ TW_TYPE_COLOR4F benutzen
	// Add button to check if siepinski is drawn
	TwAddVarRW(bar, "Draw Sierpinski", TW_TYPE_BOOLCPP, &bSierpinski, " label='Draw Sierpinski' ");
	// Add button to check if menge sponge is drawn
	TwAddVarRW(bar, "Draw Menger Sponge", TW_TYPE_BOOLCPP, &bMengerSponge, " label='Draw Menger Sponge' ");
	// Add button to change the depth (max of 3) of the recursion, if the value is changed we need to draw the geometry again
	TwAddVarRW(bar, "Depth", TW_TYPE_INT32, &depth, " label='Depth' min=0 max=10 ");
	// add sensitive slider for the scaling of the geometry
	TwAddVarRW(bar, "Scale", TW_TYPE_FLOAT, &globalScale, " label='Scale' min=0.1 max=10 step=0.1 ");
}

void CreateGeometry()
{
	createTetrahedronWithVBOVBA();
	createCubeWithVertexArrays(cubeBatch);
}

// Aufruf draw scene
void RenderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	modelViewMatrix.PushMatrix();

	modelViewMatrix.Translate(xTrans, yTrans, zTrans);

	modelViewMatrix.Scale(globalScale, globalScale, globalScale);
	glm::mat4 rot = glm::mat4_cast(glm::quat(rotation.z, rotation.w, rotation.x, rotation.y));
	modelViewMatrix.MultMatrix(glm::value_ptr(rot));

	// has the depth of the recursion changed?
	if (prevDepth != depth) {
		// clear the batches
		pyramidBatch.~GLBatch();
		pyramidBatch = GLBatch();
		cubeBatch.~GLBatch();
		cubeBatch = GLBatch();
		CreateGeometry();
	}
	// Draw the Sierpinski pyramid if bSierpinski is true
	if (bSierpinski) {
		modelViewMatrix.PushMatrix();
		float sierpinskiScaleFactor = 1 / pow(2, depth);
		modelViewMatrix.Scale(sierpinskiScaleFactor, sierpinskiScaleFactor, sierpinskiScaleFactor);
		drawSierpinski(depth);
		modelViewMatrix.PopMatrix();
	}
	if (bMengerSponge) {
		// die würfel müssen an den richtigen stellen gezeichnet werden
		modelViewMatrix.PushMatrix();
		mengerScaleFactor = 1.0f / pow(3, depth);
		modelViewMatrix.Scale(mengerScaleFactor, mengerScaleFactor, mengerScaleFactor);
		drawMenger(depth);
		modelViewMatrix.PopMatrix();
	}
		
	// save the depth of the recursion
	prevDepth = depth;

	modelViewMatrix.PopMatrix();
	gltCheckErrors();
	TwDraw();
	glutSwapBuffers();
	glutPostRedisplay();
}

// Initialisierung des Rendering Kontextes
void SetupRC()
{
	// hellorange
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glFrontFace(GL_CW);
	// Backface Culling aktivieren
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//initialisiert die standard shader
	shaderManager.InitializeStockShaders();
	//Matrix stacks für die Transformationspipeline setzen, damit werden dann automatisch die Matrizen multipliziert
	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);

	//erzeuge die geometrie
	CreateGeometry();
	InitGUI();
}

void SpecialKeys(int key, int x, int y)
{
	float angleStep = 8.0f;  // Rotationswinkel in Grad

	switch (key)
	{
	case GLUT_KEY_UP: // 101
		// Vorwärts bewegen
		//zTrans -= 1.0f;
		// Rotation um die X-Achse
		rotation = glm::rotate(rotation, glm::radians(-angleStep), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case GLUT_KEY_DOWN: // 103
		// Rückwärts bewegen
		//zTrans += 1.0f;
		// Rotation um die X-Achse
		rotation = glm::rotate(rotation, glm::radians(angleStep), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case GLUT_KEY_LEFT: // 100
		// Links bewegen
		xTrans -= 1.0f;
		// Rotation um die Y-Achse
		rotation = glm::rotate(rotation, glm::radians(-angleStep), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case GLUT_KEY_RIGHT: // 102
		// Rechts bewegen
		xTrans += 1.0f;
		// Rotation um die Y-Achse
		rotation = glm::rotate(rotation, glm::radians(angleStep), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case GLUT_KEY_PAGE_UP:
		// Hoch bewegen
		yTrans += 1.0f;
		break;
	case GLUT_KEY_PAGE_DOWN:
		// Runter bewegen
		yTrans -= 1.0f;
		break;
	default:
		break;
	}

	// AntTweakBar Event
	TwEventSpecialGLUT(key, x, y);

	// Fenster neu zeichnen
	glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
	GLfloat nRange = 7.0f;

	// Verhindere eine Division durch Null
	if(h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();
	
	// Definiere das viewing volume (left, right, bottom, top, near, far)
	if (w <= h) 
		viewFrustum.SetOrthographic(-nRange, nRange, -nRange*h/w, nRange*h/w, -nRange, nRange);
	else 
		viewFrustum.SetOrthographic(-nRange*w/h, nRange*w/h, -nRange, nRange, -nRange, nRange);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	// Ruecksetzung des Model view matrix stack
	modelViewMatrix.LoadIdentity();
	
	TwWindowSize(w, h);
}

void ShutDownRC()
{
	//Aufräumen
	TwTerminate();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000,800);
	glutCreateWindow("Aufgabe1");
	glutCloseFunc(ShutDownRC);
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Veralteter Treiber etc.
		std::cerr <<"Error: "<< glewGetErrorString(err) << "\n";
		return 1;
	}
	
	//GLUT Callbacks setzen
	//Um Mausevents selbst zu erhalten eigene Funktionen für glutMouseFunc, glutMotionFunc, glutPassiveMotionFunc setzen
	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);

	
	glutReshapeFunc(ChangeSize);
	glutSpecialFunc(SpecialKeys);
	glutDisplayFunc(RenderScene);
	
	TwInit(TW_OPENGL_CORE, NULL);
	SetupRC();

	glutMainLoop();

	return 0;
}
