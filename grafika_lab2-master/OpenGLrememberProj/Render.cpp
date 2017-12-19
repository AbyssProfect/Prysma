#include "Render.h"

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"



bool textureMode = true;
bool lightMode = true;
bool alpha = false;
//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
	if (key == 'P' && !alpha)
	{
		lightMode = !lightMode;
		textureMode = !textureMode;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		alpha = true;
	}

	else if (key == 'P' && alpha)
	{
		lightMode = !lightMode;
		textureMode = !textureMode;

		glDisable(GL_BLEND);
		alpha = false;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	GLuint texId;
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
}

void normal(double a1[], double b1[], double c1[], double vn[])
{
	double a[] = { a1[0] - b1[0],a1[1] - b1[1],a1[2] - b1[2] };
	double b[] = { c1[0] - b1[0],c1[1] - b1[1],c1[2] - b1[2] };

	vn[0] = a[1] * b[2] - b[1] * a[2];
	vn[1] = -a[0] * b[2] + b[0] * a[2];
	vn[2] = a[0] * b[1] - b[0] * a[1];

	double length = sqrt(pow(vn[0], 2) + pow(vn[1], 2) + pow(vn[2], 2));

	vn[0] /= length;
	vn[1] /= length;
	vn[2] /= length;
}
void fun_low_vip() //выпуклость на нижнем основании
{
	double a2[] = { 10,0,0 };
	double h2[] = { 13,6,0 };
	double m2[] = { 14,1,0 };
	double o2[] = { 11.35, 3.07,0 }; //центр окружности
	double r = 3.37; //радиус окружности
	double point[] = { 0,0,0 }; //точка дуги, x и y выбираем в цикле
	double alpha = -113.5;
	double betta = 61.5;
	glColor3f(1.0f, 0.5f, 0.3f);
	double vn[] = { 0,0,0 };//вектор нормали
	normal(a2, m2, h2, vn);
	glNormal3dv(vn);


	for (double i = alpha; i <= betta; i = i + 0.05)
	{	
		glBegin(GL_LINES);

		point[0] = o2[0] + r*cos(i*3.14 / 180);
		point[1] = o2[1] + r*sin(i*3.14 / 180);

		normal(a2, m2, h2, vn);
		glNormal3dv(vn);
		glVertex3dv(point);
		glVertex3dv(o2);
		glEnd();
	}

	//glEnd();
}

void fun_up_vip() //выпуклость на верхнем основании
{
	double a1[] = { 10, 0, 2};
	double h1[] = { 13, 6, 2};
	double m1[] = { 14, 1, 2};
	double o1[] = { 11.35, 3.07, 2}; //центр окружности
	double r = 3.37; //радиус окружности

	double point1[] = { 0,0,2 }; //точка дуги, x и y выбираем в цикле
	double point2[] = { 0,0,0 }; //точка дуги, x и y выбираем в цикле
	double alpha = -113.5;
	double betta = 61.5;
	double vector_n[] = { 0,0,0 };

	//glBegin(GL_POLYGON);

	double vn[] = { 0,0,0 };//вектор нормали
	normal(h1, m1, a1, vn);
	glNormal3dv(vn);


	for (double i = alpha; i <= betta; i=i+0.05)
	{
		glBegin(GL_LINES);

		point1[0] = o1[0] + r*cos(i*3.14 / 180);
		point1[1] = o1[1] + r*sin(i*3.14 / 180);


		point2[0] = o1[0] + r*cos(i*3.14 / 180);
		point2[1] = o1[1] + r*sin(i*3.14 / 180);

		vector_n[0] = point1[0];
		vector_n[1] = point1[1];
		vector_n[2] = 1;

		normal(h1, m1, a1, vn);
		glNormal3dv(vn);
		glVertex3dv(point1);
		glVertex3dv(o1);

		glNormal3dv(vector_n);
		glVertex3dv(point1);
		glVertex3dv(point2);
		glEnd();
	}

	//glEnd();

}


