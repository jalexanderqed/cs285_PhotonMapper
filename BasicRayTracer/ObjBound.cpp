#include "ObjBound.h"

ObjBound::~ObjBound() {
	for (PolyBound* p : children) {
		delete p;
	}
}

ObjBound::ObjBound(const ObjIO* o) : obj{ o } {
	if (obj->type == SPHERE_OBJ) {
		boundBox = BoundingBox((SphereIO*)obj->data);
	}
	else {
		boundBox = boundPolySet((PolySetIO*)obj->data);
	}
}

bool ObjBound::mightIntersect(const glm::vec3& vec, const glm::vec3& origin, glm::vec3& res) {
	return boundBox.intersect(vec, origin, res);
}

IntersectionPoint ObjBound::intersect(const glm::vec3& vec, const glm::vec3& origin) {
	if (obj->type == SPHERE_OBJ) {
		return intersectSphere(vec, origin, obj);
	}
	else {
		return intersectPolySet(vec, origin, obj);
	}
}