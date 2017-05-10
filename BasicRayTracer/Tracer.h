#ifndef TRACER_H
#define TRACER_H

#include "util.h"
#include "scene_io.h"
#include <iostream>
#include <vector>

using namespace std;

extern const int IMAGE_WIDTH;
extern const int IMAGE_HEIGHT;

void jacksRenderScene(SceneIO* scene);

#endif // !TRACER_H
