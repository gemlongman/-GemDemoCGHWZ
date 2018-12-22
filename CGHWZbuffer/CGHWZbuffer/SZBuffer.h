#pragma once

#include "PublicStruct.h"


struct Edge
{
	float x;
	float dx;//-1/k
	int dy;//边跨越的扫描线数目
	int PolygonId;//边所属多边形的编号
};

struct ActiveEdge
{
	float x; // 交点的x坐标
	float dx; // (交点边上)两相邻扫描线交点的x坐标之差.-1/k
	int dy; // 以和交点所在边相交的扫描线数为初值，以后向下每处理一条扫描线减1
	float z; // 交点处多边形所在平面的深度值
	float dzx; // 沿扫描线向右走过一个像素时，多边形所在平面的深度增量。对于平面方程，dzx = -a/c(c!=0)
	float dzy; // 沿y方向向下移过一根扫描线时，多边形所在平面的深度增量。对于平面方程，dzy = b/c(c!=0)
	int PolygonId; // 交点对所在的多边形的编号
};

//这里的多边形边表元素，不记录多边形颜色，通过PolygonId查询shader.model中face的color得到
//包括了活化边表
struct Polygon
{
	float a, b, c, d;//face: a^2+b^2+c^2=1，normal :(a,b,c)
	int PolygonId;//多边形编号
	int dy;//多边形跨跃的扫描线数目
	vector<ActiveEdge> activeEdgeTable;
};

class SZBuffer
{
public:

	~SZBuffer();
	void SetWidthHeight(int width, int height);
	void GetWidthHeight(int& width, int& height);
	void Scan(Model& model);
	int** PolygonIdBuffer;//记录多边形的PolygonId，用于查询model.faces[i].color
	bool needUpdate;

private:
	int width, height;
	float* zBuffer;

	vector<vector<Polygon>> polygonTable;
	vector<vector<Edge>> edgeTable;
	vector<Polygon> activePolygonTable;//包括了活化边表

	void release();//释放zBuffer和PolygonIdBuffer
	void buildPolygonTable(const Model& model);//构建多边形表、边表
	void addEdge(int y, Polygon* active_polygon);//将相关边加到活化边表中
	int clipRoundY(float y);// in case moving or scaling over windows
	int clipRoundX(float x);

};

