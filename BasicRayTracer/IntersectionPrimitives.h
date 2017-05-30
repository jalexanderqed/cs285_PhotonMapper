#ifndef INTERSECTION_PRIMITIVES_H
#define INTERSECTION_PRIMITIVES_H

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "scene_io.h"

extern float EPSILON;

class PolyIntersectionPoint {
public:
	glm::vec3 position;
	const PolygonIO *poly;

	PolyIntersectionPoint();

	PolyIntersectionPoint(const PolyIntersectionPoint& p);

	PolyIntersectionPoint(const glm::vec3& p, const PolygonIO* py);
};

class IntersectionPoint {
public:
	glm::vec3 position;
	const ObjIO *object;
	PolyIntersectionPoint polyIntersect;

	IntersectionPoint();

	IntersectionPoint(const glm::vec3& p, const ObjIO *o, const PolyIntersectionPoint& pi);
};

IntersectionPoint intersectSphere(const glm::vec3& vec, const glm::vec3& origin, const ObjIO* sphere);

PolyIntersectionPoint intersectPoly(const glm::vec3& vec, const glm::vec3& origin, const PolygonIO* poly);

IntersectionPoint intersectPolySet(const glm::vec3& vec, const glm::vec3& origin, const ObjIO* shape);

#endif // !INTERSECTION_PRIMITIVES_H
