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
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLFrustum.h>
#include <math.h>
#include <math3d.h>
#include <GL/glut.h>
#include <ImageLoader/ImageLoader.h>
#include <ObjLoader/Model.h>
#include <AntTweakBar.h>
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;
GLBatch modelBatch;
GLuint shaders;

GLuint diffuseMap;
GLuint bumpMap;
GLuint displacementMap;

/// View space light position
float light_pos[4] = {5.0f,5.0f,0.0f,1.0f} ;
glm::quat rotation = glm::quat(0, 0, 0, 1);
bool  wireframe = false;
float tessLevelInner = 2.0f;
float tessLevelOuter = 2.0f;
float displacement = 1.0f;
TwBar *bar;

/** 
* Eigenes Model laden:
Die Normal-Map wird im Material-File definiert (z.B. sphere_bump.mtl). Leider übernimmt das nicht jedes Model-Tool. 
map_bump ..\\Texturen\\bumpmapping\\brickwork_normal-map.jpg

Es wird aktuell nur eine Normal-Map je Model bei CreateGeometry() unterstützt. 
*/
//const std::string modelFile = "../Modelle/plane_bump.obj";
//const std::string modelFile = "../Modelle/sphere_11102.obj";
const std::string modelFile = "../Modelle/sphere_bump.obj";
std::string diffuseMapFile = "../Texturen/displacementmapping/spnza_bricks_a_diff.tga";

void CreateGeometry()
{	
    obj::Model model (modelFile);
    modelBatch.Begin(GL_TRIANGLES,model.GetTriangleCount()*3,1);
    for (unsigned int i =0; i < model.GetTriangleCount();++i)
    {
        obj::Triangle* tr = model.GetTriangle(i);

        //Vektoren zu M3D* konvertieren
        M3DVector3f normal[3];
        M3DVector3f vertices[3];
        M3DVector2f texCoords[3];

        m3dLoadVector3(normal[0], tr->vertex[0].normal.x,tr->vertex[0].normal.y,tr->vertex[0].normal.z);
        m3dLoadVector3(normal[1], tr->vertex[1].normal.x,tr->vertex[1].normal.y,tr->vertex[1].normal.z);
        m3dLoadVector3(normal[2], tr->vertex[2].normal.x,tr->vertex[2].normal.y,tr->vertex[2].normal.z);

        m3dLoadVector3(vertices[0],tr->vertex[0].pos.x,tr->vertex[0].pos.y,tr->vertex[0].pos.z);
        m3dLoadVector3(vertices[1],tr->vertex[1].pos.x,tr->vertex[1].pos.y,tr->vertex[1].pos.z);
        m3dLoadVector3(vertices[2],tr->vertex[2].pos.x,tr->vertex[2].pos.y,tr->vertex[2].pos.z);

        m3dLoadVector2(texCoords[0],tr->vertex[0].texCoord.x,tr->vertex[0].texCoord.y);
        m3dLoadVector2(texCoords[1],tr->vertex[1].texCoord.x,tr->vertex[1].texCoord.y);
        m3dLoadVector2(texCoords[2],tr->vertex[2].texCoord.x,tr->vertex[2].texCoord.y);

        for (int j =0; j < 3;++j)
        {
            //berechne Tangente mittels der m3d-Funktion
            M3DVector3f tangent;
            m3dCalculateTangentBasis(tangent,vertices,texCoords, normal[j]);

            //binormale berechnet sich nun einfach aus dem Kreuzprodukt zwischen Normale und Tangente
            M3DVector3f binormal;
            m3dCrossProduct3(binormal,tangent,normal[j]);
            //Daten setzen
            modelBatch.MultiTexCoord2fv(0,texCoords[j]);
            modelBatch.Normal3fv(normal[j]);
            modelBatch.Tangent3fv(tangent);
            modelBatch.Binormal3fv(binormal);
            modelBatch.Vertex3fv(vertices[j]);
        }
    }
    modelBatch.End();

    //Shader Programme laden
    shaders =  gltLoadShaderQuadrupleWithAttributes("VertexShader.glsl", "TessControlShader.glsl", "TessEvalShader.glsl", "FragmentShader.glsl", 5,
                                                    GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                    GLT_ATTRIBUTE_NORMAL, "vNormal",
                                                    GLT_ATTRIBUTE_BINORMAL, "vBinormal",
                                                    GLT_ATTRIBUTE_TANGENT, "vTangent",
                                                    GLT_ATTRIBUTE_TEXTURE0, "vTexCoord");

    gltCheckErrors(shaders);
}

