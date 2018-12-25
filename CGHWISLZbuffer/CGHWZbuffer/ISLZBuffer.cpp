#include "ISLZBuffer.h"

using namespace std;

ISLZBuffer::~ISLZBuffer()
{
	release();
}

void ISLZBuffer::SetWidthHeight(int width, int height)
{
	if (width == this->width && height == this->height)
	{
		return;
	}

	release();

	this->width = width;
	this->height = height;
	needUpdate = true;
	FaceIndexBufferMatrix = new Matrix(height, width);
}

void ISLZBuffer::GetWidthHeight(int& width, int& height)
{
	width = this->width;
	height = this->height;
}

void ISLZBuffer::release()
{
	if ( NULL != FaceIndexBufferMatrix )
	{
		delete FaceIndexBufferMatrix;
		FaceIndexBufferMatrix = NULL;
	}
}

void ISLZBuffer::buildPolygonTable(const Model& model)
{
	polygonTable.clear();
	edgeTable.clear();

	polygonTable.resize(height);
	edgeTable.resize(height);

	int facesSize = model.faces.size();
	for (int i = 0; i < facesSize; i++)
	{
		float minY = height-1;
		float maxY = 0;
		// add all edge into ET according y-index,
		int vertexIndexSize = model.faces[i].vertexIndex.size();
		for (int j = 0; j < vertexIndexSize; j++)
		{
			Point3f pt1 = model.vertexes[model.faces[i].vertexIndex[j]].point;
			Point3f pt2 = model.vertexes[model.faces[i].vertexIndex[(j + 1) % vertexIndexSize]].point;

			if (pt1.y < pt2.y)
			{
				swap(pt1, pt2);
			}
			//pt1 is up 

			Edge edge;
			edge.dy = clipRoundY(pt1.y) - clipRoundY(pt2.y);
			//edge.dy = pt1.y - pt2.y; //this casue blue over line

			//if (  !isEqualf(pt1.y, pt2.y) )//this casue blue over line
			if (edge.dy != 0)
			{
				//edge.dx = ( clipRoundX(pt2.x) - clipRoundX(pt1.x) ) / edge.dy; // this will loss more detail
				edge.dx = (pt2.x - pt1.x) / (pt1.y - pt2.y);
			}
			else
			{
				continue;
				//edge.dx = 0; //this casue blue over line
			}

			//if ( isEqualf(pt1.y, pt2.y) )
			//{
			//	edge.dx = 0;
			//}
			//else 
			//{
			//	edge.dx = (pt2.x - pt1.x) / (pt1.y - pt2.y);
			//	
			//}
			edge.PolygonId = i;
			edge.x = pt1.x;
			//edge.ymax = pt1.y;
			edgeTable[clipRoundY(pt1.y)].push_back(edge);

			minY = min(pt2.y, minY);
			maxY = max(pt1.y, maxY);

		}

		Polygon polygon;// this face
		polygon.PolygonId = i;
		//add face into PT according maxY-index,
		polygon.dy = clipRoundY(maxY) - clipRoundY(minY);
		if (polygon.dy > 0 && maxY > 0 && minY < height) // else continue // ignore this face
		{
			Point3f vertex0 = model.vertexes[ model.faces[i].vertexIndex[0] ].point;
			polygon.a = model.faces[i].normal.x;
			polygon.b = model.faces[i].normal.y;
			polygon.c = model.faces[i].normal.z;
			polygon.d = -(polygon.a * vertex0.x + polygon.b * vertex0.y + polygon.c * vertex0.z);
			polygon.flag = 0;
			//need not color
			polygonTable[clipRoundY(maxY)].push_back(polygon);
		}
	}
}

static bool edgeCompare(const Edge & aEdge, const Edge & bEdge)
{
	if (aEdge.x < bEdge.x)
	{
		return true;
	}
	else if (aEdge.x == bEdge.x)
	{
		if (round(aEdge.dx) < round(bEdge.dx))
		{
			return true;
		}
	}
	return false;
}

int ISLZBuffer::clipRoundX(float x)
{
	if (0 > x)
	{
		return 0;
	}
	if ( x > width-1)
	{
		return width-1;
	}
	return round(x);
}

