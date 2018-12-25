#pragma once

#include "PublicStruct.h"


struct Edge
{
	float x;
	float dx;// -1/k
	int dy;//detal of y-lines
	//float ymax;
	int PolygonId;//actually it is face Id of modul
};


//actually it is face of module
struct Polygon
{
	int PolygonId;//actually faceId , which is easily used to get color
	float a, b, c, d;//face: ax+by+cz+d=0 ; normal :(a,b,c)
	//color
	bool flag;//in 1 out 0 default 0
	int dy;

};

class ISLZBuffer
{
public:

	~ISLZBuffer();
	void SetWidthHeight(int width, int height);
	void GetWidthHeight(int& width, int& height);
	void Scan(Model& model);
	Matrix * FaceIndexBufferMatrix;// for display every pix
	bool needUpdate;

private:
	int width, height;

	vector<vector<Polygon>> polygonTable;
	vector<vector<Edge>> edgeTable;
	vector<Polygon> activePolygonTable;
	vector<Edge> activeEdgeTable;
	void release();

	void buildPolygonTable(const Model& model);

	int clipRoundY(float y);// in case moving or scaling over windows
	int clipRoundX(float x);

	bool changePolygonFlag(int polygonId);

	void updateActiveEdge();// move y to y-1 row
	void updateActivePolyon();

};

