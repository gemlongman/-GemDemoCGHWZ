#pragma once

#include <GL/glut.h>
#include "ISLZBuffer.h"

void GLHandleInit(Model* model, ISLZBuffer* islzBuffer);
void GLHandleRun();

#ifdef debuggy
void printMatrix(float matrix[][3]);
#endif // debuggy