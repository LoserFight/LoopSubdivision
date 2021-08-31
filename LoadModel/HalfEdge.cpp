#include "HalfEdge.h"
#define My_PI 3.1415926

Vert* TriMesh::createVertex(glm::vec3 pos, int id) {
	Vert* ver = new Vert(id, pos);
	m_vertices.push_back(ver);
	return ver;
}

//输入一个三角形的三个顶点数据，生成半边和面
Face* TriMesh::createFace(Vert* vertexs[3]) {
	Face* face = new Face();
	HalfEdge* edges[3];
	//建立一个三角形的半边
	for (int i=0; i < 3; i++) {
		edges[i] = createEdge(vertexs[i % 3], vertexs[(i + 1) % 3]);
		if (edges[i] == NULL) {
			std::cout << "Create Face went wrong" << std::endl;
			return NULL;
		}

	}
	//将对边通过next连接起来，并和该face绑定
	for (int i = 0; i < 3; i++) {
		edges[i]->next = edges[(i + 1) % 3];
		edges[i]->face = face;
		m_edges.push_back(edges[i]);
	}

	//给面赋值任意一个半边
	face->halfEdge = edges[0];
	m_faces.push_back(face);
	return face;
}

HalfEdge* TriMesh::createEdge(Vert* v1, Vert* v2) {
	if (v1 == NULL || v2 == NULL) {
		return NULL;
	}
	EdgeKey key(v1->id, v2->id);
	if (m_hashmap_edge.find(key) != m_hashmap_edge.end())
	{
		//若已经存在
		return m_hashmap_edge[key];
	}
	HalfEdge* HE = new HalfEdge();

	HalfEdge* HE_O = new HalfEdge();
	HE->vert = v2;//记录指向终点
	HE_O->vert = v1;

	HE->opposite = HE_O;
	HE_O->opposite = HE;

	//记录从v1出发的任意半边
	v1->halfEdge = HE;
	m_hashmap_edge[EdgeKey(v1->id, v2->id)] = HE;
	m_hashmap_edge[EdgeKey(v2->id, v1->id)] = HE_O;
	return HE;


}

TriMesh* TriMesh::LoopSubdivison(TriMesh* ori) {
	TriMesh* result = new TriMesh();
	auto& oriVertexs = ori->Vertexs();
	std::vector<Vert*> newVertexs;
	int vid = 0;//新顶点的id
	for (int i = 0; i < oriVertexs.size(); i++) {
		glm::vec3 newPos = { 0.0f,0.0f,0.0f };

		if (!oriVertexs[i]->isBoundary) {
			auto neighborV = ori->getNeighborVertexs(oriVertexs[i]);
			int n = neighborV.size();
			float beta = (5.0 / 8.0 - pow(3.0 / 8.0 + 1.0 / 4.0 * std::cos(2.0 * My_PI / n), 2.0)) / n;
			for (int j=0; j < neighborV.size(); j++) {
				newPos += neighborV[j]->vcoord;

			}
			//计算even points
			newPos = newPos * beta;
			newPos += oriVertexs[i]->vcoord * (1 - n * beta);

		}
		else {
			auto BoundaryNV=ori->getBoundaryNeighborVertexs(oriVertexs[i]);
			newPos = 0.125f * (BoundaryNV[0]->vcoord + BoundaryNV[1]->vcoord) + 0.75f * oriVertexs[i]->vcoord;
		}
		//建立新的点
		Vert* newV=result->createVertex(newPos, vid++);
		newVertexs.push_back(newV);


	}

	//生成新的顶点
	std::unordered_map<EdgeKey, Vert*, EdgeKeyHashFuc> map_ev;//从边到点的映射
	auto &oriEdges = ori->HalfEdges();
	int count = 0;
	for (int i = 0; i < oriEdges.size(); i++) {
		HalfEdge* he = oriEdges[i];
		Vert* v1 = he->opposite->vert;
		Vert* v2 = he->vert;
		EdgeKey key(v1->id, v2->id);
		if (map_ev.find(key) != map_ev.end()) {
		
			continue;
		}
		if (he->isBoundary) {
			glm::vec3 newPos = (v1->vcoord + v2->vcoord) / 2.0f;
			Vert* newVertex = result->createVertex(newPos, vid++);
			//记录点的映射，在下一步中使用
			map_ev[EdgeKey(v1->id, v2->id)] = newVertex;
			map_ev[EdgeKey(v2->id, v1->id)] = newVertex;
			count++;
		}
		else {
			Vert* v3 = he->opposite->next->vert;
			Vert* v4 = he->next->vert;
			glm::vec3 newPos = (v1->vcoord + v2->vcoord)*(3.0f) / 8.0f+
								1.0f/8.0f* (v3->vcoord + v4->vcoord);
			count++;
			Vert* newVertex = result->createVertex(newPos, vid++);
			map_ev[EdgeKey(v1->id, v2->id)] = newVertex;
			map_ev[EdgeKey(v2->id, v1->id)] = newVertex;
		
		}


	}
	//生成全新的mesh
	auto& oriFaces = ori->Faces();
	for (int i = 0; i < oriFaces.size(); i++) {
		Face* face = oriFaces[i];
		HalfEdge* fhe[3];
		fhe[0] = face->halfEdge;
		fhe[1] = fhe[0]->next;
		fhe[2] = fhe[1]->next;
		Vert* center[3];
		for (int j = 0; j < 3; j++) {
			EdgeKey key = getHalfEdgeKey(fhe[j]);
			center[j] = map_ev[key];
			
		}
		result->createFace(center);
		Vert* triVert[3];
		for (int j = 0; j < 3; j++) {

		int oriVid= fhe[j]->next->next->vert->id;
		triVert[0] = newVertexs[oriVid];
		triVert[1] = map_ev[getHalfEdgeKey(fhe[j])];
		triVert[2] = map_ev[getHalfEdgeKey(fhe[(j+2)%3])];
		result->createFace(triVert);
		}

		
	}
	result->createBoundary();
	return result;

}

