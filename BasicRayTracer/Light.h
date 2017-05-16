#ifndef LIGHT_H
#define LIGHT_H

#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"

class Light {
public:
	LightIO* sceneLight;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 color;

	Light(LightIO* l) {
		sceneLight = l;
		position = glm::vec3(l->position[0], l->position[1], l->position[2]);
		direction = glm::vec3(l->direction[0], l->direction[1], l->direction[2]);
		color = glm::vec3(l->color[0], l->color[1], l->color[2]);
	}
};

#endif // !LIGHT_H
