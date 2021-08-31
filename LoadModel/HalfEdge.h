#ifndef HALFEDGE_H
#define HALFEDGE_H

#include <glm/glm.hpp>
#include <assert.h>
#include <list>
#include <map>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

struct HalfEdge;
struct Vert;
struct Face;
struct  HalfEdge
{
	//���ָ��Ķ���
	Vert* vert;
	HalfEdge* next;
	HalfEdge* opposite;
	Face* face;
	bool isBoundary;//�Ƿ��Ǳ߽�
	HalfEdge() :vert(NULL), next(NULL), opposite(NULL), face(NULL), isBoundary(false) {}
};
struct Vert
{
	int id;
	glm::vec3 vcoord;
	glm::vec3 ncoord;
	glm::vec3 tcoord;
	HalfEdge* halfEdge;//�ӵ㷢����һ�����
	bool isBoundary;//�Ƿ��Ǳ߽�
	Vert(int ID = -1, glm::vec3 v = glm::vec3(0, 0, 0), glm::vec3 n = glm::vec3(0, 0, 0), glm::vec3 t = glm::vec3(0, 0, 0)) :id(ID), vcoord(v), ncoord(n), tcoord(t), halfEdge(NULL), isBoundary(false) {}

};

struct  Face
{
	int id;
	HalfEdge* halfEdge;
	Face(HalfEdge* he = NULL) :halfEdge(he) {}
};

struct EdgeKey {
	int v1, v2;
	EdgeKey(int vd1 = -1, int vd2 = -1) :v1(vd1), v2(vd2) {}

	bool operator==(const EdgeKey& key) const { return v1 == key.v1 && v2 == key.v2; }

};
struct EdgeKeyHashFuc
{
	std::size_t operator()(const EdgeKey& key) const
	{
		return std::hash<int>()(key.v1) + std::hash<int>()(key.v2);
	}
};

class TriMesh {

public:
	int save_obj(std::string filename);
	Vert* createVertex(glm::vec3, int id);
	Face* createFace(Vert* vertexs[3]);

	std::vector<Face*> createFaces(Face* face);

	static TriMesh* LoopSubdivison(TriMesh* ori);

	void createBoundary();
	void calculateNormal();
	//������ϵ����е㣬3��
	//std::vector<Vert*> getVertexsFromFace(const Face* face);
	//������ϵ����а�ߣ�3��
	std::vector<HalfEdge*> getEdgesFromFace(const Face* face);
	//��øõ�������ھ�
	std::vector<Vert*> getNeighborVertexs(const Vert* vertex);
	//��øõ�����б߽����ڵ�
	std::vector<Vert*> getBoundaryNeighborVertexs(const Vert* vertex);
	//��õ���������а��
	std::vector<HalfEdge*> getEdgesFromVertex(const Vert* vertex);
	//��øõ��������
	std::vector<Face*> getFacesFromVertex(const Vert* vertex);


	HalfEdge* createEdge(Vert* v1, Vert* v2);

	TriMesh() {}
	const std::vector<HalfEdge*>& HalfEdges() { return m_edges; }
	const std::vector<Vert*>& Vertexs() { return m_vertices; }
	const std::vector<Face*>& Faces() { return m_faces; }
private:
	std::vector<HalfEdge*> m_edges;
	std::vector<Face*> m_faces;
	std::vector<Vert*> m_vertices;


	std::unordered_map<EdgeKey, HalfEdge*, EdgeKeyHashFuc> m_hashmap_edge;
	static EdgeKey getHalfEdgeKey(HalfEdge* he);
	//glm::vec3 calculateFaceNormal(Face* face);

};







#endif