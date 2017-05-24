#ifndef POLY_BOUND_H
#define POLY_BOUND_H

#include "BoundingBox.h"
#include "scene_io.h"
#include <list>

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
};

#endif // !POLY_BOUND_H
