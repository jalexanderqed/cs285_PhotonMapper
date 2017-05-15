#include "Tracer.h"

#define CAMERA_C 1.5
#define CLIP_DIST 100
#define EPSILON 0.00001f

#define LIGHT_NUM 1.0f
#define LIGHT_C1 0.25f
#define LIGHT_C2 0.1f
#define LIGHT_C3 0.01f

#define DEBUG

const glm::vec3 BACK_COLOR(0);

class SceneCamera {
public:
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 forward;
	glm::vec3 pos;
	glm::vec3 screenPos;
	glm::vec3 screenVert;
	glm::vec3 screenHoriz;
	float vertFov;
	float horizFov;

	SceneCamera(CameraIO* camera) {
		up = glm::normalize(glm::vec3(camera->orthoUp[0], camera->orthoUp[1], camera->orthoUp[2]));
		forward = glm::normalize(glm::vec3(camera->viewDirection[0], camera->viewDirection[1], camera->viewDirection[2]));
		right = glm::normalize(glm::cross(forward, up));
		up = glm::normalize(glm::cross(right, forward));
		pos = glm::vec3(camera->position[0], camera->position[1], camera->position[2]);
		screenPos = pos + CAMERA_C * forward;
		vertFov = camera->verticalFOV;
		horizFov = (((float)IMAGE_WIDTH) / IMAGE_HEIGHT) * vertFov;
		screenVert = CAMERA_C * tan(vertFov / 2) * up;
		screenHoriz = CAMERA_C * tan(horizFov / 2) * right;
	}
};

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

vector<Light> lights;

class PolyIntersectionPoint {
public:
	glm::vec3 position;
	PolygonIO *poly;

	PolyIntersectionPoint() :
		poly{ NULL } {}

	PolyIntersectionPoint(const PolyIntersectionPoint& p) :
		position{ p.position }, poly{ p.poly } {}

	PolyIntersectionPoint(const glm::vec3& p, PolygonIO* py) :
		position{ p }, poly{ py } {}
};

class IntersectionPoint {
public:
	glm::vec3 position;
	ObjIO *object;
	PolyIntersectionPoint polyIntersect;

	IntersectionPoint() :
		object{ NULL } {}

	IntersectionPoint(const glm::vec3& p, ObjIO *o, const PolyIntersectionPoint& pi) :
		position{ p }, object{ o }, polyIntersect{ pi } {}
};

IntersectionPoint intersectSphere(const glm::vec3& vec, const glm::vec3& origin, ObjIO* sphere) {
	IntersectionPoint point;

	SphereIO* objData = (SphereIO*)sphere->data;

	glm::vec3 center(objData->origin[0], objData->origin[1], objData->origin[2]);
	glm::vec3 oToC = center - origin;
	float rayProj = glm::dot(vec, oToC);
	if (rayProj < 0) return point;

	float distFromCenter = sqrt(glm::length2(oToC) - rayProj * rayProj);
	if (distFromCenter > objData->radius) return point;
	float halfChordLength = sqrt(objData->radius * objData->radius - distFromCenter * distFromCenter);
	float distToIntersect;
	if (glm::length(oToC) > objData->radius) distToIntersect = rayProj - halfChordLength;
	else distToIntersect = rayProj + halfChordLength;

	point = { (distToIntersect * vec) + origin, sphere, PolyIntersectionPoint() };

	return point;
}

PolyIntersectionPoint intersectPoly(const glm::vec3& vec, const glm::vec3& origin, PolygonIO* poly) {
#ifdef DEBUG
	if (poly->numVertices != 3) {
		cerr << "ERROR: Polygon does not have 3 vertices" << endl;
		exit(1);
	}
#endif // DEBUG

	PolyIntersectionPoint point;
	glm::vec3 vert0(poly->vert[0].pos[0], poly->vert[0].pos[1], poly->vert[0].pos[2]);
	glm::vec3 vert1(poly->vert[1].pos[0], poly->vert[1].pos[1], poly->vert[1].pos[2]);
	glm::vec3 vert2(poly->vert[2].pos[0], poly->vert[2].pos[1], poly->vert[2].pos[2]);

	glm::vec3 edge0 = vert1 - vert0;
	glm::vec3 edge1 = vert2 - vert0;
	glm::vec3 p = glm::cross(vec, edge1);

	float determinant = glm::dot(edge0, p);
	if (determinant > -EPSILON && determinant < EPSILON) return point;
	float invDeterminant = 1 / determinant;

	glm::vec3 vertToOrigin = origin - vert0;

	float u = glm::dot(vertToOrigin, p) * invDeterminant;
	if (u < 0 || u > 1) return point;

	glm::vec3 q = glm::cross(vertToOrigin, edge0);
	float v = glm::dot(q, vec) * invDeterminant;
	if (v < 0 || u + v > 1) return point;

	float t = glm::dot(edge1, q) * invDeterminant;

	if (t > EPSILON) {
		point = { origin + t * vec, poly };
	}

	return point;
}

