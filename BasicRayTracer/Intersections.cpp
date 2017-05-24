#include "Intersections.h"

glm::vec3 getBarycentricWeights(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& point) {
	glm::vec3 res;
	float sideLengths[3] = { glm::distance(p1, p2),
		glm::distance(p2, p3),
		glm::distance(p3, p1) };
	const glm::vec3* verts[4] = { &p1, &p2, &p3, &p1 };

	for (int i = 0; i < 3; i++) {
		float a = sideLengths[i];
		float b = glm::distance(*verts[i], point);
		float c = glm::distance(*verts[i + 1], point);
		float p = (a + b + c) / 2.0f;
		res[i] = max(sqrt(p * (p - a) * (p - b) * (p - c)), 0.0f);
		//res[i] = sqrt(p * (p - a) * (p - b) * (p - c));
	}
	float sum = res.x + res.y + res.z;
	res *= 1.0f / sum;
	return res;
}

glm::vec3 interpolateVecs(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& inter1, const glm::vec3& inter2, const glm::vec3& inter3,
	const glm::vec3& point) {
	glm::vec3 weights = getBarycentricWeights(p1, p2, p3, point);
	return inter3 * weights.x + inter1 * weights.y + inter2 * weights.z;
}

MaterialIO interpolateMaterials(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const MaterialIO* material1, const MaterialIO* material2, const MaterialIO* material3,
	const glm::vec3& point) {
	glm::vec3 weights = getBarycentricWeights(p1, p2, p3, point);
	MaterialIO resMat;
	resMat.diffColor[0] = material3->diffColor[0] * weights.x +
		material1->diffColor[0] * weights.y +
		material2->diffColor[0] * weights.z;
	resMat.diffColor[1] = material3->diffColor[1] * weights.x +
		material1->diffColor[1] * weights.y +
		material2->diffColor[1] * weights.z;
	resMat.diffColor[2] = material3->diffColor[2] * weights.x +
		material1->diffColor[2] * weights.y +
		material2->diffColor[2] * weights.z;

	resMat.ambColor[0] = material3->ambColor[0] * weights.x +
		material1->ambColor[0] * weights.y +
		material2->ambColor[0] * weights.z;
	resMat.ambColor[1] = material3->ambColor[1] * weights.x +
		material1->ambColor[1] * weights.y +
		material2->ambColor[1] * weights.z;
	resMat.ambColor[2] = material3->ambColor[2] * weights.x +
		material1->ambColor[2] * weights.y +
		material2->ambColor[2] * weights.z;

	resMat.specColor[0] = material3->specColor[0] * weights.x +
		material1->specColor[0] * weights.y +
		material2->specColor[0] * weights.z;
	resMat.specColor[1] = material3->specColor[1] * weights.x +
		material1->specColor[1] * weights.y +
		material2->specColor[1] * weights.z;
	resMat.specColor[2] = material3->specColor[2] * weights.x +
		material1->specColor[2] * weights.y +
		material2->specColor[2] * weights.z;

	resMat.emissColor[0] = material3->emissColor[0] * weights.x +
		material1->emissColor[0] * weights.y +
		material2->emissColor[0] * weights.z;
	resMat.emissColor[1] = material3->emissColor[1] * weights.x +
		material1->emissColor[1] * weights.y +
		material2->emissColor[1] * weights.z;
	resMat.emissColor[2] = material3->emissColor[2] * weights.x +
		material1->emissColor[2] * weights.y +
		material2->emissColor[2] * weights.z;

	resMat.shininess = material3->shininess * weights.x +
		material1->shininess * weights.y +
		material2->shininess * weights.z;

	resMat.ktran = material3->ktran * weights.x +
		material1->ktran * weights.y +
		material2->ktran * weights.z;

	return resMat;
}

glm::vec3 getNormal(IntersectionPoint iPoint) {
	glm::vec3 normal;
	const SphereIO* sphereData;
	switch (iPoint.object->type) {
	case SPHERE_OBJ:
		sphereData = (SphereIO*)iPoint.object->data;
		normal = iPoint.position - glm::vec3(sphereData->origin[0], sphereData->origin[1], sphereData->origin[2]);
		break;
	case POLYSET_OBJ:
		const PolygonIO* poly = iPoint.polyIntersect.poly;
		glm::vec3 vert0Pos(poly->vert[0].pos[0], poly->vert[0].pos[1], poly->vert[0].pos[2]);
		glm::vec3 vert1Pos(poly->vert[1].pos[0], poly->vert[1].pos[1], poly->vert[1].pos[2]);
		glm::vec3 vert2Pos(poly->vert[2].pos[0], poly->vert[2].pos[1], poly->vert[2].pos[2]);
		if (((PolySetIO*)(iPoint.object->data))->normType == PER_VERTEX_NORMAL) {
			glm::vec3 vert0Norm(poly->vert[0].norm[0], poly->vert[0].norm[1], poly->vert[0].norm[2]);
			glm::vec3 vert1Norm(poly->vert[1].norm[0], poly->vert[1].norm[1], poly->vert[1].norm[2]);
			glm::vec3 vert2Norm(poly->vert[2].norm[0], poly->vert[2].norm[1], poly->vert[2].norm[2]);
			normal = interpolateVecs(vert0Pos, vert1Pos, vert2Pos,
				vert0Norm, vert1Norm, vert2Norm,
				iPoint.position);
		}
		else {
			normal = glm::cross(vert1Pos - vert0Pos, vert2Pos - vert0Pos);
		}
		break;
	}
	return glm::normalize(normal);
}

IntersectionPoint intersectScene(const glm::vec3& vec, const glm::vec3& origin, const SceneIO* scene) {
	IntersectionPoint finalPoint;
	if (useAcceleration) {
		glm::vec3 inter;
		for (ObjBound* bound : boundBoxes) {
			IntersectionPoint currPoint;
			if (bound->mightIntersect(vec, origin, inter)) {
				currPoint = bound->intersect(vec, origin);
			}
			if (currPoint.object != NULL && (finalPoint.object == NULL ||
				glm::distance2(currPoint.position, origin) < glm::distance2(finalPoint.position, origin))) {
				finalPoint = currPoint;
			}
		}
	}
	else {
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
	}
	return finalPoint;
}