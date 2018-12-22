#include "PublicStruct.h"
#include <algorithm>
#include <omp.h>

using namespace std;

void clip01(float & value)
{
	if (1.0 < value)
	{
		value = 1.0;
		return;
	}
	if (0.0 > value)
	{
		value = 0.0;
		return;
	}
	return;
}

bool isEqualf(float a, float b)
{
	return fabs(a - b) < EPS;
}

void PointLight::ShaderModel(Model& model)
{
	int faceNum = model.faces.size();

	for (int i = 0; i < faceNum; i++)
	{
		Face & face = model.faces[i];
		int faceVertexNum = face.vertexIndex.size();
		for (int j = 0; j < faceVertexNum; j++)
		{
			Vertex faceVertex = model.vertexes[ face.vertexIndex[j] ];
			Vector3f rayDirection = (lightPosition - faceVertex.point).normalize();

			Vector3f normal;
			if ( 0 <= face.normalIndex[j])
			{
				normal = model.normals[ face.normalIndex[j] ];
			}
			else
			{
				normal =  face.normal;
			}
			
			float cosValue = rayDirection.dot(normal);
			if ( 0 < cosValue ) 
			{
				face.color = face.color + lightColor * 0.8 * cosValue;//diffuse reflection
			}
				
			face.color = face.color + ambientColor;
		}
		face.color = face.color / face.vertexIndex.size(); // average vertex color

		clip01(face.color.r);
		clip01(face.color.g);
		clip01(face.color.b);
	}
}



