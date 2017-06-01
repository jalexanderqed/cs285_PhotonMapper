#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "util.h"
#include "scene_io.h"
#include <iostream>
#include <cmath>
#include "IntersectionPrimitives.h"
#include "ObjBound.h"

using namespace std;

extern float EPSILON;
extern bool useAcceleration;
extern list<ObjBound*> boundBoxes;
extern bool complexIntersectShaders;

MaterialIO dupMaterial(const MaterialIO* material);

MaterialIO interpolateMaterials(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, 
	const MaterialIO* material1, const MaterialIO* material2, const MaterialIO* material3, 
	const glm::vec3& point);

IntersectionPoint intersectScene(const glm::vec3& vec, const glm::vec3& origin, const SceneIO* scene);

glm::vec3 getNormal(IntersectionPoint iPoint);

#endif