IntersectionPoint intersectPolySet(const glm::vec3& vec, const glm::vec3& origin, ObjIO* shape) {
	IntersectionPoint finalPoint;

	PolySetIO * polySet = (PolySetIO *)shape->data;

#ifdef DEBUG
	if (polySet->type != POLYSET_TRI_MESH) {
		cerr << "ERROR: Polyset type is not POLYSET_TRI_MESH" << endl;
		exit(1);
	}
#endif // DEBUG

	PolygonIO* poly = polySet->poly;
	for (int i = 0; i < polySet->numPolys; i++, poly++) {
		PolyIntersectionPoint currPoint = intersectPoly(vec, origin, poly);
		if (currPoint.poly != NULL && (finalPoint.object == NULL ||
			glm::distance2(currPoint.position, origin) < glm::distance2(finalPoint.position, origin))) {
			finalPoint = IntersectionPoint(currPoint.position, shape, currPoint);
		}
	}

	return finalPoint;
}

IntersectionPoint intersectScene(const glm::vec3& vec, const glm::vec3& origin, SceneIO* scene) {
	IntersectionPoint finalPoint;
	for (ObjIO *object = scene->objects; object != NULL; object = object->next) {
		IntersectionPoint currPoint;
		switch (object->type) {
		case SPHERE_OBJ:
			currPoint = intersectSphere(vec, origin, object);
			break;
		case POLYSET_OBJ:
			currPoint = intersectPolySet(vec, origin, object);
			break;
		default:
			cerr << "ERROR: Unrecognized object type in intersectScene" << endl;
			exit(1);
		}
		if (currPoint.object != NULL && (finalPoint.object == NULL ||
			glm::distance2(currPoint.position, origin) < glm::distance2(finalPoint.position, origin))) {
			finalPoint = currPoint;
		}
	}
	return finalPoint;
}

glm::vec3 getNormal(const IntersectionPoint& iPoint) {
	glm::vec3 normal;
	SphereIO* sphereData;
	switch (iPoint.object->type) {
	case SPHERE_OBJ:
		sphereData = (SphereIO*)iPoint.object->data;
		normal = glm::normalize(iPoint.position - glm::vec3(sphereData->origin[0], sphereData->origin[1], sphereData->origin[2]));
		break;
	case POLYSET_OBJ:
		PolygonIO* poly = iPoint.polyIntersect.poly;
		glm::vec3 vert0(poly->vert[0].pos[0], poly->vert[0].pos[1], poly->vert[0].pos[2]);
		glm::vec3 vert1(poly->vert[1].pos[0], poly->vert[1].pos[1], poly->vert[1].pos[2]);
		glm::vec3 vert2(poly->vert[2].pos[0], poly->vert[2].pos[1], poly->vert[2].pos[2]);
		normal = glm::normalize(glm::cross(vert1 - vert0, vert2 - vert0));
		break;
	}
	return normal;
}

glm::vec3 reflect(const glm::vec3& out, const glm::vec3& normal) {
	return glm::normalize(2 * glm::dot(normal, out) * normal - out);
}

glm::vec3 lightContrib(const glm::vec3& lightColor,
	const glm::vec3& normal, const glm::vec3& inDir,
	const glm::vec3& dirToLight, float distToLight,
	const glm::vec3& diffuseColor, const glm::vec3& specularColor,
	float shiny, float kTrans) {
	glm::vec3 contrib;

	glm::vec3 outDir = -1 * inDir;

	float diffuseContrib = max(0.0f, glm::dot(normal, dirToLight));
	contrib += (1.0f - kTrans) * diffuseContrib * diffuseColor;

	glm::vec3 lightReflectDir = reflect(dirToLight, normal);
	float specularContrib = max(0.0f, pow(glm::dot(lightReflectDir, outDir), shiny * 40));
	contrib += specularContrib * specularColor;

	float atten = distToLight > 0 ?
		min(1.0f, LIGHT_NUM / (LIGHT_C1 + LIGHT_C2 * distToLight + LIGHT_C3 * distToLight * distToLight)) : 1.0f;

	glm::vec3 color = atten * contrib * lightColor;
	return color;
}