void TW_CALL getWireframe(void *value, void *clientData)
{
    *static_cast<bool*>(value) = wireframe;
}

void TW_CALL setWireframe(const void *value, void *clientData)
{
    wireframe = *static_cast<const bool*>(value);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}

void InitGUI()
{
    bar = TwNewBar("TweakBar");
    TwDefine(" TweakBar size='200 400'");
    TwAddVarRW(bar,"Model Rotation",TW_TYPE_QUAT4F, glm::value_ptr(rotation),"");
    TwAddVarRW(bar, "Light Position", TW_TYPE_DIR3F, &light_pos," axisx=-x axisy=-y axisz=-z");
    TwAddVarCB(bar, "Wireframe?", TW_TYPE_BOOLCPP, setWireframe, getWireframe, NULL, "");
    TwAddVarRW(bar, "Tess Level Inner", TW_TYPE_FLOAT, &tessLevelInner, "min=1");
    TwAddVarRW(bar, "Tess Level Outer", TW_TYPE_FLOAT, &tessLevelOuter, "min=1");
    TwAddVarRW(bar, "Displacement Factor", TW_TYPE_FLOAT, &displacement, "step=0.01");
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

    //setze den Shader für das Rendern
    glUseProgram(shaders);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bumpMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, displacementMap);
    // Model View Projection Matrix setzen
    glUniformMatrix4fv(glGetUniformLocation(shaders, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(glGetUniformLocation(shaders, "mvMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    glUniformMatrix3fv(glGetUniformLocation(shaders, "normalMatrix"), 1, GL_FALSE, transformPipeline.GetNormalMatrix());
    //diffuse farbe setzen
    glUniform3fv(glGetUniformLocation(shaders, "lightPos"),1,light_pos);
    glUniform4f(glGetUniformLocation(shaders, "lightColorAmbient"),0.1f,0.1f,0.1f,1);
    glUniform4f(glGetUniformLocation(shaders, "lightColorSpecular"),1.0f,1.0f,1.0f,1);
    glUniform4f(glGetUniformLocation(shaders, "lightColorDiffuse"),1.0f,1.0f,1.0f,1);
    glUniform1f(glGetUniformLocation(shaders, "specPow"),60.0f);

    glUniform1f(glGetUniformLocation(shaders, "displacement"), displacement);

    glUniform1i(glGetUniformLocation(shaders, "diffuseMap"),0);
    glUniform1i(glGetUniformLocation(shaders, "bumpMap"),1);
    glUniform1i(glGetUniformLocation(shaders, "displacementMap"),2);
    //Zeichne Model

    glUniform1f(glGetUniformLocation(shaders, "tessLevelInner"), tessLevelInner);
    glUniform1f(glGetUniformLocation(shaders, "tessLevelOuter"), tessLevelOuter);

    modelBatch.Draw(GL_PATCHES);


    // Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
    modelViewMatrix.PopMatrix();
    //gltCheckErrors(shaders);
    gltCheckErrors();
    TwDraw();
    // Vertausche Front- und Backbuffer
    glutSwapBuffers();
    glutPostRedisplay();
}

void ComputeHeightMap(unsigned char* data, int width, int height)
{
	//HeightMap berechnen
	unsigned char* heightBuf = new unsigned char[width * height];
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			unsigned char red = data[((x + y*width) * 3)];
			unsigned char green = data[((x + y*width) * 3) + 1];
			unsigned char blue = data[((x + y*width) * 3) + 2];

			glm::vec3 rgb = glm::vec3(red / 255.0f, green / 255.0f, blue / 255.0f);

			float lumi = glm::dot(rgb, glm::vec3(0.21, 0.72, 0.07));

			heightBuf[((x + y*width))] = lumi * 255.0f;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, heightBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete[] heightBuf;
}

void ComputeBumpMap(unsigned char* heightData, int width, int height)
{
    // BumpMap berechnen basierend auf HeightMap
    unsigned char* bumpBuf = new unsigned char[width * height * 3];
    for (int y = 1; y < height - 1; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            // Höhenwerte der benachbarten Pixel
            float heightL = heightData[(x - 1) + y * width] / 255.0f;
            float heightR = heightData[(x + 1) + y * width] / 255.0f;
            float heightD = heightData[x + (y - 1) * width] / 255.0f;
            float heightU = heightData[x + (y + 1) * width] / 255.0f;

            // Berechne den Gradienten
            glm::vec3 gradient = glm::vec3((heightL - heightR), (heightD - heightU), 1.0f);
            glm::vec3 normal = glm::normalize(gradient);

            // Konvertiere Normalenvektor in den Bereich [0, 255]
            bumpBuf[(x + y * width) * 3] = (unsigned char)((normal.x * 0.5f + 0.5f) * 255.0f);
            bumpBuf[(x + y * width) * 3 + 1] = (unsigned char)((normal.y * 0.5f + 0.5f) * 255.0f);
            bumpBuf[(x + y * width) * 3 + 2] = (unsigned char)((normal.z * 0.5f + 0.5f) * 255.0f);
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bumpBuf);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    delete[] bumpBuf;
}


// Initialisierung des Rendering Kontextes
void SetupRC()
{
    // Schwarzer Hintergrund
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    // In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
    // Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans benützen
    glFrontFace(GL_CW);

    transformPipeline.SetMatrixStacks(modelViewMatrix,projectionMatrix);
    //erzeuge die geometrie
    CreateGeometry();

    glEnable(GL_TEXTURE_2D);
    img::ImageLoader imgLoader;
    glGenTextures(1, &diffuseMap);
    glBindTexture(GL_TEXTURE_2D,diffuseMap);
    int width,height;
    //Textur einlesen
    unsigned char* data = imgLoader.LoadTextureFromFile(diffuseMapFile.c_str(),&width,&height,true);
    //Textur hochladen
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) ;
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) ;


    glGenTextures(1, &displacementMap);
    glBindTexture(GL_TEXTURE_2D, displacementMap);

	//Berechne Heigthmap aus Farbe
	ComputeHeightMap(data, width, height);


    glGenTextures(1,&bumpMap);
    glBindTexture(GL_TEXTURE_2D, bumpMap);

    ComputeBumpMap(data, width, height);

    delete[] data;


    InitGUI();
}

void ShutDownRC()
{
    TwTerminate();
    glDeleteProgram(shaders);
}


void ChangeSize(int w, int h)
{
    //Model ist normalisiert im Ursprung
    GLfloat nRange = 1.0f;
    // Verhindere eine Division durch Null
    if(h == 0)
        h = 1;
    // Setze den Viewport gemaess der Window-Groesse
    glViewport(0, 0, w, h);
    // Ruecksetzung des Projection matrix stack
    projectionMatrix.LoadIdentity();
    viewFrustum.SetPerspective(90.0f, w / (float) h, 0.1f, 100.0f);
    // Definiere das viewing volume (left, right, bottom, top, near, far)
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    // Ruecksetzung des Model view matrix stack
    modelViewMatrix.LoadIdentity();
    modelViewMatrix.Translate(0,0,-1.5f);
    // Send the new window size to AntTweakBar
    TwWindowSize(w, h);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Aufgabe5 - Displacement Mapping");
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
    glutSpecialFunc((GLUTspecialfun)TwEventKeyboardGLUT);

    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    TwInit(TW_OPENGL, NULL);

    SetupRC();

    glutMainLoop();

    ShutDownRC();

    return 0;
}
