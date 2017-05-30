#include "ObjBound.h"

ObjBound::~ObjBound() {
	if (obj->type == SPHERE_OBJ) {
		delete boundBox;
	}
	else {
		delete myPolyBound;
	}
}

ObjBound::ObjBound(const ObjIO* o) : obj{ o } {
	if (obj->type == SPHERE_OBJ) {
		boundBox = new BoundingBox((SphereIO*)obj->data);
		myPolyBound = nullptr;
	}
	else {
		myPolyBound = new PolyBound(o, 0);
		PolygonIO* poly = ((PolySetIO*)obj->data)->poly;
		for (int i = 0; i < ((PolySetIO*)obj->data)->numPolys; i++, poly++) {
			myPolyBound->addPolygon(poly);
		}
		boundBox = &myPolyBound->boundBox;
	}
}

bool ObjBound::mightIntersect(const glm::vec3& vec, const glm::vec3& origin, glm::vec3& res) {
	return boundBox->intersect(vec, origin, res);
}

void ObjBound::split() {
	if (obj->type == POLYSET_OBJ) {
		myPolyBound->split();
	}
}

IntersectionPoint ObjBound::intersect(const glm::vec3& vec, const glm::vec3& origin) {
	if (obj->type == SPHERE_OBJ) {
		return intersectSphere(vec, origin, obj);
	}
	else {
		return myPolyBound->intersect(vec, origin);
	}
}