/*void POL(double r, double x, double y, double z, int numpoly)
{
	double vn[] = { 0,0,0 };//вектор нормали
	

	glColor3f(0.5f, 0.5f, 0.5f);
	double pi = 3.1415926535897932384;
	glBegin(GL_TRIANGLE_STRIP);
	
	glNormal3dv(vn);

	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3i(x, y, z);
	for (int i = 0; i<numpoly + 1; i++)
	{
		//normal(a, b, c, vn);
		if (i<10) glColor3f(0.5f, 0.5f, 0.5f);
		else glColor3f(0.5f, 0.5f, 0.5f);
		glVertex3f(r*cos(i*pi / numpoly) + x, r*sin(i*pi / numpoly) + y, z);
		if (i<10)glColor3f(0.5f, 0.5f, 0.5f);
		else glColor3f(0.5f, 0.5f, 0.5f);
		glVertex3i(x, y, z);
	}
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);

	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3i(x, y, 0);
	for (int i = 0; i<numpoly + 1; i++)
	{
		if (i<10)
			glColor3f(0.5f, 0.5f, 0.5f);
		else
			glColor3f(0.5f, 0.5f, 0.5f);
		glVertex3f(r*cos(i*pi / numpoly) + x, r*sin(i*pi / numpoly) + y, 0);
		if (i<10) glColor3f(0.5f, 0.5f, 0.5f);
		else glColor3f(0.5f, 0.5f, 0.5f);
		glVertex3i(x, y, 0);
	}
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < numpoly + 1; i++) {
		if (i<10) glColor3f(1.0f, 0.5f, 0.3f);
		else	glColor3f(1.0f, 0.5f, 0.3f);
		glVertex3f(r*cos(i*pi / numpoly) + x, r*sin(i*pi / numpoly) + y, 0);
		if (i<10)	glColor3f(1.0f, 0.5f, 0.3f);
		else	glColor3f(1.0f, 0.5f, 0.3f);

		glVertex3f(r*cos(i*pi / numpoly) + x, r*sin(i*pi / numpoly) + y, z);
	}
	glEnd();
}*/
void fun_circle(void) //круг внутри призмы
{
	double center[] = { -2,4,1 };
	GLfloat theta;
	GLfloat pi = acos(-1.0); //пи
	GLfloat radius = 1.0f; // радиус

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.8f, 0.3f, 0.0f); //красный
	glNormal3f(0, 0, 1);

	glTexCoord2d(0.0625, 0.9375);
	glVertex3f(center[0], center[1], 1.0f);

	for (GLfloat a = 0.0f; a <= 360.0f; a++) {
		theta = pi * a / 180.0f;

		GLdouble x = 0.0625 + 0.0625 * cos(theta);
		GLdouble y = 0.9375 + 0.0625 * sin(theta);

		glTexCoord2d(x, y);
		glVertex3f(radius * cos(theta) + center[0], radius * sin(theta) + center[1], 1.0f);
	}
	glEnd();
}

/*void build_circle()
{
	double O[] = { -3,6,1 };
	double P[] = { -5,6,1 };
	double point_cur[3] = { 0,0,1 };
	double i = 0;
	for (; i < 6.3; )
	{
		point_cur[0] = O[0] + 2 * cos(i);
		point_cur[1] = O[1] + 2 * sin(i);

		glBegin(GL_TRIANGLES);
		glColor3d(1, 0, 1);
		glVertex3dv(O);
		glVertex3dv(P);
		glVertex3dv(point_cur);
		glEnd();
		i = i + 0.1;
		P[0] = point_cur[0];
		P[1] = point_cur[1];
	}
}
*/



