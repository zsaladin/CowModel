#define RESET	1
#define ORTHO	2
#define PERSP	3
#define TEXTU   4

#define PI 3.141592654
#define TWOPI 6.283185308

/*	Create checkerboard texture	*/
#define	checkImageWidth 64
#define	checkImageHeight 40


#include <vector>

#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>

#include "openobj.h"

using namespace std;

static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
static GLuint texName;

int projection = ORTHO;
int preX = -1, preY = -1;
int mouseX = -1, mouseY = -1;

vector<pos3d> varray;
vector<vector<int>> polyarray ;
vector<pos3d> polygonNormals;
vector<pos3d> vertexNormals;

vector<float> vertexDiffuse;
vector<float> vertexSpecular;

vector<float> vertexReflection;

GLfloat *facesTriangles;
GLuint *indices;

float curTime = GetTickCount();

float firstWidth = 500, firstHeight = 500;
float currentWidth = firstWidth, currentHeight = firstHeight;
float cameraX = 0, cameraY = 0, cameraZ = 15;
float firstCameraX = 0, firstCameraY = 0, firstCameraZ = 15;

float angleX = 0, angleY = 0;
float angleXL = 0, angleYL = 0;

bool mouseClick = false;
bool mouseMove = false;
bool hidden = false;
bool flat = false;
bool gouraud = false;
bool texture = false;

float ka = 0.2;
float kd = 0.4;
float ks = 0.4;

float intensity = 1;

pos3d light;



float angle = 0.0;
float distance = 15;

float upX = 0, upY = 1, upZ = 0;
float firstUpX = 0, firstUpY = 1, firstUpZ = 0;

void makeCheckImage(void)
{
   int i, j, c;
    
   for (i = 0; i < checkImageHeight; i++)     
    for (j = 0; j < checkImageWidth; j++)
	{
		c = ((((i&0x8)==0)^((j&0x8))==0))*255; 
		checkImage[i][j][0] = (GLubyte) c;
		checkImage[i][j][1] = (GLubyte) c; 
		checkImage[i][j][2] = (GLubyte) c; 
		checkImage[i][j][3] = (GLubyte) 255;

		if(checkImage[i][j][0] == 0 && checkImage[i][j][1] == 0 && checkImage[i][j][2] == 0)
		{
			checkImage[i][j][0] = 0;
			checkImage[i][j][1] = 255;
			checkImage[i][j][2] = 0;
		}
	}
}
void cameraRotate()
{

	cameraX = firstCameraX*cos(-angleY/100)+firstCameraZ*sin(-angleX/100);
	cameraY = firstCameraX*sin(-angleY/100)*sin(-angleX/100)+firstCameraY*cos(-angleY/100)+firstCameraZ*-sin(-angleY/100)*cos(-angleX/100);
	cameraZ = firstCameraX*-sin(-angleX/100)*cos(-angleY/100)+firstCameraY*sin(-angleY/100)+firstCameraZ*cos(-angleY/100)*cos(-angleX/100);

	upX = firstUpX*cos(-angleY/100)+firstUpZ*sin(-angleX/100);
	upY = firstUpX*sin(-angleY/100)*sin(-angleX/100)+firstUpY*cos(-angleY/100)+firstUpZ*-sin(-angleY/100)*cos(-angleX/100);
	upZ = firstUpX*-sin(-angleX/100)*cos(-angleY/100)+firstUpY*sin(-angleY/100)+firstUpZ*cos(-angleY/100)*cos(-angleX/100);


}

