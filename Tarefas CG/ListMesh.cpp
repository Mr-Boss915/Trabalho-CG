#include "ListMesh.h"
#include <iostream>

using std::cout;

LstVertex::LstVertex(const Vector3& p) : pos_(p) {}
const Vector3& LstVertex::pos() const { return pos_; }

LstEdge::LstEdge(int a, int b) : v0_(a), v1_(b) {}
int LstEdge::v0() const { return v0_; }
int LstEdge::v1() const { return v1_; }

LstFace::LstFace(std::vector<int> vs) : verts_(std::move(vs)) {}
const std::vector<int>& LstFace::verts() const { return verts_; }

int ListMesh::addVertex(const Vector3& p) { vertices_.emplace_back(p); return (int)vertices_.size() - 1; }
int ListMesh::addEdge(int a, int b) { edges_.emplace_back(a, b); return (int)edges_.size() - 1; }
int ListMesh::addFace(const std::vector<int>& vs) { faces_.emplace_back(vs); return (int)faces_.size() - 1; }

const std::vector<LstVertex>& ListMesh::vertices() const { return vertices_; }
const std::vector<LstFace>& ListMesh::faces()    const { return faces_; }

ListMesh ListMesh::MakeTetra() {
	ListMesh M;
	int v0 = M.addVertex({ 0,0,0 });
	int v1 = M.addVertex({ 1,0,0 });
	int v2 = M.addVertex({ 0,1,0 });
	int v3 = M.addVertex({ 0,0,1 });
	M.addEdge(v0, v1); M.addEdge(v0, v2); M.addEdge(v0, v3);
	M.addEdge(v1, v2); M.addEdge(v1, v3); M.addEdge(v2, v3);
	M.addFace({ v0,v2,v1 }); // base
	M.addFace({ v0,v1,v3 });
	M.addFace({ v1,v2,v3 });
	M.addFace({ v2,v0,v3 });
	return M;
}

void ListMesh::print() const {
	cout << "== ListMesh ==\nVertices (" << vertices_.size() << "):\n";
	for (size_t i = 0; i < vertices_.size(); ++i) {
		cout << "  v" << i << " = "; printVector(vertices_[i].pos()); cout << "\n";
	}
	cout << "Edges (" << edges_.size() << "):\n";
	for (size_t i = 0; i < edges_.size(); ++i) {
		cout << "  e" << i << " = (" << edges_[i].v0() << "," << edges_[i].v1() << ")\n";
	}
	cout << "Faces (" << faces_.size() << "):\n";
	for (size_t i = 0; i < faces_.size(); ++i) {
		cout << "  f" << i << " = (";
		const auto& vs = faces_[i].verts();
		for (size_t j = 0; j < vs.size(); ++j) { cout << "v" << vs[j] << (j + 1 < vs.size() ? "," : ""); }
		cout << ")\n";
	}
}