#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "util.h"
#include "scene_io.h"
#include <iostream>
#include "IntersectionPrimitives.h"
#include "ObjBound.h"

using namespace std;

const extern float EPSILON;
extern bool useAcceleration;
extern list<ObjBound*> boundBoxes;

glm::vec3 getBarycentricWeights(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& point);

glm::vec3 interpolateVecs(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, 
	const glm::vec3& inter1, const glm::vec3& inter2, const glm::vec3& inter3, 
	const glm::vec3& point);

MaterialIO interpolateMaterials(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, 
	const MaterialIO* material1, const MaterialIO* material2, const MaterialIO* material3, 
	const glm::vec3& point);

IntersectionPoint intersectScene(const glm::vec3& vec, const glm::vec3& origin, const SceneIO* scene);

glm::vec3 getNormal(IntersectionPoint iPoint);

#endif