EdgeKey TriMesh::getHalfEdgeKey(HalfEdge* he)
{
	if (he == NULL)
	{
		return EdgeKey();
	}
	Vert* v1 = he->next->next->vert;
	Vert* v2 = he->vert;
	EdgeKey key(v1->id, v2->id);
	return key;

}

void TriMesh::createBoundary() {
	for (int i = 0; i < m_edges.size(); i++) {
		HalfEdge* he = m_edges[i];
		HalfEdge* heO = he->opposite;
		if (heO->face == NULL) {
			he->isBoundary = true;
			he->vert->isBoundary = true;// 目标点
			he->next->next->vert->isBoundary = true;//起始点
		}

	}
}

std::vector<Vert*> TriMesh::getNeighborVertexs(const Vert* vertex) {
	std::vector<Vert*> result;
	HalfEdge* he = vertex->halfEdge;
	
	HalfEdge* heTemp = he->opposite->next;
	result.push_back(he->vert);
	//HalfEdge* heTemp2 = he;
	while (heTemp != he) {
		//result.push_back(heTemp2->vert);
		
		result.push_back(heTemp->vert);
		heTemp = heTemp->opposite;
		heTemp = heTemp->next;
	
	}
	return result;

}

std::vector<HalfEdge*> TriMesh::getEdgesFromVertex(const Vert* vertex) {
	std::vector<HalfEdge*> result;
	HalfEdge* he = vertex->halfEdge;
	HalfEdge* temp = he;
	do {
		if (temp->isBoundary) {
			result.push_back(temp);
			break;
		}
		result.push_back(temp);
	} while (temp != he);
	
	if (temp->isBoundary) {
		temp = he->next->next;
		//换个方向
		std::reverse(result.begin(), result.end());
		do {
			if (temp->isBoundary || temp == NULL) {
				break;
			}
			result.push_back(temp->opposite);
			temp = temp->opposite->next->next;

		} while (temp->vert->id == vertex->id);//true
		//换回来
		std::reverse(result.begin(), result.end());

	}
	
	return result;

}

std::vector<Face*> TriMesh::getFacesFromVertex(const Vert* vertex)
{
	std::vector<Face*> faces;

	auto edges = getEdgesFromVertex(vertex);

	for (int i = 0; i < edges.size(); i++) {

		faces.push_back(edges[i]->face);

	}

	return faces;
}

