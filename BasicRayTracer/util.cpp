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

glm::vec3 reflect(const glm::vec3& out, const glm::vec3& normal) {
	return glm::normalize(2 * glm::dot(normal, out) * normal - out);
}

glm::vec3 refract(const glm::vec3& out, const glm::vec3& normal, float oldIR, float newIR) {
	float dotProd = glm::dot(out, normal);
	glm::vec3 outNormal = dotProd < 0 ? -1 * normal : normal;

	glm::vec3 in = -1 * out;
	float ratio = (oldIR / newIR);
	float cosine = glm::dot(outNormal, in);
	float sin2 = ratio * ratio * (1.0f - cosine * cosine);
	if (sin2 > 1) return glm::vec3(0);
	return glm::normalize(ratio * in - (ratio * cosine + sqrt(1.0f - sin2)) * outNormal);
}