#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "util.h"
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "scene_io.h"

using namespace std;

class BoundingBox {
public:
	glm::vec3 vMax;
	glm::vec3 vMin;

	BoundingBox() : vMin{ glm::vec3(INFINITY) },
		vMax{ glm::vec3(-1 * INFINITY) } { }

	BoundingBox(const PolygonIO* poly);

	BoundingBox(const SphereIO* sphere);

	void apply(const BoundingBox& box);
};

BoundingBox boundScene(SceneIO* scene);

BoundingBox boundPolySet(const PolySetIO* polySet);

#endif // !BOUNDING_BOX_H
