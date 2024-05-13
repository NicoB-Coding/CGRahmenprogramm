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
#include "Aufgabe1.h"
#define GL_PI 3.1415f
#include <GL/glut.h>
#include <Utils/Timer.h>

// matrix and shader stuff
GLShaderManager shaderManager;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;
TwBar* bar; // GUI bar
GLFrame cameraFrame; // camera frame


// batches for the geometry
GLBatch pyramidBatch;
GLBatch cubeBatch;

// cube buffer objects
GLuint VAOcube;
GLuint indexBufferCube;
GLuint vBufferIdCube;
// tetra buffer objects
GLuint VAOtetra;
GLuint indexBufferTetra;
GLuint vBufferIdTetra;

// rotation
glm::quat rotation = glm::quat(0, 0, 0, 1);
glm::quat animation = glm::quat(0, 0, 0, 1);
float currentAnimationAngle = 0.0f;


int depth = 0; // recursion depth
int prevDepth = 0;

GLfloat globalScale = 2.7f;
float currentLength = 2.0f; // cube length
float mengerScaleFactor = 1.0f;

// Camera Translation
static float xTrans = 0.0f;
static float yTrans = 0.0f;
static float zTrans = 0.0f;

// bool for levers
bool bSierpinski = false;
bool bMengerSponge = false;
bool bDepth = true;
bool bPerspective = false;
bool bUFO = false;

void drawSierpinski(int sierpinskiDepth) {
	if (sierpinskiDepth == 0) {
		// Grundfall: Zeichne ein einzelnes Tetraeder
		shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());
		pyramidBatch.Draw();
		//glBindVertexArray(VAOtetra);
		//glBindBuffer(GL_ARRAY_BUFFER, vBufferIdTetra);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferTetra);
		//glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0);
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
		shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());
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

void ChangeSize(int w, int h)
{
	GLfloat nRange = 15.0f;

	// Verhindere eine Division durch Null
	if (h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();


	// orthogonale oder perspektivische Projektion?
	if (bPerspective) {
		// Definiere das viewing volume (left, right, bottom, top, near, far)
		viewFrustum.SetPerspective(50.0f, float(w) / float(h), 1.0f, 100.0f);
	}
	else {
		// Definiere das viewing volume (left, right, bottom, top, near, far)
		if (w <= h)
			viewFrustum.SetOrthographic(-nRange, nRange, -nRange * h / w, nRange * h / w, -nRange, nRange);
		else
			viewFrustum.SetOrthographic(-nRange * w / h, nRange * w / h, -nRange, nRange, -nRange, nRange);
	}
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	// Ruecksetzung des Model view matrix stack
	modelViewMatrix.LoadIdentity();

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	TwWindowSize(w, h);
}

void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'");
	TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F, &rotation, "");
	// Add button to check if siepinski is drawn
	TwAddVarRW(bar, "Draw Sierpinski", TW_TYPE_BOOLCPP, &bSierpinski, " label='Draw Sierpinski' ");
	// Add button to check if menge sponge is drawn
	TwAddVarRW(bar, "Draw Menger Sponge", TW_TYPE_BOOLCPP, &bMengerSponge, " label='Draw Menger Sponge' ");
	// Add button to change the depth (max of 3) of the recursion, if the value is changed we need to draw the geometry again
	TwAddVarRW(bar, "Depth", TW_TYPE_INT32, &depth, " label='Depth' min=0 max=10 ");
	// add sensitive slider for the scaling of the geometry
	TwAddVarRW(bar, "Scale", TW_TYPE_FLOAT, &globalScale, " label='Scale' min=0.1 max=10 step=0.1 ");
	// add button to change the perspective
	TwAddVarRW(bar, "Perspective", TW_TYPE_BOOLCPP, &bPerspective, " label='Perspective?' ");
	// add button for UfO mode
	TwAddVarRW(bar, "UFO", TW_TYPE_BOOLCPP, &bUFO, " label='Animation in UFO Mode?' ");
}

void CreateGeometry()
{
	createTetrahedron(pyramidBatch);
	//createTetrahedronWithVBOVBA(VAOtetra, indexBufferTetra, vBufferIdTetra);
	createCube(cubeBatch);
}

