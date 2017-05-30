#ifndef UTIL_H
#define UTIL_H

#include <atlimage.h>
#include <algorithm>
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "scene_io.h"
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"

extern float EPSILON;

using namespace std;

extern const int IMAGE_WIDTH;
extern const int IMAGE_HEIGHT;

extern float* image;

void setPixel(int x, int y, const glm::vec3& color);

glm::vec3 getPixel(int x, int y);

glm::vec3 reflect(const glm::vec3& out, const glm::vec3& normal);

glm::vec3 refract(const glm::vec3& out, const glm::vec3& normal, float oldIR, float newIR);

#endif