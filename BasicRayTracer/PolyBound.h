#ifndef POLY_BOUND_H
#define POLY_BOUND_H

#include "BoundingBox.h"
#include "scene_io.h"
#include <list>
#include <iostream>

using namespace std;

class PolyBound {
public:
	BoundingBox boundBox;
	int depth;
	list<const PolygonIO*> polySet;
	list<PolyBound*> children;
	const ObjIO* parentObject;

	~PolyBound();

	PolyBound(const ObjIO* p, int d);

	void addPolygon(const PolygonIO* p);

	void split();

	bool mightIntersect(const glm::vec3& vec, const glm::vec3& origin, glm::vec3& res);

	IntersectionPoint intersect(const glm::vec3& vec, const glm::vec3& origin);
};

#endif // !POLY_BOUND_H
