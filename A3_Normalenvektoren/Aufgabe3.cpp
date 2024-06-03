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
GLBatch modelBatch;
GLBatch normalBatch;
obj::Model model;
//Dateiname für das Modell.
const std::string modelFile = "../Modelle/cylinder.obj";
GLuint shaders;

glm::quat rotation = glm::quat(0, 0, 0, 1);

TwBar *bar;

float grenzWinkel = 45.0f;
float previousGrenzWinkel = 45.0f;
bool bInfiniteLight = true;
bool bSmoothShading = true;
bool bShowNormals = false;
bool bPreviousSmoothShading = true;
bool bPhongShading = false;
bool bPreviousPhongShading = false;

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
	TwAddVarRW(bar, "Infinite Light?", TW_TYPE_BOOLCPP, &bInfiniteLight,"");
	TwAddVarRW(bar, "Smooth Shading?", TW_TYPE_BOOLCPP, &bSmoothShading,"");
	TwAddVarRW(bar, "Phong Shading?", TW_TYPE_BOOLCPP, &bPhongShading, "");
	TwAddVarRW(bar, "Show Normals?", TW_TYPE_BOOLCPP, &bShowNormals,"");
	// RGBA Slider
	TwAddVarRW(bar, "Diffuse", TW_TYPE_COLOR4F, &light_diffuse, "group='Light'");
	TwAddVarRW(bar, "Specular", TW_TYPE_COLOR4F, &light_specular, "group='Light'");
	TwAddVarRW(bar, "Shininess", TW_TYPE_FLOAT, &specular_power, "group='Light' min=0");
	TwAddVarRW(bar, "Emissive", TW_TYPE_COLOR4F, &emissive_color, "group='Light'");
	TwAddVarRW(bar, "Ambient", TW_TYPE_COLOR4F, &ambient_color, "group='Light'");
}
void CreateGeometry()
{
	//Modell laden und Batch erzeugen.
	model.Load(modelFile);
	normalBatch.Reset();
	modelBatch.Reset();
	modelBatch = GLBatch();
	normalBatch = GLBatch();
	
	modelBatch.Begin(GL_TRIANGLES,model.GetTriangleCount()*3);
	std::vector<std::vector<glm::vec3>> normalsToDraw;
	std::vector <glm::vec3> batchNormals;
	for (unsigned int i =0; i < model.GetTriangleCount();++i)
	{
		normalsToDraw.clear();
		normalsToDraw.resize(3);
		obj::Triangle* tr = model.GetTriangle(i);
		std::vector<glm::vec3> normals; // Liste der einzigartigen normalen
		for (int point = 0; point < 3; point++) {
			normals.clear();
			// Kreuzprodukt der Kanten
			glm::vec3 edge1 = glm::vec3(tr->vertex[1].pos.x - tr->vertex[0].pos.x, tr->vertex[1].pos.y - tr->vertex[0].pos.y, tr->vertex[1].pos.z - tr->vertex[0].pos.z);
			glm::vec3 edge2 = glm::vec3(tr->vertex[2].pos.x - tr->vertex[0].pos.x, tr->vertex[2].pos.y - tr->vertex[0].pos.y, tr->vertex[2].pos.z - tr->vertex[0].pos.z);
			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
			normals.push_back(normal);

			obj::Model::PrimitiveCollection adjacentTriangles;

			obj::Vertex* v = &tr->vertex[point];
			model.GetAdjacentTriangles(adjacentTriangles, v);

			// suche die anliegenden Normalen heraus
			for (unsigned int j = 0; j < adjacentTriangles.size(); j++)
			{
				obj::Triangle* adjTr = adjacentTriangles[j];
				// Kreuzprodukt der Kanten
				glm::vec3 edge1 = glm::vec3(adjTr->vertex[1].pos.x - adjTr->vertex[0].pos.x, adjTr->vertex[1].pos.y - adjTr->vertex[0].pos.y, adjTr->vertex[1].pos.z - adjTr->vertex[0].pos.z);
				glm::vec3 edge2 = glm::vec3(adjTr->vertex[2].pos.x - adjTr->vertex[0].pos.x, adjTr->vertex[2].pos.y - adjTr->vertex[0].pos.y, adjTr->vertex[2].pos.z - adjTr->vertex[0].pos.z);
				glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
				bool found = false;
				for (unsigned int k = 0; k < normals.size(); k++)
				{
					float angle = glm::degrees(glm::acos(glm::dot(normals[k], normal)));
					if (angle < 0.1f)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					normals.push_back(normal);
				}
			}

			std::vector<bool> used(normals.size(), false); // Verfolgung der verwendeten Normalen
			std::vector<glm::vec3> normalsToAverage;

			// Durchlaufe alle Normalen und finde diejenigen, die sich nur um den Grenzwinkel unterscheiden
			for (int i = 0; i < normals.size(); i++) {
				if (used[i]) continue;
				normalsToAverage.clear();
				normalsToAverage.push_back(normals[i]);
				used[i] = true;

				for (int j = i + 1; j < normals.size(); j++) {
					if (used[j]) continue;
					float angle = glm::degrees(glm::acos(glm::dot(normals[i], normals[j])));
					if (angle < grenzWinkel) {
						normalsToAverage.push_back(normals[j]);
						used[j] = true;
					}
				}

				glm::vec3 averagedNormal = glm::vec3(0, 0, 0);
				for (const auto& n : normalsToAverage) {
					averagedNormal += n;
				}
				averagedNormal = glm::normalize(averagedNormal);

				normalsToDraw[point].push_back(averagedNormal);
			}

			// Gehe sicher, dass alle Normalen verwendet werden
			for (int i = 0; i < normals.size(); i++) {
				if (!used[i]) {
					normalsToDraw[point].push_back(normals[i]);
				}
			}

		}

		// durchlaufe normalsToDraw und füge die normalen in den Batch ein
		for (int point = 0; point < 3; point++) {
			for (int i = 0; i < normalsToDraw[point].size(); i++) {
				batchNormals.push_back(glm::vec3(tr->vertex[point].pos.x, tr->vertex[point].pos.y, tr->vertex[point].pos.z));
				batchNormals.push_back(glm::vec3(tr->vertex[point].pos.x + 0.1f * normalsToDraw[point][i].x, tr->vertex[point].pos.y + 0.1f * normalsToDraw[point][i].y, tr->vertex[point].pos.z + 0.1f * normalsToDraw[point][i].z));
			}
		}
		if (bSmoothShading) {
			for (int point = 0; point < 3; point++) {
				modelBatch.Normal3f(normalsToDraw[point][0].x, normalsToDraw[point][0].y, normalsToDraw[point][0].z);
				modelBatch.Vertex3f(tr->vertex[point].pos.x,tr->vertex[point].pos.y,tr->vertex[point].pos.z);
			}
		}
		else {
			// eine Normale für alle 3 Vertices
			glm::vec3 edge1 = glm::vec3(tr->vertex[1].pos.x, tr->vertex[1].pos.y, tr->vertex[1].pos.z) - glm::vec3(tr->vertex[0].pos.x, tr->vertex[0].pos.y, tr->vertex[0].pos.z);
			glm::vec3 edge2 = glm::vec3(tr->vertex[2].pos.x, tr->vertex[2].pos.y, tr->vertex[2].pos.z) - glm::vec3(tr->vertex[0].pos.x, tr->vertex[0].pos.y, tr->vertex[0].pos.z);
			glm::vec3 normal = normalize(cross(edge1, edge2));

			// Setze die Normale für alle Vertices des Dreiecks
			modelBatch.Normal3f(normal.x, normal.y, normal.z);
			modelBatch.Vertex3f(tr->vertex[0].pos.x, tr->vertex[0].pos.y, tr->vertex[0].pos.z);
			modelBatch.Normal3f(normal.x, normal.y, normal.z);
			modelBatch.Vertex3f(tr->vertex[1].pos.x, tr->vertex[1].pos.y, tr->vertex[1].pos.z);
			modelBatch.Normal3f(normal.x, normal.y, normal.z);
			modelBatch.Vertex3f(tr->vertex[2].pos.x, tr->vertex[2].pos.y, tr->vertex[2].pos.z);
		}
	}
	normalBatch.Begin(GL_LINES, batchNormals.size());
	for (int i = 0; i < batchNormals.size(); i += 2) {
		normalBatch.Vertex3f(batchNormals[i].x, batchNormals[i].y, batchNormals[i].z);
		normalBatch.Vertex3f(batchNormals[i + 1].x, batchNormals[i + 1].y, batchNormals[i + 1].z);
	}
	modelBatch.End();
	normalBatch.End();
	//Shader Programme laden.  Die letzen Argumente geben die Shader-Attribute an. Hier wird Vertex und Normale gebraucht.
	if(bPhongShading && bSmoothShading)
		shaders =  gltLoadShaderPairWithAttributes("PhongVertexShader.glsl", "PhongFragmentShader.glsl", 2, 
					GLT_ATTRIBUTE_VERTEX, "vVertex", 
					GLT_ATTRIBUTE_NORMAL, "vNormal");
	else {
		shaders = gltLoadShaderPairWithAttributes("VertexShader.glsl", "FragmentShader.glsl", 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal");
	}

	if (shaders == 0) {
		std::cerr << "Fehler beim Laden der Shader" << std::endl;
		exit(1);
	}
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
	if (!bInfiniteLight)
	{
		glUniform4fv(glGetUniformLocation(shaders, "light_pos_vs"),1,light_pos);
	}
	else
	{
		float infinite_light[4] = {light_pos[0],light_pos[1],light_pos[2],0};
		glUniform4fv(glGetUniformLocation(shaders, "light_pos_vs"),1,infinite_light);
	}
	glUniform4fv(glGetUniformLocation(shaders, "light_diffuse"),1,light_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "light_specular"),1,light_specular);
	glUniform1f(glGetUniformLocation(shaders, "spec_power"),specular_power);
	
	glUniform4fv(glGetUniformLocation(shaders, "emissive_color"),1,emissive_color);
	glUniform4fv(glGetUniformLocation(shaders, "ambient_color"),1,ambient_color);
	glUniform4fv(glGetUniformLocation(shaders, "mat_diffuse"),1,mat_diffuse);
	glUniform4fv(glGetUniformLocation(shaders, "mat_specular"),1,mat_specular);
	if (previousGrenzWinkel != grenzWinkel || bPreviousSmoothShading != bSmoothShading || bPreviousPhongShading != bPhongShading) {
		previousGrenzWinkel = grenzWinkel;
		normalBatch.~GLBatch();
		modelBatch.~GLBatch();
		CreateGeometry();
	}

	modelBatch.Draw();
	if (bShowNormals) normalBatch.Draw();
	
	bPreviousPhongShading = bPhongShading;
	bPreviousSmoothShading = bSmoothShading;
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
	glClearColor(0.5f, 0.5f, 0.0f, 1.0f );
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
