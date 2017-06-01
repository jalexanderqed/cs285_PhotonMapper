#include "SceneStructure.h"

void jacksBuildBounds(SceneIO* scene) {
	for (ObjIO *object = scene->objects; object != NULL; object = object->next) {
		ObjBound* bound = new ObjBound(object);
		bound->split();
		boundBoxes.push_back(bound);
	}
}

void jacksCleanupBounds() {
	for (ObjBound* o : boundBoxes) {
		delete o;
	}
}