void calculatePolygonNormal()
{
	polygonNormals.clear();
	for(int i = 0; i < polyarray.size(); ++i)
	{
		pos3d v0 = varray[polyarray[i][0]];
		pos3d v1 = varray[polyarray[i][1]];
		pos3d v2 = varray[polyarray[i][2]];

		pos3d normal;
		pos3d temp0;
		pos3d temp1;

		temp0.x = v1.x - v0.x;
		temp0.y = v1.y - v0.y;
		temp0.z = v1.z - v0.z;

		temp1.x = v2.x - v0.x;
		temp1.y = v2.y - v0.y;
		temp1.z = v2.z - v0.z;

		normal.x = (temp0.y*temp1.z)-(temp0.z*temp1.y);
		normal.y = (temp0.z*temp1.x)-(temp0.x*temp1.z);
		normal.z = (temp0.x*temp1.y)-(temp0.y*temp1.x);

		float normalAb = sqrt(normal.x*normal.x+normal.y*normal.y+normal.z*normal.z);
		normal.x /= normalAb;
		normal.y /= normalAb;
		normal.z /= normalAb;
		polygonNormals.push_back(normal);

	}

}
void calculateVertexNormal()
{
	vertexNormals.clear();
	vertexNormals.resize(varray.size());
	for(int i = 0; i < polyarray.size(); ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			int index = polyarray[i][j];
			vertexNormals[index].x += polygonNormals[i].x;
			vertexNormals[index].y += polygonNormals[i].y;
			vertexNormals[index].z += polygonNormals[i].z;
		}
	}

	for(int k = 0; k < vertexNormals.size(); ++k)
	{
		float vertexNormalAb = sqrt(vertexNormals[k].x*vertexNormals[k].x+vertexNormals[k].y*vertexNormals[k].y+vertexNormals[k].z*vertexNormals[k].z); 
		vertexNormals[k].x /= vertexNormalAb;			
		vertexNormals[k].y /= vertexNormalAb;
		vertexNormals[k].z /= vertexNormalAb;
	}

}
void calculateVertexDiffuse()
{
	vertexDiffuse.clear();
	pos3d lightDirection;

	for(int i = 0; i < varray.size(); ++i)
	{
		lightDirection.x = light.x - varray[i].x;
		lightDirection.y = light.y - varray[i].y;
		lightDirection.z = light.z - varray[i].z;

		float lightDirectionAb = sqrt(lightDirection.x*lightDirection.x+lightDirection.y*lightDirection.y+lightDirection.z*lightDirection.z);
		lightDirection.x /= lightDirectionAb;
		lightDirection.y /= lightDirectionAb;
		lightDirection.z /= lightDirectionAb;

		vertexDiffuse.push_back(intensity*
			(lightDirection.x*vertexNormals[i].x+lightDirection.y*vertexNormals[i].y+lightDirection.z*vertexNormals[i].z));
	}
}
void calculateVertexSpecular()
{
	vertexSpecular.clear();

	pos3d viewingDirection;
	pos3d reflecttionDirection;
	pos3d lightDirection;

	const float n = 10;

	for(int i = 0; i < varray.size(); ++i)
	{
		lightDirection.x = light.x - varray[i].x;
		lightDirection.y = light.y - varray[i].y;
		lightDirection.z = light.z - varray[i].z;

		float lightDirectionAb = sqrt(lightDirection.x*lightDirection.x+lightDirection.y*lightDirection.y+lightDirection.z*lightDirection.z);
		lightDirection.x /= lightDirectionAb;
		lightDirection.y /= lightDirectionAb;
		lightDirection.z /= lightDirectionAb;

		float cosq = lightDirection.x*vertexNormals[i].x+lightDirection.y*vertexNormals[i].y+lightDirection.z*vertexNormals[i].z;

		reflecttionDirection.x = 2*vertexNormals[i].x*cosq-lightDirection.x;
		reflecttionDirection.y = 2*vertexNormals[i].y*cosq-lightDirection.y;
		reflecttionDirection.z = 2*vertexNormals[i].z*cosq-lightDirection.z;

		viewingDirection.x = cameraX - varray[i].x;
		viewingDirection.y = cameraY - varray[i].y;
		viewingDirection.z = cameraZ - varray[i].z;

		float viewingDirectionAb = sqrt(viewingDirection.x*viewingDirection.x+viewingDirection.y*viewingDirection.y+viewingDirection.z*viewingDirection.z);
		viewingDirection.x /= viewingDirectionAb;
		viewingDirection.y /= viewingDirectionAb;
		viewingDirection.z /= viewingDirectionAb;

		vertexSpecular.push_back(intensity*
			pow((reflecttionDirection.x*viewingDirection.x+reflecttionDirection.y*viewingDirection.y+reflecttionDirection.z*viewingDirection.z), n));

	}
}
void calculateVertexReflection()
{
	vertexReflection.clear();
	for(int i = 0; i < varray.size(); ++i)
	{
		vertexReflection.push_back(ka*intensity+kd*vertexDiffuse[i]+ks*vertexSpecular[i]);
	}
}

float* sphericalMap(float r, float u, float v)
{
	float x = r * cos(2 * PI * u);
	float y = r * sin(2 * PI * u) * cos(2 * PI * v);
	float z = r * sin(2 * PI * u) * sin(2 * PI * v);

	float arr[3];
	arr[0] = x;
	arr[1] = y;
	arr[2] = z;

	return arr;

}

void SphereMap(float x,float y,float z,float *u, float *v)
{
	float firstX = x, firstY = y, firstZ = z;

	float r;

	r =  sqrt(x*x+y*y+z*z);
	*v = atan(y/z)/(PI/2)*0.5+0.5;
	*u = acos(x/r)/PI;
}

