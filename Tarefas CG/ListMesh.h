#pragma once
#include <vector>
#include <utility>
#include "Vector3.h"

class LstVertex {
    Vector3 pos_;
public:
    explicit LstVertex(const Vector3& p = {});
    const Vector3& pos() const;
};

class LstEdge {
    int v0_{ -1 }, v1_{ -1 };
public:
    LstEdge(int a = -1, int b = -1);
    int v0() const;
    int v1() const;
};

class LstFace {
	std::vector<int> verts_;   // loop antihorrario dos vertices
public:
    explicit LstFace(std::vector<int> vs = {});
    const std::vector<int>& verts() const;
};

class ListMesh {
    std::vector<LstVertex> vertices_;
    std::vector<LstEdge>   edges_;
    std::vector<LstFace>   faces_;
public:
    int addVertex(const Vector3& p);
    int addEdge(int a, int b);
    int addFace(const std::vector<int>& vs);

    static ListMesh MakeTetra();

    const std::vector<LstVertex>& vertices() const;
    const std::vector<LstFace>& faces()    const;

    void print() const;
};