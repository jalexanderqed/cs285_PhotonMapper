#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <algorithm>
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "scene_io.h"
#include "IntersectionPrimitives.h"

using namespace std;

class BoundingBox {
public:
	glm::vec3 vMax;
	glm::vec3 vMin;

	BoundingBox();

	BoundingBox(const PolygonIO* poly);

	BoundingBox(const SphereIO* sphere);

	void apply(const BoundingBox& box);

	bool intersect(const glm::vec3& vec, const glm::vec3& origin, glm::vec3& res);

	bool inside(glm::vec3 point);
};

BoundingBox boundScene(SceneIO* scene);

BoundingBox boundPolySet(const PolySetIO* polySet);

#endif // !BOUNDING_BOX_H
