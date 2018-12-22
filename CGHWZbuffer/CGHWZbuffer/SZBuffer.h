#pragma once

#include "PublicStruct.h"


struct Edge
{
	float x;
	float dx;//-1/k
	int dy;//�߿�Խ��ɨ������Ŀ
	int PolygonId;//����������εı��
};

struct ActiveEdge
{
	float x; // �����x����
	float dx; // (�������)������ɨ���߽����x����֮��.-1/k
	int dy; // �Ժͽ������ڱ��ཻ��ɨ������Ϊ��ֵ���Ժ�����ÿ����һ��ɨ���߼�1
	float z; // ���㴦���������ƽ������ֵ
	float dzx; // ��ɨ���������߹�һ������ʱ�����������ƽ����������������ƽ�淽�̣�dzx = -a/c(c!=0)
	float dzy; // ��y���������ƹ�һ��ɨ����ʱ�����������ƽ����������������ƽ�淽�̣�dzy = b/c(c!=0)
	int PolygonId; // ��������ڵĶ���εı��
};

//����Ķ���α߱�Ԫ�أ�����¼�������ɫ��ͨ��PolygonId��ѯshader.model��face��color�õ�
//�����˻�߱�
struct Polygon
{
	float a, b, c, d;//face: a^2+b^2+c^2=1��normal :(a,b,c)
	int PolygonId;//����α��
	int dy;//����ο�Ծ��ɨ������Ŀ
	vector<ActiveEdge> activeEdgeTable;
};

class SZBuffer
{
public:

	~SZBuffer();
	void SetWidthHeight(int width, int height);
	void GetWidthHeight(int& width, int& height);
	void Scan(Model& model);
	int** PolygonIdBuffer;//��¼����ε�PolygonId�����ڲ�ѯmodel.faces[i].color
	bool needUpdate;

private:
	int width, height;
	float* zBuffer;

	vector<vector<Polygon>> polygonTable;
	vector<vector<Edge>> edgeTable;
	vector<Polygon> activePolygonTable;//�����˻�߱�

	void release();//�ͷ�zBuffer��PolygonIdBuffer
	void buildPolygonTable(const Model& model);//��������α��߱�
	void addEdge(int y, Polygon* active_polygon);//����ر߼ӵ���߱���
	int clipRoundY(float y);// in case moving or scaling over windows
	int clipRoundX(float x);

};

