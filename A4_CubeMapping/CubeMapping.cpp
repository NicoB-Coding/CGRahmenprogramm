// Ausgangssoftware des 4. Praktikumsversuchs 
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
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLFrustum.h>
#include <math.h>
#include <math3d.h>
#include <GL/freeglut.h>
#include <ImageLoader/ImageLoader.h>
#include <AntTweakBar.h>

GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;

GLuint shaders;
GLTriangleBatch sphereBig;
GLTriangleBatch sphereSmall;


//Textur Id für die Cube-Map
GLuint cubeMapTex = 0 ;
//Dateinamen für die Cube-Map
const std::string cubeMapNames[] = {
	"../Texturen/cubemap/positive_x.jpg",
	"../Texturen/cubemap/negative_x.jpg",
	"../Texturen/cubemap/positive_y.jpg",
	"../Texturen/cubemap/negative_y.jpg",
	"../Texturen/cubemap/positive_z.jpg",
	"../Texturen/cubemap/negative_z.jpg"
} ;


// Rotationsgroessen
glm::quat rotation = glm::quat(0, 0, 0, 1);

//GUI
TwBar *bar;
void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'");
	TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F, &rotation, "");
	//Hier weitere GUI Variablen anlegen. Für Farbe z.B. den Typ TW_TYPE_COLOR4F benutzen
}

void CreateGeometry()
{
	gltMakeSphere(sphereBig,0.4f,30,30);
	gltMakeSphere(sphereSmall,0.2f,30,30);

	//Shader Programme laden. Die letzen Argumente geben die Shader-Attribute an. Hier wird Vertex und Normale gebraucht.
	//Fürs Texture Mapping benötigt man als drittes Vertex-Attribut die Texturkoordinaten, die man z.B. über die Erweiterung 
	//GLT_ATTRIBUTE_TEXTURE0, "vTexCoord" definieren kann. Achtung: da in diesem Fall 3 statt 2 Vertex-Attribute übergeben 
	//werden, muss der dritte Parameter in der Methode gltLoadShaderPairWithAttributes von 2 auf 3 hochgesetzt werden.
     shaders =  gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl", 2,
         GLT_ATTRIBUTE_VERTEX, "vVertex",
         GLT_ATTRIBUTE_NORMAL, "vNormal");

	gltCheckErrors(shaders);
}

// Aufruf draw scene
void RenderScene(void)
{
	// Clearbefehle für den color buffer und den depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	
	// Speichere den matrix state und führe die Rotation durch
	modelViewMatrix.PushMatrix();
	glm::mat4 rot = glm::mat4_cast(glm::quat(rotation.z, rotation.w, rotation.x, rotation.y));
	modelViewMatrix.MultMatrix(glm::value_ptr(rot));

	// Aktivieren der automatischen Texturkoordinaten-Generierung
	static GLfloat s[] = { 0.1f, 0.0f, 0.0f, 0.0f };
	static GLfloat t[] = { 0.0f, 0.1f, 0.0f, 0.0f };
	static GLfloat r[] = { 0.0f, 0.0f, 0.1f, 0.0f };
	
	glUniform4fv(glGetUniformLocation(shaders, "ObjectPlaneS"), 1, s);
	glUniform4fv(glGetUniformLocation(shaders, "ObjectPlaneT"), 1, t);
	glUniform4fv(glGetUniformLocation(shaders, "ObjectPlaneR"), 1, r);

	glBindTexture(GL_TEXTURE_CUBE_MAP,cubeMapTex);
	//setze den Shader für das Rendern
	glUseProgram(shaders);


	// Model View Projection Matrix setzen
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());	
	//Zeichne Model
	sphereBig.Draw();
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(0.6f,0.6f,0.6f);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	sphereSmall.Draw();
	modelViewMatrix.PopMatrix();

	// Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
	modelViewMatrix.PopMatrix();
	gltCheckErrors(shaders);
	
	TwDraw();
	// Vertausche Front- und Backbuffer
	glutSwapBuffers();
	glutPostRedisplay();
}

// Initialisierung des Rendering Kontextes
void SetupRC()
{
	// Schwarzer Hintergrund
	glClearColor( 0.12f,0.35f,0.674f,0.0f ) ;

	// In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
	// Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans benützen
	glFrontFace(GL_CW);

	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);
	//erzeuge die geometrie
	CreateGeometry();
	InitGUI();

	//Erzeuge Textur für cube map
	glGenTextures(1, &cubeMapTex);
	img::ImageLoader imgLoader;	
	//Aktive Textur setzen
	glBindTexture(GL_TEXTURE_CUBE_MAP,cubeMapTex );
	for(int i=0; i<6; ++i )
	{
		int width,height;
		//Textur einlesen. Bei JPEG bildern das topdown flag auf true setzen, sonst stehen die Bilder auf den Kopf.
		unsigned char* data = imgLoader.LoadTextureFromFile(cubeMapNames[i],&width,&height, true);
		//Textur hochladen, bei JPEG bildern muss GL_BGR verwendet werden
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, width, height,
			0, GL_BGR, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}
	//Zugriffsflags setzen, wichtig!
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) ;
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) ;
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE ) ;
} 

void ShutDownRC()
{
	//Aufräumen
	glDeleteProgram(shaders);
	glDeleteTextures(1,&cubeMapTex);

	TwTerminate();
}

void SpecialKeys(int key, int x, int y)
{
	TwEventKeyboardGLUT(key, x, y);
	// Zeichne das Window neu
	glutPostRedisplay();
}


void ChangeSize(int w, int h)
{
	// Verhindere eine Division durch Null
	if(h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();
	viewFrustum.SetPerspective(45,w/(float)h,1,100);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	// Ruecksetzung des Model view matrix stack
	modelViewMatrix.LoadIdentity();
	modelViewMatrix.Translate(0,0,-3);
	
	TwWindowSize(w, h);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,600);
	glutCreateWindow("Aufgabe 4 - Cube Mapping");
	glutCloseFunc(ShutDownRC);
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		//Veralteter Treiber etc.
		std::cerr <<"Error: "<< glewGetErrorString(err) << "\n";
		return 1;
	}
	
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