void display (void) {

	glClearColor(1.0, 1.0, 1.0, 1);
	glClear (GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	if(flat)
		glShadeModel(GL_FLAT); //Flat Shading
	else if(gouraud)
		glShadeModel(GL_SMOOTH); //Gouroud Shading

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();



	if(mouseMove)
	{
		angleXL = (mouseX - preX);
		angleYL = (mouseY - preY);

		angleX += (mouseX - preX);
		angleY += (mouseY - preY);

		mouseMove = false;
		cameraRotate();

	}






	gluLookAt(cameraX, cameraY, cameraZ, 0.0, 0.0, 0.0, upX, upY, upZ);

	if(texture)
	{
		
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		

		makeCheckImage();

		glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);



		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	}

	calculateVertexSpecular();
	calculateVertexReflection();


	if(hidden || flat || gouraud)
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 1.0, 1.0 );


		//glPolygonMode(GL_BACK, GL_FILL);
		for(int i = 0; i < polyarray.size(); i++)
		{
			glBegin(GL_TRIANGLES);

			for(int j = 0; j < 3; j++){
				float r, g, b;
				if(hidden) r= g = b = 1;
				else r = g = b = vertexReflection[polyarray[i][j]];

				float u, v;
				glColor3f (r, g, b);

				float x = varray[polyarray[i][j]].x;
				float y = varray[polyarray[i][j]].y;
				float z = varray[polyarray[i][j]].z;

		
				if(hidden && !texture)
				{
					glVertex3f(x, y, z);
				}
				else
				{
					SphereMap(x, y, z, &u, &v);
					glTexCoord2f(u, v);
					glVertex3f(x, y, z);
				}

			}
			glEnd();
		}

	}
	glDisable(GL_TEXTURE_2D);
	if( !flat && !gouraud)
	{
		glColor3f (0, 0, 0);

		for(int i = 0; i < polyarray.size(); i++){
			glBegin(GL_LINE_LOOP);
			for(int j = 0; j < 3; j++)
			{
				glVertex3f(varray[polyarray[i][j]].x, varray[polyarray[i][j]].y, varray[polyarray[i][j]].z);

			}
			glEnd();
		}
	}
	glDisable(GL_POLYGON_OFFSET_FILL);

	
	glutSwapBuffers();

}

void myReshape (int w, int h) {
	glViewport (0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspectWidth = w/firstWidth;
	float aspectHeight = h/firstHeight;
	if(projection == ORTHO)
	{
		glOrtho(-6.0f*aspectWidth/aspectHeight, 6.0f*aspectWidth/aspectHeight, -6.0f, 6.0f, 0.001, 1000.0); //샘플과 동일하게 구현(세로 넓힐 경우 오브젝트도 확대
	}
	else 
		gluPerspective(40, (float)w/h, 0.001, 1000.0);
	currentWidth = w;
	currentHeight = h;
	display();
}
void keyfunc(unsigned char key, int x, int y)
{
	switch(key){
	case 'q':
		exit(0);
		break;
	case 'w':
		hidden = false;
		gouraud = false;
		flat = false;
		break;
	case 'h':
		hidden = true;
		gouraud = false;
		flat = false;
		break;
	case 'g':
		hidden = false;
		flat = false;
		gouraud = true;
		break;
	case 'f':
		hidden = false;
		gouraud = true;
		flat = true;
		break;
	default:
		break;
	}
}
void anifunc()
{
	if(GetTickCount() - curTime > 66)
	{
		curTime +=  67;
	}
	display();
}

void menufunc(int value)
{
	switch(value)
	{
	case RESET:
		//projection = ORTHO;
		angleX = 0;
		angleY = 0;
		cameraRotate();
		break;
	case ORTHO:
		projection = ORTHO;
		myReshape(currentWidth, currentHeight);
		break;
	case PERSP:
		projection = PERSP;
		myReshape(currentWidth, currentHeight);
		break;
	case TEXTU:
		if(texture) texture = false;
		else texture = true;
		break;

	}
	display();
}

void onMouseClick(int button, int state, int xx, int yy)
{

	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseClick = true;
		mouseX = xx;
		mouseY = yy;
		preX = mouseX;
		preY = mouseY;

	}
	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		mouseClick = false;
		mouseX = -1;
		mouseY = -1;
		preX = -1;
		preY = -1;
	}
	display();
}

void onMouseMove(int xx, int yy)
{
	if(mouseClick)
	{
		preX = mouseX;
		preY = mouseY;
		mouseX = xx;
		mouseY = yy;
		mouseMove = true;
	}
}



void main (int argc, char **argv) 
{	
	light.x = 0;
	light.y = 0;
	light.z = 15;
	openObj("cow.obj", varray, polyarray);
	calculatePolygonNormal();
	calculateVertexNormal();
	calculateVertexDiffuse();


	glutInit (&argc, argv);	
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500,500);    	
	glutInitWindowPosition(0,0); 
	glutCreateWindow ("simple");
	glutReshapeFunc (myReshape);
	glutDisplayFunc (display);
	glutKeyboardFunc(keyfunc);
	glutCreateMenu(menufunc);
	glutAddMenuEntry("Reset", RESET);
	glutAddMenuEntry("Ortho", ORTHO);
	glutAddMenuEntry("Persp", PERSP);
	glutAddMenuEntry("Texture mapping", TEXTU);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutIdleFunc(anifunc);
	glutMouseFunc(onMouseClick);
	glutMotionFunc(onMouseMove);

	glutMainLoop();
}
