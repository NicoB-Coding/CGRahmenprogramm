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

GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;

GLuint subDivideShader;
GLuint renderShader;
GLuint vaoGeo[2];
GLuint geo[2];
GLuint query;
GLuint primitives_written;
int current_buffer;

// Rotationsgroessen
glm::quat rotation = glm::quat(0, 0, 0, 1);

struct ColoredVertex
{
	glm::vec4 position;
	glm::vec4 color;
};

TwBar *bar;
void Subdivide();

void TW_CALL subdivideCallback(void* data)
{
	Subdivide();
}

void InitGUI()
{
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 200'");
	TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F,&rotation,"");
	TwAddVarRO(bar,"Primitives",TW_TYPE_UINT32,&primitives_written,"");
	TwAddButton(bar,"Subdivide",subdivideCallback,NULL,"");
	
}

void Subdivide()
{
	//Hier dann transform feedback ausführen
	//aktuell sieht man natürlich nichts, da kein fragment shader gesetzt ist.
	glUseProgram(subDivideShader);
	glBindVertexArray(vaoGeo[current_buffer]);
	glDrawArrays(GL_TRIANGLES, 0, primitives_written*3);
	glBindVertexArray(0);
	glUseProgram(0);
	//switch the buffer
	current_buffer = 1 - current_buffer;
}

void CreateGeometry()
{
	//Hier dann die eigentliche Primitive angeben

	ColoredVertex v[3];

	v[0].position = glm::vec4(0, 1, 0, 1);
	v[1].position = glm::vec4(-1, 0, 0, 1);
	v[2].position = glm::vec4(1, 0, 0, 1);

	v[0].color = glm::vec4(0, 1, 0, 1);
	v[1].color = glm::vec4(0, 1, 0, 1);
	v[2].color = glm::vec4(0, 1, 0, 1);

	//1 Dreieck wird gezeichnet. Diese Variable steuert die Anzahl der Dreiecke beim Rendern
	//Durch eine query nach Transform Feedback dann entsprechend aktualisieren
	primitives_written=1;

	int max_buffer_size = 1<<20;
	current_buffer=0;
	// Erzeuge das VBO
	glGenBuffers(2, geo);
	glBindBuffer(GL_ARRAY_BUFFER, geo[0]);
	//Buffer größe festlegen
	glBufferData(GL_ARRAY_BUFFER,max_buffer_size* sizeof(ColoredVertex), 0, GL_STATIC_DRAW);
	//Kopiere die 3 vertices ins VBO
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(ColoredVertex)*3, v);
	glBindBuffer(GL_ARRAY_BUFFER, geo[1]);
	glBufferData(GL_ARRAY_BUFFER, max_buffer_size*sizeof(ColoredVertex),0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(2,vaoGeo);
	glBindVertexArray(vaoGeo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, geo[0]);
	// erstes Shader-Attribut aktivieren (Vertex-Position)
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,  sizeof(ColoredVertex),	NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,  sizeof(ColoredVertex),	(const GLvoid *)(sizeof(glm::vec4)));
	//VAO wieder unbinden
	glBindVertexArray(0);
	glBindVertexArray(vaoGeo[1]);
	glBindBuffer(GL_ARRAY_BUFFER, geo[1]);
	// erstes Shader-Attribut aktivieren (Vertex-Position)
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,  sizeof(ColoredVertex),	NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,  sizeof(ColoredVertex),	(const GLvoid *)(sizeof(glm::vec4)));
	//VAO wieder unbinden
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	

	//Shader Programme laden
	//postionOut und colorOut geben die Namen der Ausgaben im Geometry Shader (glTransformFeedbackVaryings)
	//diese müssen vor dem linken des Shaders gesetzt werden. Es werden Interleaved Attribute verwendet.
	//Einen Fragment-Shader braucht man hier nicht, da die Dreiecke direkt nach dem Geometry Shader ausgelesen werden.
	subDivideShader =  gltLoadShaderPairWithAttributesTransformFeedback("SubdivideVertexShader.glsl", "SubdivideGeometryShader.glsl",2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_COLOR, "vColor",
		2, "positionOut", "colorOut", GL_INTERLEAVED_ATTRIBS);

	gltCheckErrors(subDivideShader);
	renderShader =  gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl",2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_COLOR, "vColor");

	gltCheckErrors(renderShader);
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
	glm::mat4 rot = glm::mat4_cast(rotation);
    modelViewMatrix.MultMatrix(glm::value_ptr(rot));

	//setze den Shader für das Rendern
	glUseProgram(renderShader);
	// Model View Projection Matrix setzen
	glUniformMatrix4fv(glGetUniformLocation(renderShader, "mvpMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());

	glBindVertexArray(vaoGeo[current_buffer]);
	glDrawArrays(GL_TRIANGLES, 0, primitives_written*3);
	glBindVertexArray(0);
	glUseProgram(0);
	// Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
	modelViewMatrix.PopMatrix();
	gltCheckErrors(renderShader);

	TwDraw();

	// Vertausche Front- und Backbuffer
	glutSwapBuffers();
	glutPostRedisplay();
}

// Initialisierung des Rendering Kontextes
void SetupRC()
{
	// Schwarzer Hintergrund
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	// In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
	// Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans benützen
	glFrontFace(GL_CCW);

	transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);
	//erzeuge die geometrie
	CreateGeometry();
	InitGUI();
}

void ShutDownRC()
{
	glDeleteProgram(renderShader);
	glDeleteProgram(subDivideShader);

	TwTerminate();
}


void ChangeSize(int w, int h)
{
	//Model ist normalisiert im Ursprung
	GLfloat nRange = 2.0f;

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
	TwWindowSize(w,h);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,600);
	glutCreateWindow("Aufgabe5 - GeometryShader");
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
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	TwInit(TW_OPENGL_CORE, NULL);
	SetupRC();

	glutMainLoop();

	return 0;
}