int ISLZBuffer::clipRoundY(float y)
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

void ISLZBuffer::Scan(Model& model)
{
	clock_t t = clock();

	if (!needUpdate)
	{
		return;
	}

	PointLight light;
	light.ShaderModel(model);// color face

	buildPolygonTable(model);

	activePolygonTable.clear();
	activeEdgeTable.clear();

	FaceIndexBufferMatrix->FillSet( -1);

	for (int y = height - 1; y >= 0; y--)
	{
		//FaceIndexBufferMatrix->FillSetARow(y,-1);

		//add AET
		for (vector<Edge>::iterator it = edgeTable[y].begin(); it != edgeTable[y].end(); it++)
		{
			activeEdgeTable.push_back(*it);
		}
		//add APT
		for (vector<Polygon>::iterator it = polygonTable[y].begin(); it != polygonTable[y].end(); it++)
		{
			activePolygonTable.push_back(*it);
		}

		if (activeEdgeTable.empty())
		{
			continue;  
		}
		else
		{
			sort(activeEdgeTable.begin(), activeEdgeTable.end(), edgeCompare);			
		}

		// scan a row, find every active edge, get in and out point x
		for (vector<Edge>::iterator activeEdgeIt = activeEdgeTable.begin(); activeEdgeIt != activeEdgeTable.end(); activeEdgeIt++)
		{
			bool flagAfter = changePolygonFlag(activeEdgeIt->PolygonId); // find it, change and get new value

			int topZPolygonId = -1;
			float topZ = -0xffffff;
			//find a face(polynog) in z-top , to set color
			for (vector<Polygon>::iterator activePolygonIt = activePolygonTable.begin(); activePolygonIt != activePolygonTable.end(); activePolygonIt++)
			{
				//Polygon & curpolygon = *activePolygonIt;
				if (activePolygonIt->flag) {
					flagAfter = 1; //for second in
					if (0 != activePolygonIt->c) {
						float tmpZ = -(activePolygonIt->a * activeEdgeIt->x + activePolygonIt->b * y + activePolygonIt->d) / activePolygonIt->c;

						if (topZ < tmpZ) {
							topZ = tmpZ;
							topZPolygonId = activePolygonIt->PolygonId;
						}
					}
					//else
					//{
					//	continue;
					//}
				}
				//else
				//{
				//	continue;
				//}
			}

			if( flagAfter && (activeEdgeIt + 1) != activeEdgeTable.end() ) //flagAfter : in
			{ 
				int right = clipRoundX( ( *(activeEdgeIt + 1) ).x );
				for (int left = clipRoundX( activeEdgeIt->x ); left < right; left++)// right is next line
				{
					FaceIndexBufferMatrix->Set(y, left, topZPolygonId);
				}
			}
		}

		updateActiveEdge();
		updateActivePolyon();
	}

	needUpdate = false;
	cout << "cost:" << clock() - t << "ms" << endl;
}

bool ISLZBuffer::changePolygonFlag(int polygonId)
{
	for (vector<Polygon>::iterator it = activePolygonTable.begin(); it != activePolygonTable.end(); it++)
	{
		if ( it->PolygonId == polygonId)
		{
			it->flag = ! it->flag;
			return it->flag;
		}
	}
}

void ISLZBuffer::updateActiveEdge()
{
	int len = 0;
	for (vector<Edge>::iterator it = activeEdgeTable.begin(); it != activeEdgeTable.end(); it++)
	{
		it->dy--;
		if (0 >= it->dy)
		{
			continue;
		}
		else
		{
			it->x += it->dx;
			activeEdgeTable[len] = *it;
			len++;
		}
	}
	activeEdgeTable.resize(len);
}

void ISLZBuffer::updateActivePolyon()
{
	int len = 0;
	for (vector<Polygon>::iterator it = activePolygonTable.begin(); it != activePolygonTable.end(); it++)
	{
		it->dy--;
		if ( 0 >= it->dy) 
		{
			continue;
		}
		else
		{
			activePolygonTable[len] = *it;
			len++;
		}
	}
	activePolygonTable.resize(len);
}