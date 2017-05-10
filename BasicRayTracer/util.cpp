#include "util.h"

void setPixel(int x, int y, const glm::vec3& color) {
	*(image + 3 * IMAGE_WIDTH * y + x * 3 + 0) = color.r;
	*(image + 3 * IMAGE_WIDTH * y + x * 3 + 1) = color.g;
	*(image + 3 * IMAGE_WIDTH * y + x * 3 + 2) = color.b;
}

glm::vec3 getPixel(int x, int y) {
	return glm::vec3(*(image + 3 * IMAGE_WIDTH * y + x * 3 + 0),
		*(image + 3 * IMAGE_WIDTH * y + x * 3 + 1),
		*(image + 3 * IMAGE_WIDTH * y + x * 3 + 2));
}