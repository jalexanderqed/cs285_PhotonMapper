#ifndef TRACER_H
#define TRACER_H

#include "util.h"
#include "Intersections.h"
#include "SceneCamera.h"
#include "Light.h"
#include "scene_io.h"
#include <iostream>
#include <vector>
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "SceneStructure.h"

using namespace std;

extern const int IMAGE_WIDTH;
extern const int IMAGE_HEIGHT;
const extern float EPSILON;

extern list<ObjBound*> boundBoxes;

void jacksRenderScene(SceneIO* scene);

#endif // !TRACER_H
