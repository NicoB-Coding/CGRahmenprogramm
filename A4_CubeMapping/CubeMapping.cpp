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
GLuint skyboxShader;
GLTriangleBatch sphereBig;
GLTriangleBatch sphereSmall;
GLBatch skyboxBatch;

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
	gltMakeCube(skyboxBatch, 4.0f);

	//Shader Programme laden. Die letzen Argumente geben die Shader-Attribute an. Hier wird Vertex und Normale gebraucht.
	//Fürs Texture Mapping benötigt man als drittes Vertex-Attribut die Texturkoordinaten, die man z.B. über die Erweiterung 
	//GLT_ATTRIBUTE_TEXTURE0, "vTexCoord" definieren kann. Achtung: da in diesem Fall 3 statt 2 Vertex-Attribute übergeben 
	//werden, muss der dritte Parameter in der Methode gltLoadShaderPairWithAttributes von 2 auf 3 hochgesetzt werden.
     shaders =  gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl", 2,
         GLT_ATTRIBUTE_VERTEX, "vVertex",
         GLT_ATTRIBUTE_NORMAL, "vNormal");
	 skyboxShader = gltLoadShaderPairWithAttributes("SkyboxVertexShader.glsl", "SkyboxFragmentShader.glsl", 1,
		 GLT_ATTRIBUTE_VERTEX, "vVertex");

	gltCheckErrors(shaders);
	gltCheckErrors(skyboxShader);
}
void CheckShaderCompileError(GLuint shader) {
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
	}
}

void CheckGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	while (err != GL_NO_ERROR)
	{
		const char* error;
		switch (err)
		{
		case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		default:                        error = "Unknown Error";          break;
		}
		printf("OpenGL error [%s] (%s:%d): %s\n", error, fname, line, stmt);
		err = glGetError();
	}
}

#define GL_CHECK(stmt) do { \
        stmt; \
        CheckGLError(#stmt, __FILE__, __LINE__); \
    } while (0)


glm::mat4 ConvertToMat4(const M3DMatrix44f m)
{
	return glm::mat4(
		m[0], m[1], m[2], m[3],
		m[4], m[5], m[6], m[7],
		m[8], m[9], m[10], m[11],
		m[12], m[13], m[14], m[15]
	);
}

// Aufruf draw scene
void RenderScene(void)
{
	// Clearbefehle für den color buffer und den depth buffer
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	GL_CHECK(glEnable(GL_DEPTH_TEST));

	// Aktivieren der automatischen Texturkoordinaten-Generierung
	static GLfloat s[] = { 0.1f, 0.0f, 0.0f, 0.0f };
	static GLfloat t[] = { 0.0f, 0.1f, 0.0f, 0.0f };
	static GLfloat r[] = { 0.0f, 0.0f, 0.1f, 0.0f };

	GL_CHECK(glUseProgram(skyboxShader));
	// Speichere den matrix state und führe die Rotation durch
	modelViewMatrix.PushMatrix();
	glm::mat4 rot = glm::mat4_cast(glm::quat(rotation.z, rotation.w, rotation.x, rotation.y));
	// Model View Projection Matrix setzen
	modelViewMatrix.PushMatrix();

	modelViewMatrix.MultMatrix(glm::value_ptr(rot));
	glm::mat4 mvpMatrix = ConvertToMat4(transformPipeline.GetModelViewProjectionMatrix());
	GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix)));
	GL_CHECK(glUniform4fv(glGetUniformLocation(skyboxShader, "ObjectPlaneS"), 1, s));
	GL_CHECK(glUniform4fv(glGetUniformLocation(skyboxShader, "ObjectPlaneT"), 1, t));
	GL_CHECK(glUniform4fv(glGetUniformLocation(skyboxShader, "ObjectPlaneR"), 1, r));

	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex));
	skyboxBatch.Draw();

	modelViewMatrix.PopMatrix();

	// Set up transformation for the objects
	GL_CHECK(glUseProgram(shaders));
	modelViewMatrix.MultMatrix(glm::value_ptr(rot));

	glm::mat4 mvM = ConvertToMat4(transformPipeline.GetModelViewMatrix());
	glm::mat4 projectionMatrix = ConvertToMat4(transformPipeline.GetProjectionMatrix());
	mvpMatrix = ConvertToMat4(transformPipeline.GetModelViewProjectionMatrix());

	glm::mat3 invRotMatrix = glm::transpose(glm::mat3(mvM));
	glm::mat4 textureMatrix = glm::mat4(1.0f); // Einheitsmatrix
	textureMatrix = glm::mat4(
		invRotMatrix[0][0], invRotMatrix[0][1], invRotMatrix[0][2], 0.0f,
		invRotMatrix[1][0], invRotMatrix[1][1], invRotMatrix[1][2], 0.0f,
		invRotMatrix[2][0], invRotMatrix[2][1], invRotMatrix[2][2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Extrahiere die oberen 3x3 Komponenten der modelViewMatrix
	glm::mat3 normalMatrix = glm::mat3(
		mvM[0][0], mvM[0][1], mvM[0][2],
		mvM[1][0], mvM[1][1], mvM[1][2],
		mvM[2][0], mvM[2][1], mvM[2][2]
	);
	normalMatrix = glm::transpose(glm::inverse(normalMatrix));

	// Set uniform values for matrices
	GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shaders, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(mvM)));
	GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shaders, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix)));
	GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix)));
	GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shaders, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix)));

	// Übergebe die Texturmatrix als Uniform an den Shader
	GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shaders, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrix)));

	GL_CHECK(glUniform4fv(glGetUniformLocation(shaders, "ObjectPlaneS"), 1, s));
	GL_CHECK(glUniform4fv(glGetUniformLocation(shaders, "ObjectPlaneT"), 1, t));
	GL_CHECK(glUniform4fv(glGetUniformLocation(shaders, "ObjectPlaneR"), 1, r));

	// Model View Projection Matrix setzen
	// Zeichne Model
	sphereBig.Draw();
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(0.6f, 0.6f, 0.6f);
	GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix()));
	sphereSmall.Draw();
	modelViewMatrix.PopMatrix();

	// Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
	modelViewMatrix.PopMatrix();
	gltCheckErrors(shaders);

	TwDraw();
	// Vertausche Front- und Backbuffer
	GL_CHECK(glutSwapBuffers());
	GL_CHECK(glutPostRedisplay());
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
	glDeleteProgram(skyboxShader);
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
