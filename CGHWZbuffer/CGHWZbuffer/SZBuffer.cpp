#include "SZBuffer.h"

using namespace std;

SZBuffer::~SZBuffer()
{
	release();
}

void SZBuffer::SetWidthHeight(int width, int height)
{
	if (width == this->width && height == this->height)
	{
		return;
	}

	release();

	this->width = width;
	this->height = height;
	needUpdate = true;
	zBuffer = new float[width];

	PolygonIdBuffer = new int*[height];
	for (int i = 0; i < height; i++)
	{
		PolygonIdBuffer[i] = new int[width];
	}
}

void SZBuffer::GetWidthHeight(int& width, int& height)
{
	width = this->width;
	height = this->height;
}

void SZBuffer::release()
{
	if (zBuffer != NULL)
	{
		delete[] zBuffer;
		zBuffer = NULL;
	}

	if (PolygonIdBuffer != NULL)
	{
		for (int i = 0; i < height; i++)
		{
			delete[] PolygonIdBuffer[i];
			PolygonIdBuffer[i] = NULL;
		}
	}
	delete[] PolygonIdBuffer;
	PolygonIdBuffer = NULL;
}

void SZBuffer::buildPolygonTable(const Model& model)
{
	polygonTable.clear();
	edgeTable.clear();

	polygonTable.resize(height);
	edgeTable.resize(height);


	int facesSize = model.faces.size();
	for (int i = 0; i < facesSize; i++)
	{
		float minY = height;
		float maxY = 0;

		Polygon polygon;
		polygon.PolygonId = i;
		//构建分类边表
		const vector<int> & vertexIndex = model.faces[i].vertexIndex;
		for (int j = 0, vsize = vertexIndex.size(); j < vsize; j++)
		{
			//比较得到，边的上端点pt1
			Point3f pt1 = model.vertexes[vertexIndex[j]].point;
			Point3f pt2 = model.vertexes[vertexIndex[(j + 1) % vsize]].point;
			if (pt1.y < pt2.y)
			{
				swap(pt1, pt2);
			}

			Edge edge;
			edge.dy = clipRoundY(pt1.y) - clipRoundY(pt2.y);
			if (edge.dy <= 0) 
			{
				continue;
			}

			edge.x = pt1.x;
			edge.PolygonId = polygon.PolygonId;
			edge.dx = -(pt1.x - pt2.x) / (pt1.y - pt2.y);

			edgeTable[clipRoundY(pt1.y)].push_back(edge);

			minY = min(pt2.y, minY);
			maxY = max(pt1.y, maxY);

		}

		//构建分类多边形表
		polygon.dy = clipRoundY(maxY) - clipRoundY(minY);
		if (polygon.dy > 0 && maxY > 0 && minY < height)
		{
			Point3f v = model.vertexes[ model.faces[i].vertexIndex[0] ].point;
			polygon.a = model.faces[i].normal.x;
			polygon.b = model.faces[i].normal.y;
			polygon.c = model.faces[i].normal.z;
			polygon.d = -(polygon.a*v.x + polygon.b*v.y + polygon.c*v.z);

			polygonTable[clipRoundY(maxY) ].push_back(polygon);
		}
	}
}

static bool edgeCampare(const ActiveEdge & aEdge, const ActiveEdge & bEdge)
{
	if (round(aEdge.x) < round(bEdge.x))
	{
		return true;
	}
	else if (round(aEdge.x) == round(bEdge.x))
	{
		if (round(aEdge.dx) < round(bEdge.dx))
		{
			return true;
		}
	}
	return false;
}

void SZBuffer::addEdge(int y, Polygon* activePolygon)
{
	//把该多边形在oxy平面上的投影和扫描线相交的边加入到活化边表中
	for (vector<Edge>::iterator it = edgeTable[y].begin(); it != edgeTable[y].end();it++)
	{
		if (it->PolygonId != activePolygon->PolygonId)
		{
			continue;
		}

		ActiveEdge activeEdge;
		activeEdge.x = it->x;
		activeEdge.dx = it->dx;
		activeEdge.dy = it->dy;

		if (isEqualf(activePolygon->c, 0))
		{
			activeEdge.z = 0;
			activeEdge.dzx = 0;
			activeEdge.dzy = 0;
		}
		else
		{
			activeEdge.z = -(activePolygon->d + activePolygon->a * it->x + activePolygon->b * y) / activePolygon->c;
			activeEdge.dzx = -(activePolygon->a / activePolygon->c);
			activeEdge.dzy = activePolygon->b / activePolygon->c;
		}

		activePolygon->activeEdgeTable.push_back(activeEdge);
		it->PolygonId = -1;
	}

	sort(activePolygon->activeEdgeTable.begin(), activePolygon->activeEdgeTable.end(), edgeCampare);
}