std::vector<Vert*>TriMesh::getBoundaryNeighborVertexs(const Vert* vertex) {
	std::vector<Vert*> boundaryVertex;
	auto faces = this->getFacesFromVertex(vertex);
	for (int i = 0; i < faces.size(); i++) {
		auto edges = this->getEdgesFromFace(faces[i]);
		for (int j = 0; j < edges.size(); j++) {
			if (edges[j]->isBoundary) {
				Vert* from = edges[j]->next->next->vert;
				if (edges[j]->vert->id == vertex->id) {
					boundaryVertex.push_back(from);
				}
				else if (from->id == vertex->id)
				{
					boundaryVertex.push_back(edges[j]->vert);
				}

			}
		}
	}
	return boundaryVertex;
}

std::vector<HalfEdge*> TriMesh::getEdgesFromFace(const Face* face) {
	std::vector<HalfEdge*> result;
	HalfEdge* he;
	he = face->halfEdge;
	result.push_back(he);
	he=he->next;
	result.push_back(he);
	he = he->next;
	result.push_back(he);
	return result;
}

void TriMesh::calculateNormal() {
	for (int i = 0; i < m_vertices.size(); i++) {
		glm::vec3 S(0.f, 0.f, 0.f), T(0.f, 0.f, 0.f);
		if (!m_vertices[i]->isBoundary) {
			auto ring = getNeighborVertexs(m_vertices[i]);
			for (int j = 0; j < ring.size(); j++) {
				S += float(std::cos(2 * My_PI * j / ring.size()))*ring[j]->vcoord;
				T += float(std::sin(2 * My_PI * j / ring.size())) * ring[j]->vcoord;
			}
		}
		else {
			auto ringE = getEdgesFromVertex(m_vertices[i]);
			int size = ringE.size();
			S = ringE[size - 1]->vert->vcoord - ringE[0]->vert->vcoord;
			if (size == 2) {
				T = ringE[0]->vert->vcoord + ringE[1]->vert->vcoord - m_vertices[i]->vcoord;

			}
			else if(size==3){
				T = ringE[1]->vert->vcoord - m_vertices[i]->vcoord;

			}
			else if(size==4)
			{
				T = -1.0f * ringE[0]->vert->vcoord + 2.0f * ringE[1]->vert->vcoord +
					2.0f * ringE[2]->vert->vcoord - 1.0f * ringE[3]->vert->vcoord
					- 2.0f * m_vertices[i]->vcoord;

			}
			else {
				float theta = My_PI / float(size - 1);
				T = std::sin(theta) * (ringE[0]->vert->vcoord + ringE[size - 1]->vert->vcoord);
				for (int k = 1; k < size - 1; k++) {
					float wt = (2 * std::cos(theta) - 2) * std::sin((k)*theta);
					T += wt * ringE[k]->vert->vcoord;
				}
				
			
			}
			T = -1.0f * T;
		}
		m_vertices[i]->ncoord = glm::normalize(glm::cross(T, S));

	}
}

int TriMesh::save_obj(std::string filename) {

	std::ofstream OF(filename.c_str());

	//remove vertices
	for (auto viter = m_vertices.begin(); viter != m_vertices.end(); viter++) {
		Vert* v = *viter;
		OF << "v" << " " << v->vcoord.x << " " << v->vcoord.y << " " << v->vcoord.z << "\n";

	}

	for (auto viter = m_vertices.begin(); viter != m_vertices.end(); viter++) {
		Vert* v = *viter;
		OF << "vn" << " " << v->ncoord.x << " " << v->ncoord.y << " " << v->ncoord.z << "\n";
	}

	for (auto fiter = m_faces.begin(); fiter != m_faces.end(); fiter++) {
		Face* f = *fiter;
		OF << "f";

		HalfEdge* he = f->halfEdge;

		do {
			//fprintf(_os, " %d/%d", he->vert->id, he->vert->id);
			OF << " "<< he->vert->id + 1<<"//"<< he->vert->id + 1;
			//fprintf(_os, " %d//%d", he->vert->id + 1, he->vert->id + 1);
			he = he->next;
		} while (he != f->halfEdge);

		OF << "\n";
	}


	OF.close();
	std::cout << "save obj file ok !" << "\n";
	return 0;


}