void Render(OpenGL *ogl)
{       	
	
	
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);\
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

    //чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	/*ff1();
	ff2();
	ff3();
	ff4();*/
	//POL(4, -3, 4, 2, 180);
    //ff5();


	glBegin(GL_TRIANGLES);

	double a1[] = { 15,18,2 };
	double b1[] = { 8,15,2 };
	double c1[] = { 1,18,2 };
	double d1[] = { 6,13,2 };
	double e1[] = { 0,7,2 };
	double f1[] = { 3,0,2 };
	double g1[] = { 7,7,2 };
	double h1[] = { 9,7.5,2 };
	double i1[] = { 10,0,2 };
	double j1[] = { 13,6,2 };
	double k1[] = { 10,13,2 };
	
	
	double vn[] = { 0,0,0 };//вектор нормали

	// первый рыб (верхний)

	normal(f1, d1, g1, vn);
	glNormal3dv(vn);

	glColor3d(0.3, 0, 0.67);

	glVertex3dv(a1);
	glVertex3dv(b1);
	glVertex3dv(k1);

	glVertex3dv(b1);
	glVertex3dv(c1);
	glVertex3dv(d1);

	glVertex3dv(k1);
	glVertex3dv(b1);
	glVertex3dv(d1);

	glVertex3dv(d1);
	glVertex3dv(e1);
	glVertex3dv(g1);

	glVertex3dv(e1);
	glVertex3dv(f1);
	glVertex3dv(g1);

	glVertex3dv(g1);
	glVertex3dv(h1);
	glVertex3dv(d1);

	glVertex3dv(h1);
	glVertex3dv(i1);
	glVertex3dv(j1);

	glVertex3dv(j1);
	glVertex3dv(h1);
	glVertex3dv(k1);

	glVertex3dv(h1);
	glVertex3dv(k1);
	glVertex3dv(d1);

	// второй рыб (нижний)

	double a2[] = { 15,18,0 };
	double b2[] = { 8,15,0 };
	double c2[] = { 1,18,0 };
	double d2[] = { 6,13,0 };
	double e2[] = { 0,7,0 };
	double f2[] = { 3,0,0};
	double g2[] = { 7,7,0 };
	double h2[] = { 9,7.5,0 };
	double i2[] = { 10,0,0 };
	double j2[] = { 13,6,0 };
	double k2[] = { 10,13,0 };
	double m2[] = { 14,1,0 };



	normal(h2, m2, a2, vn);
	glNormal3dv(vn);

	glColor3d(0.3, 0, 0.67);

	glVertex3dv(a2);
	glVertex3dv(b2);
	glVertex3dv(k2);

	glVertex3dv(b2);
	glVertex3dv(c2);
	glVertex3dv(d2);

	glVertex3dv(k2);
	glVertex3dv(b2);
	glVertex3dv(d2);

	glVertex3dv(d2);
	glVertex3dv(e2);
	glVertex3dv(g2);

	glVertex3dv(e2);
	glVertex3dv(f2);
	glVertex3dv(g2);

	glVertex3dv(g2);
	glVertex3dv(h2);
	glVertex3dv(d2);

	glVertex3dv(h2);
	glVertex3dv(i2);
	glVertex3dv(j2);

	glVertex3dv(j2);
	glVertex3dv(h2);
	glVertex3dv(k2);

	glVertex3dv(h2);
	glVertex3dv(k2);
	glVertex3dv(d2);

	glEnd();

	// квадраты (объемизация рыба)

	glBegin(GL_QUADS);

	//normal(f1, d1, g1, vn);
	//glNormal3dv(vn);


	glColor3d(0.3, 0.54, 0.67);

	normal(a2, a1, b1, vn);
	glNormal3dv(vn);

	glVertex3dv(a1);
	glVertex3dv(a2);
	glVertex3dv(b2);
	glVertex3dv(b1);

	glColor3d(0.3, 0.54, 0.67);

	normal(c1, c2, b1, vn);
	glNormal3dv(vn);

	glVertex3dv(b1);
	glVertex3dv(b2);
	glVertex3dv(c2);
	glVertex3dv(c1);

	glColor3d(0.3, 0.54, 0.67);

	normal(d1, d2, c1, vn);
	glNormal3dv(vn);

	glVertex3dv(c1);
	glVertex3dv(c2);
	glVertex3dv(d2);
	glVertex3dv(d1);

	glColor3d(0.3, 0.54, 0.67);

	normal(e1, e2, d1, vn);
	glNormal3dv(vn);

	glVertex3dv(d1);
	glVertex3dv(d2);
	glVertex3dv(e2);
	glVertex3dv(e1);

	glColor3d(0.3, 0.54, 0.67);
	normal(f1, f2, e1, vn);
	glNormal3dv(vn);

	glVertex3dv(e1);
	glVertex3dv(e2);
	glVertex3dv(f2);
	glVertex3dv(f1);

	glColor3d(0.3, 0.54, 0.67);

	normal(g1, g2, f1, vn);
	glNormal3dv(vn);

	glVertex3dv(f1);
	glVertex3dv(f2);
	glVertex3dv(g2);
	glVertex3dv(g1);

	glColor3d(0.3, 0.54, 0.67);

	normal(h1, h2, g1, vn);
	glNormal3dv(vn);

	glVertex3dv(g1);
	glVertex3dv(g2);
	glVertex3dv(h2);
	glVertex3dv(h1);

	glColor3d(0.3, 0.54, 0.67);

	normal(i1, i2, h1, vn);
	glNormal3dv(vn);

	glVertex3dv(h1);
	glVertex3dv(h2);
	glVertex3dv(i2);
	glVertex3dv(i1);

	//glColor3d(0.3, 0.54, 0.67);

	//glVertex3dv(i1);
	//glVertex3dv(i2);
	//glVertex3dv(j2);
	//glVertex3dv(j1);

	glColor3d(0.3, 0.54, 0.67);

	normal(k1, k2, j1, vn);
	glNormal3dv(vn);

	glVertex3dv(j1);
	glVertex3dv(j2);
	glVertex3dv(k2);
	glVertex3dv(k1);

	glColor3d(0.3, 0.54, 0.67);

	normal(a1, a2, k1, vn);
	glNormal3dv(vn);

	glVertex3dv(k1);
	glVertex3dv(k2);
	glVertex3dv(a2);
	glVertex3dv(a1);


	glEnd();
	fun_up_vip();

	fun_low_vip();
	
	//fun_circle();
	
	//fun_low_vip();
    //fun_side();
	//fun_low_vip();
	////build_circle();
	//fun_low();
	//fun_up();
	//glColor4f(0.5f, 0.5f, 0.5f,0.0);
	//fun_up_vip();
	//fun_up();
	//glColor4f(0.5f, 0.5f, 0.5f, 1.0);

	//Начало рисования квадратика станкина
//	double A[2] = { -4, -4 };
//	double B[2] = { 4, -4 };
//	double C[2] = { 4, 4 };
//	double D[2] = { -4, 4 };

	
	
//	glBegin(GL_QUADS);

//	glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	////glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);

	//glEnd();
	//конец рисования квадратика станкина
    
	
	//текст сообщения вверху слева, если надоест - закоментировать, или заменить =)
	char c[250];  //максимальная длина сообщения
	sprintf_s(c, "(T)Текстуры - %d\n(L)Свет - %d\n\nУправление светом:\n"
		"G - перемещение в горизонтальной плоскости,\nG+ЛКМ+перемещение по вертикальной линии\n"
		"R - установить камеру и свет в начальное положение\n"
		"F - переместить свет в точку камеры", textureMode, lightMode);
	ogl->message = std::string(c);




}   //конец тела функции