int SZBuffer::clipRoundX(float x)
{
	if (0 > x)
	{
		return 0;
	}
	if ( x > width)
	{
		return width;
	}
	return round(x);
}

int SZBuffer::clipRoundY(float y)
{
	if (0 > y)
	{
		return 0;
	}
	if (y > height-1)
	{
		return height-1;// watch out ! otherwise buildPolygonTable crush at edgeTable[clipRoundY(pt1.y)].push_back(edge);
	}
	return round(y);
}

void SZBuffer::Scan(Model& model)
{
	clock_t t = clock();

	if (!needUpdate) 
	{ 
		return;
	}

	PointLight light;
	light.ShaderModel(model);// color it

	buildPolygonTable(model);
	activePolygonTable.clear();

	//扫描线从上往下进行扫描
	for (int y = height - 1; y >= 0; y--)
	{
		memset(PolygonIdBuffer[y], -1, sizeof(int)*width);
		fill(zBuffer, zBuffer + width, -0xfffffff);

		//polygonTable[y].size();
		//检查分类的多边形表，如果有新的多边形涉及该扫描线，则把它放入活化的多边形表中
		for (vector<Polygon>::iterator it = polygonTable[y].begin(); it != polygonTable[y].end(); it++)
		{
			activePolygonTable.push_back(*it);
		}
			
		int activePolygonTableSize = activePolygonTable.size();
		for (int i = 0; i < activePolygonTableSize; i++)
		{
			Polygon & activePolygon = activePolygonTable[i];
			addEdge(y, &activePolygon);

			vector<ActiveEdge>& activeEdgeTable = activePolygon.activeEdgeTable;

			int activeEdgeSize = activeEdgeTable.size();
			for (vector<ActiveEdge>::iterator activeEdgeIt = activeEdgeTable.begin(), end = activeEdgeTable.end();
				activeEdgeIt != end; activeEdgeIt++)
			{
				//取出活化边对
				ActiveEdge & edge1 = *activeEdgeIt;
				activeEdgeIt++;
				ActiveEdge & edge2 = *activeEdgeIt;

				float zx = edge1.z;
				//更新z-buffer和PolygonId缓冲器
				for (int x = clipRoundX(edge1.x), end = clipRoundX(edge2.x); x < end; x++) // x: 0-with
				{
					if (zx > zBuffer[x])
					{
						zBuffer[x] = zx;
						PolygonIdBuffer[y][x] = activePolygon.PolygonId;
					}
					zx += edge1.dzx;
				}
				//进入下一条扫描线，更新活化边参数
				edge1.dy--;
				edge2.dy--;
				edge1.x += edge1.dx;
				edge2.x += edge2.dx;
				edge1.z += edge1.dzx * edge1.dx + edge1.dzy;
				edge2.z += edge2.dzx * edge2.dx + edge2.dzy;
			}

			//移除已经结束的活化边
			int last = 0;
			activeEdgeSize = activeEdgeTable.size();
			for (int j = 0; j < activeEdgeSize; j++)
			{
				if ( 0 >= activeEdgeTable[j].dy)
				{
					continue;	
				}
				activeEdgeTable[last] = activeEdgeTable[j];
				last++;
			}
			activeEdgeTable.resize(last);

			activePolygon.dy--;//活化多边形扫描线向下移动
		}

		//移除已经完成的活化多边形
		int last = 0;
		for (int i = 0, len = activePolygonTable.size(); i < len; i++)
		{
			if(0 >= activePolygonTable[i].dy)
			{
				continue;
			}
			activePolygonTable[last] = activePolygonTable[i];
			last++;
		}
		activePolygonTable.resize(last);
	}

	needUpdate = false;
	cout << "cost:" << clock() - t << "ms" << endl;
}


