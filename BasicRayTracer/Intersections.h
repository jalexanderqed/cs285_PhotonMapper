#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "util.h"
#include "scene_io.h"
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

glm::vec3 getBarycentricWeights(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& point);

glm::vec3 interpolateVecs(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, 
	const glm::vec3& inter1, const glm::vec3& inter2, const glm::vec3& inter3, 
	const glm::vec3& point);

MaterialIO interpolateMaterials(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, 
	const MaterialIO* material1, const MaterialIO* material2, const MaterialIO* material3, 
	const glm::vec3& point);

IntersectionPoint intersectSphere(const glm::vec3& vec, const glm::vec3& origin, ObjIO* sphere);

PolyIntersectionPoint intersectPoly(const glm::vec3& vec, const glm::vec3& origin, PolygonIO* poly);

IntersectionPoint intersectPolySet(const glm::vec3& vec, const glm::vec3& origin, ObjIO* shape);

IntersectionPoint intersectScene(const glm::vec3& vec, const glm::vec3& origin, SceneIO* scene);

glm::vec3 getNormal(IntersectionPoint iPoint);

#endif