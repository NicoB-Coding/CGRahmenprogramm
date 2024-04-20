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
GLBatch cubeBatch;

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

// Rotationsgroessen
glm::quat rotation = glm::quat(0, 0, 0, 1);

// bool for levers
bool bSierpinski = false;
bool bMengerSponge = false;
// depth of recursion for sierpinski and menger sponge
int depth = 0;
int prevDepth = 0;

// Kamera Translation
static float xTrans = 0.0f;
static float yTrans = 0.0f;
static float zTrans = 0.0f;

void m3dMidPoint(float result[3], const float a[3], const float b[3]) {
	result[0] = (a[0] + b[0]) / 2.0f;
	result[1] = (a[1] + b[1]) / 2.0f;
	result[2] = (a[2] + b[2]) / 2.0f;
}

// this function divides a triangle into four subtriangles recursively until depth is 0
void divideTriangle(GLBatch& batch, GLfloat v0[], GLfloat v1[], GLfloat v2[], GLfloat v3[], int depth) {
    if (depth == 0) {
		// red color
		batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        batch.Vertex3fv(v0);
		batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        batch.Vertex3fv(v2);
		batch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        batch.Vertex3fv(v1);
		 // magenta color
		batch.Color4f(1.0f, 0.0f, 1.0f, 1.0f);
        batch.Vertex3fv(v0);
		batch.Color4f(1.0f, 0.0f, 1.0f, 1.0f);
        batch.Vertex3fv(v1);
		batch.Color4f(1.0f, 0.0f, 1.0f, 1.0f);
        batch.Vertex3fv(v3);
		// blue color
		batch.Color4f(0.0f, 0.0f, 1.0f, 1.0f);
        batch.Vertex3fv(v0);
		batch.Color4f(0.0f, 0.0f, 1.0f, 1.0f);
        batch.Vertex3fv(v3);
		batch.Color4f(0.0f, 0.0f, 1.0f, 1.0f);
        batch.Vertex3fv(v2);
		// green color
		batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        batch.Vertex3fv(v1);
		batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        batch.Vertex3fv(v2);
		batch.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        batch.Vertex3fv(v3);
    }
    else {
        GLfloat v01[3], v12[3], v20[3], v03[3], v13[3], v23[3];
        m3dMidPoint(v01, v0, v1);
        m3dMidPoint(v12, v1, v2);
        m3dMidPoint(v20, v2, v0);
        m3dMidPoint(v03, v0, v3);
        m3dMidPoint(v13, v1, v3);
        m3dMidPoint(v23, v2, v3);
        divideTriangle(batch, v0, v01, v20, v03, depth - 1);
        divideTriangle(batch, v01, v1, v12, v13, depth - 1);
        divideTriangle(batch, v20, v12, v2, v23, depth - 1);
        divideTriangle(batch, v03, v13, v23, v3, depth - 1);
    }
}

void DrawSierpinski() {
	int numVertices = 4 * (int)pow(4, depth+1);  // Berechnung der Anzahl der Vertices
	pyramidBatch.Begin(GL_TRIANGLES, numVertices);

	GLfloat sierPinski[4][3] = {
		{-1.0f, -1.0f, -1.0f}, // v0 links
		{1.0f, -1.0f, -1.0f}, // Vorne rechts
		{0.0f, 1.0f, -1.0f}, // hinten
		{0.0f, 0.0f, 1.0f} // oben
	};
	divideTriangle(pyramidBatch, sierPinski[0], sierPinski[1], sierPinski[2], sierPinski[3], depth);
	pyramidBatch.End();
}

