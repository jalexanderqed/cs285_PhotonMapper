#ifndef INTERSECTION_PRIMITIVES_H
#define INTERSECTION_PRIMITIVES_H

#define _USE_MATH_DEFINES

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "scene_io.h"
#include <cmath>
#include <algorithm>
#include <Windows.h>
#include <iostream>
#include <atlimage.h>
#include <unordered_map>

using namespace std;

extern CImage texture1;
extern unordered_map<const ObjIO*, int> sphereMap;

extern float EPSILON;
extern bool complexIntersectShaders;

glm::vec3 getBarycentricWeights(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& point);

glm::vec3 interpolateVecs(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& inter1, const glm::vec3& inter2, const glm::vec3& inter3,
	const glm::vec3& point);

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

void calcUVPoly(const PolyIntersectionPoint& iPoint, float& u, float& v);

void calcUVSphere(const IntersectionPoint& iPoint, float& u, float& v);

IntersectionPoint intersectSphere(const glm::vec3& vec, const glm::vec3& origin, const ObjIO* sphere);

PolyIntersectionPoint intersectPoly(const glm::vec3& vec, const glm::vec3& origin, const PolygonIO* poly);

IntersectionPoint intersectPolySet(const glm::vec3& vec, const glm::vec3& origin, const ObjIO* shape);

#endif // !INTERSECTION_PRIMITIVES_H