glm::vec3 calcAllLights(const IntersectionPoint& iPoint,
	const glm::vec3& inVec,
	SceneIO* scene) {

	glm::vec3 normal = getNormal(iPoint);
	glm::vec3 diffuse(iPoint.object->material->diffColor[0], iPoint.object->material->diffColor[1], iPoint.object->material->diffColor[2]);
	glm::vec3 specular(iPoint.object->material->specColor[0], iPoint.object->material->specColor[1], iPoint.object->material->specColor[2]);
	glm::vec3 ambient(iPoint.object->material->ambColor[0], iPoint.object->material->ambColor[1], iPoint.object->material->ambColor[2]);
	glm::vec3 emissive(iPoint.object->material->emissColor[0], iPoint.object->material->emissColor[1], iPoint.object->material->emissColor[2]);
	float shiny = iPoint.object->material->shininess;
	float trans = iPoint.object->material->shininess;

	glm::vec3 color = ambient * diffuse * (1.0f - iPoint.object->material->ktran);

	for (Light l : lights) {
		glm::vec3 dirToLight;
		float dist2;

		switch (l.sceneLight->type) {
		case SPOT_LIGHT:
		case POINT_LIGHT:
			dirToLight = l.position - iPoint.position;
			dist2 = glm::length2(dirToLight);
			break;
		case DIRECTIONAL_LIGHT:
			dirToLight = -1 * l.direction;
			break;
		}

		dirToLight = glm::normalize(dirToLight);

		IntersectionPoint ip = intersectScene(dirToLight, iPoint.position + EPSILON * dirToLight, scene);
		glm::vec3 seenColor = l.color;
		int count = 0;
		while (!(ip.object == NULL ||
			(l.sceneLight->type != DIRECTIONAL_LIGHT &&
				glm::distance2(ip.position, l.position) > dist2) ||
			ip.object->material->ktran < EPSILON)) {

			count++;
			glm::vec3 objColor(ip.object->material->diffColor[0],
				ip.object->material->diffColor[1],
				ip.object->material->diffColor[2]);
			objColor *= 1.0f / max(objColor.r, max(objColor.g, objColor.b));
			seenColor = seenColor * ip.object->material->ktran * objColor;

			ip = intersectScene(dirToLight, ip.position + EPSILON * dirToLight, scene);
		}

		if (ip.object == NULL ||
			(l.sceneLight->type != DIRECTIONAL_LIGHT &&
				glm::distance2(ip.position, l.position) > dist2) ||
			ip.object->material->ktran > EPSILON) {
			color += lightContrib(seenColor, normal, inVec, dirToLight,
				l.sceneLight->type == DIRECTIONAL_LIGHT ? -1 : glm::distance(l.position, ip.position),
				diffuse, specular, shiny, iPoint.object->material->ktran);
		}
	}
	return color;
}

glm::vec3 reflectRefractRecurse(const IntersectionPoint& iPoint,
	const glm::vec3& inVec,
	SceneIO* scene,
	vector<ObjIO*>& inside,
	int depth) {

	if (iPoint.object->numMaterials > 1) {
		cerr << "ERROR: More than one material not supported." << endl;
		return glm::vec3();
	}

	glm::vec3 color = calcAllLights(iPoint, inVec, scene);
	//if (depth > 5) return color;
	depth++;

	glm::vec3 normal = getNormal(iPoint);

	if (iPoint.object->material->ktran > EPSILON) {

	}
	if (iPoint.object->material->specColor[0] +
		iPoint.object->material->specColor[1] +
		iPoint.object->material->specColor[2]
		> EPSILON){

		glm::vec3 spec(iPoint.object->material->specColor[0],
		iPoint.object->material->specColor[1],
		iPoint.object->material->specColor[2]);
		glm::vec3 reflectDir = reflect(-1 * inVec, normal);
		IntersectionPoint reflectIntersect = intersectScene(reflectDir,
			iPoint.position + EPSILON * reflectDir, scene);
		if (reflectIntersect.object != NULL) {
			color += spec * reflectRefractRecurse(reflectIntersect, reflectDir, scene, inside, depth);
		}
	}
	return color;
}

glm::vec3 tracePixelVec(const glm::vec3& firstVec, const glm::vec3& camPos, SceneIO* scene) {
	IntersectionPoint iPoint = intersectScene(firstVec, camPos, scene);

	if (iPoint.object != NULL) {
		vector<ObjIO*> inside;
		return reflectRefractRecurse(iPoint, firstVec, scene, inside, 0);
	}
	else {
		return BACK_COLOR;
	}
}

void jacksRenderScene(SceneIO* scene) {
	SceneCamera cam(scene->camera);

	for (LightIO *light = scene->lights; light != NULL; light = light->next) {
		lights.push_back(Light(light));
	}

	int lastPercent = 0;

	for (int pixY = 0; pixY < IMAGE_HEIGHT; pixY++) {
		for (int pixX = 0; pixX < IMAGE_WIDTH; pixX++) {
			glm::vec2 screenSpace(
				(pixX + (1.0f / 2)) / IMAGE_WIDTH,
				(pixY + (1.0f / 2)) / IMAGE_HEIGHT
			);
			glm::vec3 screenPoint = cam.screenPos +
				(2 * screenSpace.x - 1) * cam.screenHoriz +
				(2 * screenSpace.y - 1) * cam.screenVert;
			glm::vec3 pixVec = glm::normalize(screenPoint - cam.pos);
			glm::vec3 color = tracePixelVec(pixVec, cam.pos, scene);
			//if (color.r > 1 && color.g > 1 && color.b > 1) color = glm::vec3(0.7, 0, 0.7);
			setPixel(pixX, pixY, color);

			float percent = ((float)pixY * IMAGE_WIDTH + pixX) / (IMAGE_WIDTH * IMAGE_HEIGHT);
			if (percent * 100 > lastPercent + 10) {
				lastPercent += 10;
				cout << lastPercent << "% complete" << endl;
			}
		}
	}
}