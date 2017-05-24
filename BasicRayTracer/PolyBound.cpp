#include "PolyBound.h"

void PolyBound::addPolygon(const PolygonIO* p) {
	polySet.push_back(p);
	boundBox.apply(BoundingBox(p));
}

PolyBound::~PolyBound() {
	for (PolyBound* p : children) {
		delete p;
	}
}

PolyBound::PolyBound(const ObjIO* p, int d) : parentObject{ p },
depth{ d }
{ }