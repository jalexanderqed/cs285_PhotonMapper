#include "BoundingBox.h"

BoundingBox::BoundingBox(const PolygonIO* poly) {
	for (int i = 0; i < 3; i++) {
		vMax[i] = max(poly->vert[0].pos[i],
			max(poly->vert[1].pos[i], poly->vert[2].pos[i]));
		vMin[i] = min(poly->vert[0].pos[i],
			min(poly->vert[1].pos[i], poly->vert[2].pos[i]));
	}
}

BoundingBox::BoundingBox(const SphereIO* sphere) {
	for (int i = 0; i < 3; i++) {
		vMax[i] = sphere->origin[i] + sphere->radius;
		vMin[i] = sphere->origin[i] - sphere->radius;
	}
}

void BoundingBox::apply(const BoundingBox& box) {
	for (int i = 0; i < 3; i++) {
		vMax[i] = max(vMax[i], box.vMax[i]);
		vMin[i] = min(vMin[i], box.vMin[i]);
	}
}

BoundingBox boundPolySet(const PolySetIO* polySet) {
	BoundingBox res;
	PolygonIO* poly = polySet->poly;
	for (int i = 0; i < polySet->numPolys; i++, poly++) {
		res.apply(BoundingBox(poly));
	}
	return res;
}

BoundingBox boundScene(SceneIO* scene) {
	BoundingBox res;
	for (ObjIO *object = scene->objects; object != NULL; object = object->next) {
		IntersectionPoint currPoint;
		if (object->type == SPHERE_OBJ) {
			res.apply(BoundingBox((SphereIO *)(object->data)));
		}
		else {
			res.apply(boundPolySet((PolySetIO*)(object->data)));
		}
	}
	return res;
}