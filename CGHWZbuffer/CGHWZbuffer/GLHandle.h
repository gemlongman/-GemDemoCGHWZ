#pragma once

#include <GL/glut.h>
#include "SZBuffer.h"

void GLHandleInit(Model* model, SZBuffer* slzBuffer);
void GLHandleRun();

#ifdef debuggy
void printMatrix(float matrix[][3]);
#endif // debuggy