// Aufruf draw scene
void RenderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	modelViewMatrix.PushMatrix();
	modelViewMatrix.LoadIdentity();

	modelViewMatrix.Scale(globalScale, globalScale, globalScale);
	if (bUFO) {
		//cameraFrame.MoveForward(rotation.y);
		//cameraFrame.MoveUp(rotation.x);
		//cameraFrame.MoveRight(rotation.w);
		quaternionToOrientationAndPosition(glm::quat(rotation.z, rotation.w, rotation.x, rotation.y), cameraFrame);
		// rotate the camera frame
		M3DMatrix44f M;
		cameraFrame.GetMatrix(M);
		modelViewMatrix.MultMatrix(M);
	}
	else {
		glm::mat4 rot = glm::mat4_cast(glm::quat(rotation.z, rotation.w, rotation.x, rotation.y));
		modelViewMatrix.MultMatrix(glm::value_ptr(rot));
	}
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
		// rotate with a very small angle
		modelViewMatrix.Rotate(100*currentAnimationAngle, 0.0f, 1.0f, 0.0f);
		// ModelViewMatrix in einer Kreisbahn bewegen
		modelViewMatrix.Translate(2*sin(currentAnimationAngle), 0.0f, 2*cos(currentAnimationAngle));
		currentAnimationAngle += 0.0002f;

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
		// size animation
		modelViewMatrix.Scale(0.25*sin(currentAnimationAngle)+0.5, 0.25*sin(currentAnimationAngle)+0.5, 0.25*sin(currentAnimationAngle)+0.5);
		//currentAnimationAngle += 0.0002f;
		drawMenger(depth);
		DrawCoordinateSystem(10.0f);
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
	glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
	glFrontFace(GL_CW); // Winding Order könnte noch umgedreht werden, dafür müsste ich aber alle Indices umdrehen...
	// Backface Culling aktivieren
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT); // 
	glEnable(GL_DEPTH_TEST);

	//initialisiert die standard shader
	shaderManager.InitializeStockShaders();
	//Matrix stacks für die Transformationspipeline setzen, damit werden dann automatisch die Matrizen multipliziert
	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);

	//erzeuge die geometrie
	CreateGeometry();
	InitGUI();

	cameraFrame.SetOrigin(0.0f, 0.0f, 0.0f);
	cameraFrame.SetForwardVector(0.0f, 0.0f, 1.0f);
	cameraFrame.SetUpVector(0.0f, 1.0f, 0.0f);
}

void SpecialKeys(int key, int x, int y)
{
	float angleStep = 0.1f;  // Rotationswinkel in Grad
	float moveStep = 0.5f;  // Bewegungsschritt in Einheiten

	switch (key)
	{
	case GLUT_KEY_UP: // 101
		// Bewegt die Kamera nach vorne entlang der lokalen Z-Achse
		cameraFrame.MoveUp(moveStep);
		cameraFrame.RotateLocalX(-angleStep);
		break;
	case GLUT_KEY_DOWN: // 103
		// Bewegt die Kamera nach hinten entlang der lokalen Z-Achse
		cameraFrame.MoveUp(-moveStep);
		cameraFrame.RotateLocalX(angleStep);
		break;
	case GLUT_KEY_LEFT: // 100
		// Bewegt die Kamera nach links entlang der lokalen X-Achse
		cameraFrame.MoveRight(moveStep*0.5);
		cameraFrame.RotateLocalY(angleStep);
		break;
	case GLUT_KEY_RIGHT: // 102
		// Bewegt die Kamera nach rechts entlang der lokalen X-Achse
		cameraFrame.MoveRight(-moveStep*0.5);
		cameraFrame.RotateLocalY(-angleStep);
		break;
	case GLUT_KEY_PAGE_UP: // Zusätzliche Taste für Aufwärtsbewegung
		// Bewegt die Kamera nach oben entlang der lokalen Y-Achse
		cameraFrame.MoveUp(moveStep);
		break;
	case GLUT_KEY_PAGE_DOWN: // Zusätzliche Taste für Abwärtsbewegung
		// Bewegt die Kamera nach unten entlang der lokalen Y-Achse
		cameraFrame.MoveUp(-moveStep);
		break;
	default:
		break;
	}

	// AntTweakBar Event
	TwEventSpecialGLUT(key, x, y);

	// Fenster neu zeichnen
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	if (bUFO) {
		GLfloat nXDelta = x - 500;
		GLfloat nYDelta = 500 - y;
		// Rotation um die y-Achse
		GLfloat yRot = nXDelta / 100.0f;
		// Rotation um die x-Achse
		GLfloat xRot = nYDelta / 100.0f;
		cameraFrame.RotateWorld(yRot, 0.0f, 1.0f, 0.0f);
		cameraFrame.RotateLocal(xRot, 1.0f, 0.0f, 0.0f);
	}

	// Fenster neu zeichnen
	glutPostRedisplay();
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
