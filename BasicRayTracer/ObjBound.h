#ifndef OBJ_BOUND_H
#define OBJ_BOUND_H

#include "BoundingBox.h"
#include "scene_io.h"
#include "PolyBound.h"
#include <list>
#include "IntersectionPrimitives.h"

class ObjBound {
public:
	BoundingBox* boundBox;
	PolyBound* myPolyBound;
	const ObjIO* obj;

	~ObjBound();

	ObjBound(const ObjIO* o);

	bool mightIntersect(const glm::vec3& vec, const glm::vec3& origin, glm::vec3& res);

	void split();

	IntersectionPoint intersect(const glm::vec3& vec, const glm::vec3& origin);
};

#endif // !OBJ_BOUND_H
