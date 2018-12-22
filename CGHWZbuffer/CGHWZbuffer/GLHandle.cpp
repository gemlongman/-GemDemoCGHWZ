#include "GLHandle.h"

using namespace std;

const float PI = 3.1415926;

SZBuffer* theSZBuffer = NULL;
Model* model = NULL;

void GLHandleInit(Model* iModel, SZBuffer* iSZBuffer)
{
	model = iModel;
	theSZBuffer = iSZBuffer;
}

void glDisplay()
{
	int width = 0, height = 0;
	theSZBuffer->GetWidthHeight(width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);

	theSZBuffer->Scan(*model);

	glBegin(GL_POINTS);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			Color3f rgb(1.0f, 1.0f, 1.0f);
			int tmp = theSZBuffer->PolygonIdBuffer[y][x];
			if (tmp >= 0)
			{
				rgb = model->faces[tmp].color;
			}
			//cout << "r" << rgb.r << " g " << rgb.g << " b " << rgb.b << endl;
			glColor3f(rgb.r, rgb.g, rgb.b);
			glVertex2i(x, y);
		}
	}
	glEnd();

	glFinish();
}

void reSetWindows(int w, int h)
{
	glViewport(0, 0, w, h);
	model->ResetModelSize(w, h);
	theSZBuffer->SetWidthHeight(w, h);
	theSZBuffer->Scan(*model);
}


void keyboardHandle(unsigned char key, int x, int y)
{
	float RotateMatrix[3][3] = { 0.0 };
	float rotateAngle = 0.3; //PI / 12
	
	float cosValue = cos(rotateAngle);
	float sinValue = sin(rotateAngle);//rotateAngle = -rotateAngle;

	switch (key)
	{
	case 'a':
		RotateMatrix[0][0] = cosValue;
		RotateMatrix[0][1] = 0;
		RotateMatrix[0][2] = -sinValue;

		RotateMatrix[1][0] = 0;
		RotateMatrix[1][1] = 1;
		RotateMatrix[1][2] = 0;

		RotateMatrix[2][0] = sinValue;
		RotateMatrix[2][1] = 0;
		RotateMatrix[2][2] = cosValue;
		break;
	case 'd':
		RotateMatrix[0][0] = cosValue;
		RotateMatrix[0][1] = 0;
		RotateMatrix[0][2] = sinValue;

		RotateMatrix[1][0] = 0;
		RotateMatrix[1][1] = 1;
		RotateMatrix[1][2] = 0;

		RotateMatrix[2][0] = -sinValue;
		RotateMatrix[2][1] = 0;
		RotateMatrix[2][2] = cosValue;
		break;
	case 'w':
		RotateMatrix[0][0] = 1;
		RotateMatrix[0][1] = 0;
		RotateMatrix[0][2] = 0;

		RotateMatrix[1][0] = 0;
		RotateMatrix[1][1] = cosValue;
		RotateMatrix[1][2] = sinValue;

		RotateMatrix[2][0] = 0;
		RotateMatrix[2][1] = -sinValue;
		RotateMatrix[2][2] = cosValue;
		break;
	case 's':
		RotateMatrix[0][0] = 1;
		RotateMatrix[0][1] = 0;
		RotateMatrix[0][2] = 0;

		RotateMatrix[1][0] = 0;
		RotateMatrix[1][1] = cosValue;
		RotateMatrix[1][2] = -sinValue;

		RotateMatrix[2][0] = 0;
		RotateMatrix[2][1] = sinValue;
		RotateMatrix[2][2] = cosValue;
		break;
	case 'q':
		RotateMatrix[0][0] = cosValue;
		RotateMatrix[0][1] = -sinValue;
		RotateMatrix[0][2] = 0;

		RotateMatrix[1][0] = sinValue;
		RotateMatrix[1][1] = cosValue;
		RotateMatrix[1][2] = 0;

		RotateMatrix[2][0] = 0;
		RotateMatrix[2][1] = 0;
		RotateMatrix[2][2] = 1;
		break;
	case 'e':
		RotateMatrix[0][0] = cosValue;
		RotateMatrix[0][1] = sinValue;
		RotateMatrix[0][2] = 0;

		RotateMatrix[1][0] = -sinValue;
		RotateMatrix[1][1] = cosValue;
		RotateMatrix[1][2] = 0;

		RotateMatrix[2][0] = 0;
		RotateMatrix[2][1] = 0;
		RotateMatrix[2][2] = 1;
		break;
	default:
		return;
		break;
	}

	model->ModelRotate(RotateMatrix);

	theSZBuffer->needUpdate = true;
	theSZBuffer->Scan(*model);
	glutPostRedisplay();
}

void specialKeysHandle(int key, int X, int Y)
{
	Vector3f displacement;
	float detal = model->ModelMaxDistance / 100;
	switch (key)
	{
	case GLUT_KEY_UP:
		displacement.y = detal;
		model->ModelMove(displacement);
		break;
	case GLUT_KEY_DOWN:
		displacement.y = -detal;
		model->ModelMove(displacement);
		break;
	case GLUT_KEY_RIGHT:
		displacement.x = detal;
		model->ModelMove(displacement);
		break;
	case GLUT_KEY_LEFT:
		displacement.x = -detal;
		model->ModelMove(displacement);
		break;
	case GLUT_KEY_PAGE_UP:
		model->ModelScale(1.1f);
		break;
	case GLUT_KEY_PAGE_DOWN:
		model->ModelScale(0.9f);
		break;
	default:
		return;
	}

	theSZBuffer->needUpdate = true;
	theSZBuffer->Scan(*model);
	glutPostRedisplay();
}


void GLHandleRun()
{
	int width = 0, height = 0;
	theSZBuffer->GetWidthHeight(width, height);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 80);
	glutCreateWindow("GemDemo4CGSZBuffer");
	glutDisplayFunc(glDisplay);
	glutReshapeFunc(reSetWindows);
	//glutMouseFunc(mouseMotionHandle);
	glutKeyboardFunc(keyboardHandle);
	glutSpecialFunc(specialKeysHandle);

	glutMainLoop();
}

#ifdef debuggy
void printMatrix(float matrix[][3])
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
}
#endif // debuggy