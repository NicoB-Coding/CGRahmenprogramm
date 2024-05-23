// Ausgangssoftware des 3. Praktikumsversuchs 
// zur Vorlesung Echtzeit-3D-Computergrahpik
// von Prof. Dr. Alfred Nischwitz
// Programm umgesetzt mit der GLTools Library

#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif 
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <ObjLoader/Model.h>
#include <ObjLoader/Triangle.h>
#include <ObjLoader/Vertex.h>
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLFrustum.h>
#include <math.h> 
#include <math3d.h>
#include <GL/freeglut.h>
#include <AntTweakBar.h>

/// View space light position
float light_pos[4] = {5.0f,5.0f,0.0f,1.0f} ;
/// Lichtfarben
float light_diffuse[4] = {1.0f,1.0f,1.0f,1.0f} ;
float light_specular[4] = {1.0f,1.0f,1.0f,1.0f} ;

float emissive_color[] = {0,0,0,0};
float ambient_color[] = {0.1f,0.1f,0.1f,0};
/// Materialfarben
float mat_diffuse[] =  {1.0f,0,0};
float mat_specular[] = {1.0f,1.0f,1.0f,1.0f};
float specular_power = 10;


GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager shaderManager;
GLFrustum viewFrustum;
//Ein Pointer ist hier notwendig, da Sie nach jeder Normalenberechnung
//neue Werte in die Batch schreiben müssen.
GLBatch* modelBatch = NULL;
obj::Model model;
//Dateiname für das Modell.
const std::string modelFile = "../Modelle/cylinder.obj";
GLuint shaders;

glm::quat rotation = glm::quat(0, 0, 0, 1);

TwBar *bar;

float grenzWinkel = 0.0f;


void TW_CALL angleSetCallback(const void * value, void * clientData)
{
	grenzWinkel = *(const float*)value;
	//Wird jedes mal aufgerufen, wenn der Winkel über die GUI geändert wird
	//Hier kann z.b. die Normalenrechnung aufgerufen werden
	
}
void TW_CALL angleGetCallback( void * value, void * clientData)
{
	*(float*)value = grenzWinkel;
}
void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'"); 
	TwAddVarCB(bar,"Grenzwinkel", TW_TYPE_FLOAT,angleSetCallback, angleGetCallback,NULL, "min=0 max=180");
	TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F,&rotation,"");
    TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &light_pos, "group='Light' axisx=-x axisy=-y axisz=-z");
}
void CreateGeometry()
{
	//Modell laden und Batch erzeugen.
	model.Load(modelFile);
	
	modelBatch = new GLBatch();
	modelBatch->Begin(GL_TRIANGLES,model.GetTriangleCount()*3);
	for (unsigned int i =0; i < model.GetTriangleCount();++i)
	{
		obj::Triangle* tr = model.GetTriangle(i);
		//Hier werden die Normalen nur als Beispiel aus Obj-Loader verwendet.
		//Beim Zylinder sind sie allerdings bewusst falsch. Daher müssen Sie 
		//die Normalen anhand der Dreiecke selbst berechnen. Die Normalen
		//müssen immer vor jedem Vertex gesetzt werden!

		// Idee: Suche für jeden Vertex alle angrenzenden Dreiecke und berechne die Normalen
		// Je nach Grenzwinkel zwischen den Normalen wird dann gemittelt oder nicht
		modelBatch->Normal3f(tr->vertex[0].normal.x,tr->vertex[0].normal.y,tr->vertex[0].normal.z);
		modelBatch->Vertex3f(tr->vertex[0].pos.x,tr->vertex[0].pos.y,tr->vertex[0].pos.z);

		modelBatch->Normal3f(tr->vertex[1].normal.x,tr->vertex[1].normal.y,tr->vertex[1].normal.z);
		modelBatch->Vertex3f(tr->vertex[1].pos.x,tr->vertex[1].pos.y,tr->vertex[1].pos.z);

		modelBatch->Normal3f(tr->vertex[2].normal.x,tr->vertex[2].normal.y,tr->vertex[2].normal.z);
		modelBatch->Vertex3f(tr->vertex[2].pos.x,tr->vertex[2].pos.y,tr->vertex[2].pos.z);
	}
	modelBatch->End();
	//Shader Programme laden.  Die letzen Argumente geben die Shader-Attribute an. Hier wird Vertex und Normale gebraucht.
	shaders =  gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_NORMAL, "vNormal");

	gltCheckErrors(shaders);
	InitGUI();
}


// Aufruf draw scene
void RenderScene(void)
{
	
	// Clearbefehle für den color buffer und den depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);


	// Speichere den matrix state und führe die Rotation durch
	modelViewMatrix.PushMatrix();

	// Rotiere Objekt
	glm::mat4 rot = glm::mat4_cast(glm::quat(rotation.z, rotation.w, rotation.x, rotation.y));
	modelViewMatrix.MultMatrix(glm::value_ptr(rot));

	//setze den Shader für das Rendern
	glUseProgram(shaders);
	// Model View Projection Matrix setzen
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvMatrix"),  1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(glGetUniformLocation(shaders, "normalMatrix"),  1, GL_FALSE, transformPipeline.GetNormalMatrix(true));
	// Lichteigenschaften übergeben
	glUniform4fv(glGetUniformLocation(shaders, "light_pos_vs"),1,light_pos);
	glUniform4fv(glGetUniformLocation(shaders, "light_diffuse"),1,light_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "light_specular"),1,light_specular);
	glUniform1f(glGetUniformLocation(shaders, "spec_power"),specular_power);
	
	glUniform4fv(glGetUniformLocation(shaders, "emissive_color"),1,emissive_color);
	glUniform4fv(glGetUniformLocation(shaders, "ambient_color"),1,ambient_color);
	glUniform4fv(glGetUniformLocation(shaders, "mat_diffuse"),1,mat_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "mat_specular"),1,mat_specular);

	//Modell zeichnen
	if(modelBatch)
		modelBatch->Draw();
	
	// Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
	modelViewMatrix.PopMatrix();
	gltCheckErrors(shaders);
	// Draw tweak bars
	TwDraw();
	// Vertausche Front- und Backbuffer
	glutSwapBuffers();
	glutPostRedisplay();
}

// Initialisierung des Rendering Kontextes
void SetupRC()
{
	// Schwarzer Hintergrund
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f );
	// In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
	// Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans benützen
	glFrontFace(GL_CCW);
	shaderManager.InitializeStockShaders();
	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);
	//erzeuge die geometrie
	CreateGeometry();
}

void ShutDownRC()
{
	delete modelBatch;
	glDeleteProgram(shaders);
	TwTerminate();
	
}

void ChangeSize(int w, int h)
{
	//Model ist normalisiert im Ursprung
	GLfloat nRange = 1.5f;

	// Verhindere eine Division durch Null
	if(h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();

	viewFrustum.SetPerspective(90.0f, w / (float) h, 0.1f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	// Ruecksetzung des Model view matrix stack
	modelViewMatrix.LoadIdentity();
	modelViewMatrix.Translate(0,0,-2);
	// Send the new window size to AntTweakBar
	TwWindowSize(w, h);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,600);
	glutCreateWindow("Aufgabe3 - Normalenberechnung");
	glutCloseFunc(ShutDownRC);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Veralteter Treiber etc.
		std::cerr <<"Error: "<< glewGetErrorString(err) << "\n";
		return 1;
	}

	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); 
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	glutSpecialFunc((GLUTspecialfun)TwEventKeyboardGLUT);

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	TwInit(TW_OPENGL_CORE, NULL);
	SetupRC();

	glutMainLoop();

	return 0;
}
