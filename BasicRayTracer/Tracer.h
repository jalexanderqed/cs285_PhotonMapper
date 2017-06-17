#ifndef TRACER_H
#define TRACER_H

#define _USE_MATH_DEFINES

#include "util.h"
#include "Intersections.h"
#include "SceneCamera.h"
#include "Light.h"
#include "scene_io.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "SceneStructure.h"
#include <unordered_map>

using namespace std;

extern const int IMAGE_WIDTH;
extern const int IMAGE_HEIGHT;
extern const int SAMPLES_PER_PIXEL;
extern float EPSILON;

extern const uint64_t NUM_PHOTONS;
extern float photonSearchRadius;
extern float dOmega;

extern float lensSide;
extern int numThreads;
extern bool complexColorShaders;
extern unordered_map<const ObjIO*, int> sphereMap;

extern BoundingBox sceneBox;

extern CImage texture1;

extern list<ObjBound*> boundBoxes;

void jacksRenderScene(SceneIO* scene);

void jacksTracePhotons(SceneIO* scene);

struct ThreadData {
	int threadNum;
	SceneIO* scene;
	int light;
};

#endif // !TRACER_H
