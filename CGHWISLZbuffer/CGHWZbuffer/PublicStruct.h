#pragma once

#include <io.h>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <list>
#include <algorithm>
#include <cassert>
#include <ctime>

#define debuggy

using namespace std;

const float EPS = 1e-5;

bool isEqualf(float a, float b);
void clip01(float & value);

class Vector3f
{
public:
	union {
		struct { float x, y, z; };
		struct { float r, g, b; };
	};

	Vector3f()
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	};

	Vector3f(float ix, float iy, float iz)
	{
		x = ix;
		y = iy;
		z = iz;
	}

	Vector3f & operator = (const Vector3f &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vector3f operator + (double num)
	{
		return Vector3f(x + num, y + num, z + num);
	}

	Vector3f operator + (Vector3f other)
	{
		return Vector3f(x + other.x, y + other.y, z + other.z);
	}

	Vector3f operator - (Vector3f other)
	{
		return Vector3f(x - other.x, y - other.y, z - other.z);
	}

	Vector3f operator * (double num)
	{
		return Vector3f(x * num, y * num, z * num);
	}

	Vector3f operator / (double num)
	{
		return Vector3f(x / num, y / num, z / num);
	}

	bool operator == (Vector3f other)
	{
		return isEqualf(x, other.x) && isEqualf(y, other.y) && isEqualf(z, other.z);
	}

	Vector3f cross(Vector3f other)
	{
		return Vector3f(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	float dot(Vector3f other)
	{
		return x * other.x + y * other.y + z * other.z;
	}

	float length()
	{
		return sqrt(x * x + y * y + z * z);
	}

	Vector3f normalize()
	{
		return (*this) * (double)(1.0f / this->length());
	}
};

typedef Vector3f Color3f;
typedef Vector3f Point3f;

class Vertex
{
public:
	Point3f point;
	Color3f color;
	Vector3f normal;//of vertex
};

class Face
{
public:
	vector<int> vertexIndex;
	vector<int> normalIndex;//of vertex from file
	Vector3f normal;//of face
	Color3f color;//of face
};

class Model
{
public:
	vector<Vertex> vertexes;
	vector<Face> faces;
	vector<Vector3f> normals;//normal of vertex
	Point3f centerPoint;
	float ModelMaxDistance;

	Model(string path);
	bool LoadObj(string path);
	void ModelRotate(float RotateMatrix[][3]);
	void ModelMove(Vector3f displacement);
	void ModelScale(float scale);
	void ResetModelSize(int width, int height);
	void RecomputeNormal();
};

class PointLight
{
private:
	Point3f lightPosition;
	Color3f lightColor;
	Color3f ambientColor;

public:
	PointLight()
	{
		lightColor = Color3f(0.0f, 0.3f, 0.7f);
		ambientColor = Color3f(0.0f, 0.3f, 0.7f);
		lightPosition = Point3f(500.0f, 500.0f, 500.0f);
	}

	void ShaderModel(Model& model);
};

class Matrix
{
public:
	Matrix(int m, int n);
	~Matrix();

	int Get(int i, int j) const;
	void Set(int i, int j, int val);
	void FillSet(int val);
	void FillSetARow(int row, int val);
private:
	int rowNum;
	int colNum; 
	int *pointer;
};