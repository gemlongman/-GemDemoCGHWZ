#include "PublicStruct.h"
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>

using namespace std;

bool Model::LoadObj(string path)
{
	ifstream fileSteam(path);
	if (!fileSteam.is_open())
	{
		return false;
	}

	string type;
	while (fileSteam >> type)
	{
		if (type == "v")
		{
			Vertex vt;
			fileSteam >> vt.point.x >> vt.point.y >> vt.point.z;
			vertexes.push_back(vt);
		}
		else if (type == "f")
		{
			Face face;
			int vIndex, tIndex, nIndex;
			int faceIndex = faces.size();

			//fileSteam >> pointIndex1 >> pointIndex2 >> pointIndex3 >> pointIndex4;
			while (true)
			{
				char ch = fileSteam.get();
				if (ch == ' ') continue;
				else if (ch == '\n' || ch == EOF) break;
				else fileSteam.putback(ch);


				fileSteam >> vIndex;

				char splitter = fileSteam.get();
				nIndex = 0;
				if (splitter == '/')
				{
					cout << splitter;
					splitter = fileSteam.get();
					if (splitter == '/')
					{
						fileSteam >> nIndex;//normal index
					}
					else
					{
						fileSteam.putback(splitter);
						fileSteam >> tIndex;//texture index
						splitter = fileSteam.get();
						if (splitter == '/')
						{
							fileSteam >> nIndex;
						}
						else fileSteam.putback(splitter);
					}
				}
				else fileSteam.putback(splitter);

				face.vertexIndex.push_back(vIndex - 1);
				face.normalIndex.push_back(nIndex - 1);
			}
			//normal
			if (face.vertexIndex.size() > 2)
			{
				Point3f a = vertexes[face.vertexIndex[0]].point;
				Point3f	b = vertexes[face.vertexIndex[1]].point;
				Point3f c = vertexes[face.vertexIndex[2]].point;
				Vector3f normal = ( (b - a).cross(c - b) ).normalize();

				face.normal = normal;
				faces.push_back(face);
			}
		}
		else if (type == "vn") 
		{
			
			Vector3f vn;
			fileSteam >> vn.x >> vn.y >> vn.z;
			normals.push_back(vn);
		}
	}
	fileSteam.close();
	cout << "load faces:" << faces.size() << endl;
	cout << "load vertexes:" << vertexes.size() << endl;
	return true;
}

Model::Model(string path)
{
	if ( !LoadObj(path))
	{
		cout << "load failed : " + path << endl;
	}
}

void Model::ModelRotate(float RotateMatrix[][3])
{
	int vertex_num = vertexes.size();
	for (int i = 0; i < vertex_num; i++)
	{
		Point3f point;
		Point3f tmp_point = vertexes[i].point - centerPoint;
		vertexes[i].point.x = RotateMatrix[0][0] * tmp_point.x + RotateMatrix[0][1] * tmp_point.y + RotateMatrix[0][2] * tmp_point.z;
		vertexes[i].point.y = RotateMatrix[1][0] * tmp_point.x + RotateMatrix[1][1] * tmp_point.y + RotateMatrix[1][2] * tmp_point.z;
		vertexes[i].point.z = RotateMatrix[2][0] * tmp_point.x + RotateMatrix[2][1] * tmp_point.y + RotateMatrix[2][2] * tmp_point.z;
		vertexes[i].point = vertexes[i].point + centerPoint;
	}

	//RecomputeNormal();

	int normal_num = normals.size();
	for (int i = 0; i < normal_num; i++)
	{
		Point3f tmp_point = normals[i];
		normals[i].x = RotateMatrix[0][0] * tmp_point.x + RotateMatrix[0][1] * tmp_point.y + RotateMatrix[0][2] * tmp_point.z;
		normals[i].y = RotateMatrix[1][0] * tmp_point.x + RotateMatrix[1][1] * tmp_point.y + RotateMatrix[1][2] * tmp_point.z;
		normals[i].z = RotateMatrix[2][0] * tmp_point.x + RotateMatrix[2][1] * tmp_point.y + RotateMatrix[2][2] * tmp_point.z;

	}

	int face_num = faces.size();
	for (int i = 0; i < face_num; i++)
	{
		Point3f tmp_point = faces[i].normal;
		faces[i].normal.x = RotateMatrix[0][0] * tmp_point.x + RotateMatrix[0][1] * tmp_point.y + RotateMatrix[0][2] * tmp_point.z;
		faces[i].normal.y = RotateMatrix[1][0] * tmp_point.x + RotateMatrix[1][1] * tmp_point.y + RotateMatrix[1][2] * tmp_point.z;
		faces[i].normal.z = RotateMatrix[2][0] * tmp_point.x + RotateMatrix[2][1] * tmp_point.y + RotateMatrix[2][2] * tmp_point.z;
	}
}

void Model::ModelMove(Vector3f displacement)
{
	int vertex_num = vertexes.size();
	for (int i = 0; i < vertex_num; i++)
	{
		vertexes[i].point = vertexes[i].point + displacement;
	}

	RecomputeNormal();
}

void Model::ModelScale(float scale)
{
	int vertex_num = vertexes.size();
	for (int i = 0; i < vertex_num; i++)
	{
		Point3f tmp_point = vertexes[i].point - centerPoint;
		tmp_point = tmp_point * scale;
		vertexes[i].point = tmp_point + centerPoint;
		
	}

	RecomputeNormal();
}

void Model::RecomputeNormal()
{
	int face_num = faces.size();
	for (int i = 0; i < face_num; i++)
	{
		Point3f a = vertexes[faces[i].vertexIndex[0]].point;
		Point3f	b = vertexes[faces[i].vertexIndex[1]].point;
		Point3f c = vertexes[faces[i].vertexIndex[2]].point;
		Vector3f normal = ((b - a).cross(c - b)).normalize();

		faces[i].normal = normal;
	}
}

void Model::ResetModelSize(int width, int height)
{
	Point3f minPosition(0xfffffff, 0xfffffff, 0xfffffff);
	Point3f maxPosition(-0xfffffff, -0xfffffff, -0xfffffff);
	Point3f centerPosition(0.0, 0.0, 0.0);
	int vertex_num = vertexes.size();
	for (int i = 0; i < vertex_num; i++)
	{
		const Point3f & vertex = vertexes[i].point;

		minPosition.x = min(minPosition.x, vertex.x);
		minPosition.y = min(minPosition.y, vertex.y);
		minPosition.z = min(minPosition.z, vertex.z);

		maxPosition.x = max(maxPosition.x, vertex.x);
		maxPosition.y = max(maxPosition.y, vertex.y);
		maxPosition.z = max(maxPosition.z, vertex.z);
	}
	centerPosition = (minPosition + maxPosition) / 2;

	float modelWidth = maxPosition.x - minPosition.x;
	float modelHeight = maxPosition.y - minPosition.y;
	ModelMaxDistance = max(modelWidth, modelHeight);

	float scale = min(width, height) / ModelMaxDistance;
	scale *= 0.75;

	for (int i = 0; i < vertex_num; i++)
	{
		Point3f & vertex_point = vertexes[i].point;
		vertex_point.x = (vertex_point.x - centerPosition.x) * scale + width / 2;
		vertex_point.y = (vertex_point.y - centerPosition.y) * scale + height / 2;
		vertex_point.z = (vertex_point.z - centerPosition.z) * scale;

	}
	centerPoint = Point3f(width / 2, height / 2, 0);
}