void divideCube(GLBatch& batch, int depth) {
	float length; // length of the cube
	if (depth == 0) {
		// draw the cube using triangle strip vertices with the sequence of vertices: 4 3 7 8 5 3 1 4 2 7 6 5 2 1
		// we already have the correct scaling and translation in the matrix stack
		// we just need to draw the cube
		batch.Begin(GL_TRIANGLE_STRIP, 14);
		for (int i = 0; i < 14; i++) {
			// make the cube pink
			batch.Color4f(1.0f, 0.0f, 1.0f, 1.0f);
			batch.Vertex3fv(cube_strip[i]);
		}
		batch.End();
	}
	else {
		// we want to know the depth of the recursion to determine the size of our cube
		// then we put the scaled size of the cube in our matrix stack to pop it out later for drawing
		modelViewMatrix.PushMatrix();
		length = 2.0f / (float)pow(3, depth);
		modelViewMatrix.Scale(length, length, length);
		// draw the 20 cubes
		for (int i = 0; i < 2; i++) {
			modelViewMatrix.PushMatrix();
			// translate the cube to the correct position
			switch (i) {
				case 0:
					modelViewMatrix.Translate(-length, length, length);
					break;
				case 1:
					modelViewMatrix.Translate(length, length, length);
					break;
				case 2:
					modelViewMatrix.Translate(-length, -length, length);
					break;
				case 3:
					modelViewMatrix.Translate(length, -length, length);
					break;
				case 4:
					modelViewMatrix.Translate(length, -length, -length);
					break;

			}
			divideCube(batch, depth - 1);
			modelViewMatrix.PopMatrix();
		}
		modelViewMatrix.PopMatrix();
	}
}
void DrawMengerSponge() {
	divideCube(cubeBatch, depth);
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
	// Add button to change the depth of the recursion, if the value is changed we need to draw the geometry again
	TwAddVarRW(bar, "Depth", TW_TYPE_INT32, &depth, " label='Depth' ");
}

void CreateGeometry()
{
	DrawSierpinski();
	DrawMengerSponge();
}

// Aufruf draw scene
void RenderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelViewMatrix.PushMatrix();
	// has the depth of the recursion changed?
	if (prevDepth != depth) {
		// clear the batches
		pyramidBatch.Reset();
		pyramidBatch = GLBatch();
		cubeBatch.Reset();
		cubeBatch = GLBatch();
		CreateGeometry();
	}
	modelViewMatrix.Translate(xTrans, yTrans, zTrans);
	glm::mat4 rot = glm::mat4_cast(rotation);
	modelViewMatrix.MultMatrix(glm::value_ptr(rot));

	// Konvertiere GLMatrixStack zu glm::mat4
	//glm::mat4 mvMatrix = glm::make_mat4(modelViewMatrix.GetMatrix());

	// Definiere und transformiere die Lichtpositionen mit GLM
	//glm::vec4 lightPosition0 = glm::vec4(-75.0f, 150.0f, 100.0f, 1.0f);
	//glm::vec4 lightPosition1 = glm::vec4(75.0f, 150.0f, 100.0f, 1.0f);

	//lightPosition0 = mvMatrix * lightPosition0;
	//lightPosition1 = mvMatrix * lightPosition1;

	//glLightfv(GL_LIGHT0, GL_POSITION, &lightPosition0[0]);
	//glLightfv(GL_LIGHT1, GL_POSITION, &lightPosition1[0]);

	//GLfloat vColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };
	//shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vColor);
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());


	// Draw the Sierpinski pyramid if bSierpinski is true
	if (bSierpinski) {
		pyramidBatch.Draw();
	}
	if (bMengerSponge) {
		cubeBatch.Draw();
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
	// Schwarzer Hintergrund
	glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
	//glFrontFace(GL_CW);
	// Backface Culling aktivieren
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	//initialisiert die standard shader
	shaderManager.InitializeStockShaders();
	//Matrix stacks für die Transformationspipeline setzen, damit werden dann automatisch die Matrizen multipliziert
	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);

	// Setup von Licht
	/*GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	GLfloat lightPos[] = { -75.0f, 150.0f, 100.0f, 1.0f };

	//glEnable(GL_LIGHTING);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glEnable(GL_LIGHT0);
	// Zweites Licht hinzufügen
	GLfloat lightPos2[] = { 75.0f, 150.0f, 100.0f, 1.0f };
	GLfloat diffuseLight2[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight2);

	GLfloat matAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat shininess = 40.0f;

	glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);*/

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
	GLfloat nRange = 10.0f;

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
