#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "util.h"
#include <iostream>

using namespace std;

const extern float EPSILON;

class PolyIntersectionPoint {
public:
	glm::vec3 position;
	PolygonIO *poly;

	PolyIntersectionPoint();

	PolyIntersectionPoint(const PolyIntersectionPoint& p);

	PolyIntersectionPoint(const glm::vec3& p, PolygonIO* py);
};

class IntersectionPoint {
public:
	glm::vec3 position;
	ObjIO *object;
	PolyIntersectionPoint polyIntersect;

	IntersectionPoint();

	IntersectionPoint(const glm::vec3& p, ObjIO *o, const PolyIntersectionPoint& pi);
};

IntersectionPoint intersectSphere(const glm::vec3& vec, const glm::vec3& origin, ObjIO* sphere);

PolyIntersectionPoint intersectPoly(const glm::vec3& vec, const glm::vec3& origin, PolygonIO* poly);

IntersectionPoint intersectPolySet(const glm::vec3& vec, const glm::vec3& origin, ObjIO* shape);

IntersectionPoint intersectScene(const glm::vec3& vec, const glm::vec3& origin, SceneIO* scene);

glm::vec3 getNormal(IntersectionPoint iPoint);

#endif