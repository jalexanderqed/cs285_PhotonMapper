#include "Tracer.h"

#define CLIP_DIST 100

#define LIGHT_NUM 1.0f
#define LIGHT_C1 0.25f
#define LIGHT_C2 0.1f
#define LIGHT_C3 0.01f

#define SHINY_MULT 40

//#define DEBUG

const glm::vec3 BACK_COLOR(0);

vector<Light> lights;

MaterialIO getComplexMaterial(const IntersectionPoint& iPoint) {
	if (iPoint.object->type == SPHERE_OBJ) {

		float u, v;
		calcUVSphere(iPoint, u, v);

		v = 1 - v;
		if (v >= 1) v = 0.999f;

		if (sphereMap.count(iPoint.object) > 0) {
			float sphereInd = sphereMap.at(iPoint.object);
			float rotate = sphereInd / sphereMap.size();
			if (u >= 1.0f - rotate) u += -1 + rotate;
			else u += rotate;
		}

		COLORREF color = texture1.GetPixel((int)(u * texture1.GetWidth()), (int)(v * texture1.GetHeight()));
		if (color == CLR_INVALID) {
			cerr << "Could not load color value at " << 
				(int)(u * texture1.GetWidth()) << ", " <<
				(int)(v * texture1.GetHeight()) << endl;
			cerr << "u: " << u << endl;
			cerr << "v: " << v << endl;
			exit(1);
		}

		glm::vec3 vecColor(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
		MaterialIO mat = dupMaterial(iPoint.object->material);
		float diffColor = (glm::normalize(vecColor).b > 0.5f) ? 0.0f : 1.0f;
		if (sphereMap.count(iPoint.object) > 0) {
			switch (sphereMap.at(iPoint.object)) {
			case 0:
			case 1:
				mat.shininess = (diffColor + 0.25f) / 2;
				mat.diffColor[0] = mat.diffColor[1] = mat.diffColor[2] = diffColor;
				mat.specColor[0] = mat.specColor[1] = mat.specColor[2] = (diffColor + 0.25f) / 2;
				break;
			case 3:
				mat.ktran = (diffColor + 0.8f) / 2;
			case 2:
			case 4:
				mat.diffColor[0] = vecColor.r;
				mat.diffColor[1] = vecColor.g;
				mat.diffColor[2] = vecColor.b;
				break;
			default:

				break;
			}
		}

		return mat;
	}
	else {
		float u, v;
		calcUVPoly(iPoint.polyIntersect, u, v);

		float color = (u + v) / 4;

		MaterialIO mat = dupMaterial(iPoint.object->material);
		mat.diffColor[0] = mat.diffColor[1] = mat.diffColor[2] = color;
		return mat;
	}
}

MaterialIO getMaterial(const IntersectionPoint& iPoint) {
	if (complexColorShaders) {
		return getComplexMaterial(iPoint);
	}

	if (iPoint.object->type == POLYSET_OBJ &&
		iPoint.object->numMaterials > 1 &&
		((PolySetIO*)(iPoint.object->data))->materialBinding == PER_VERTEX_MATERIAL) {
		glm::vec3 p1(iPoint.polyIntersect.poly->vert[0].pos[0],
			iPoint.polyIntersect.poly->vert[0].pos[1],
			iPoint.polyIntersect.poly->vert[0].pos[2]);
		glm::vec3 p2(iPoint.polyIntersect.poly->vert[1].pos[0],
			iPoint.polyIntersect.poly->vert[1].pos[1],
			iPoint.polyIntersect.poly->vert[1].pos[2]);
		glm::vec3 p3(iPoint.polyIntersect.poly->vert[2].pos[0],
			iPoint.polyIntersect.poly->vert[2].pos[1],
			iPoint.polyIntersect.poly->vert[2].pos[2]);
		return interpolateMaterials(p1, p2, p3,
			&iPoint.object->material[iPoint.polyIntersect.poly->vert[0].materialIndex],
			&iPoint.object->material[iPoint.polyIntersect.poly->vert[1].materialIndex],
			&iPoint.object->material[iPoint.polyIntersect.poly->vert[2].materialIndex],
			iPoint.position);
	}
	else {
		return *(iPoint.object->material);
	}
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
	float specularContrib = max(0.0f, pow(glm::dot(lightReflectDir, outDir), shiny * SHINY_MULT));
	contrib += specularContrib * specularColor;

	float atten = distToLight > 0 ?
		min(1.0f, LIGHT_NUM / (LIGHT_C1 + LIGHT_C2 * distToLight + LIGHT_C3 * distToLight * distToLight)) : 1.0f;

	glm::vec3 color = atten * contrib * lightColor;
	return color;
}

glm::vec3 calcAllLights(const IntersectionPoint& iPoint,
	const MaterialIO& interMaterial,
	const glm::vec3& inVec,
	SceneIO* scene) {

	glm::vec3 normal = getNormal(iPoint);
	glm::vec3 diffuse(interMaterial.diffColor[0], interMaterial.diffColor[1], interMaterial.diffColor[2]);
	glm::vec3 specular(interMaterial.specColor[0], interMaterial.specColor[1], interMaterial.specColor[2]);
	glm::vec3 ambient(interMaterial.ambColor[0], interMaterial.ambColor[1], interMaterial.ambColor[2]);
	glm::vec3 emissive(interMaterial.emissColor[0], interMaterial.emissColor[1], interMaterial.emissColor[2]);
	float shiny = interMaterial.shininess;
	float trans = interMaterial.shininess;

	glm::vec3 color = ambient * diffuse * (1.0f - interMaterial.ktran);

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
		MaterialIO ipMaterial;
		glm::vec3 seenColor = l.color;
		int count = 0;
		while (!(ip.object == NULL ||
			(l.sceneLight->type != DIRECTIONAL_LIGHT &&
				glm::distance2(ip.position, l.position) > dist2) ||
				(ipMaterial = getMaterial(ip)).ktran < EPSILON)) {
			count++;
			glm::vec3 objColor(ipMaterial.diffColor[0],
				ipMaterial.diffColor[1],
				ipMaterial.diffColor[2]);
			objColor *= 1.0f / max(objColor.r, max(objColor.g, objColor.b));
			seenColor = seenColor * ipMaterial.ktran * objColor;

			ip = intersectScene(dirToLight, ip.position + EPSILON * dirToLight, scene);
		}

		if (ip.object == NULL ||
			(l.sceneLight->type != DIRECTIONAL_LIGHT &&
				glm::distance2(ip.position, l.position) > dist2) ||
			ipMaterial.ktran > EPSILON) {
			color += lightContrib(seenColor, normal, inVec, dirToLight,
				l.sceneLight->type == DIRECTIONAL_LIGHT ? -1 : glm::distance(l.position, ip.position),
				diffuse, specular, shiny, interMaterial.ktran);
		}
	}
	return color;
}

glm::vec3 shadeIntersect(const IntersectionPoint& iPoint,
	const glm::vec3& inVec,
	SceneIO* scene,
	int inside,
	int depth) {

	glm::vec3 outVec = -1 * inVec;
	MaterialIO interMaterial = getMaterial(iPoint);

	glm::vec3 color = calcAllLights(iPoint, interMaterial, inVec, scene);
	if (depth > 10) return color;
	depth++;

	glm::vec3 normal = glm::normalize(getNormal(iPoint));

	if (interMaterial.ktran > EPSILON) {
		float cosVal = glm::dot(inVec, normal);
		float oldIR, newIR;
		if (cosVal > 0) { // Going out
			inside = max(0, inside - 1);
			oldIR = 1.5f;
			newIR = inside == 0 ? 1.0f : 1.5f;
		}
		else { // Going in
			oldIR = inside == 0 ? 1.0f : 1.5f;
			inside++;
			newIR = 1.5f;
		}

		glm::vec3 refractDir = oldIR == newIR ? inVec : refract(outVec, normal, oldIR, newIR);
		if (glm::length2(refractDir) < EPSILON) { // Total internal reflection
			inside++;
		}
		else {
			float newCosVal = glm::dot(refractDir, normal);
			IntersectionPoint refractIntersect = intersectScene(refractDir,
				iPoint.position + EPSILON * refractDir, scene);
			if (refractIntersect.object != NULL) {
				color += interMaterial.ktran * shadeIntersect(refractIntersect, refractDir, scene, inside, depth);
			}
		}
	}
	if (interMaterial.specColor[0] +
		interMaterial.specColor[1] +
		interMaterial.specColor[2]
		> EPSILON) {

		glm::vec3 spec(interMaterial.specColor[0],
			interMaterial.specColor[1],
			interMaterial.specColor[2]);
		glm::vec3 reflectDir = reflect(outVec, normal);
		IntersectionPoint reflectIntersect = intersectScene(reflectDir,
			iPoint.position + EPSILON * reflectDir, scene);
		if (reflectIntersect.object != NULL) {
			color += spec * shadeIntersect(reflectIntersect, reflectDir, scene, inside, depth);
		}
	}
	return color;
}

glm::vec3 tracePixelVec(const glm::vec3& firstVec, const glm::vec3& camPos, SceneIO* scene) {
	IntersectionPoint iPoint = intersectScene(firstVec, camPos, scene);

	if (iPoint.object != NULL) {
		return shadeIntersect(iPoint, firstVec, scene, 0, 0);
	}
	else {
		return BACK_COLOR;
	}
}

DWORD WINAPI renderLoop(void* params) {
	ThreadData *data = (ThreadData*)params;
	SceneIO* scene = data->scene;
	SceneCamera cam(scene->camera);

	thread_local default_random_engine generator(data->threadNum + time(NULL));
	uniform_real_distribution<float> randomGen(0.0f, 1.0f);

	int lastPercent = 0;
	float invSampPix = 1.0f / SAMPLES_PER_PIXEL;
	glm::vec3 focalPlane = cam.screenPos + cam.focalDistance * cam.forward;

	for (int pixY = data->threadNum; pixY < IMAGE_HEIGHT; pixY += numThreads) {
		for (int pixX = 0; pixX < IMAGE_WIDTH; pixX++) {
			glm::vec3 color(0);
			for (int subY = 0; subY < SAMPLES_PER_PIXEL; subY++) {
				for (int subX = 0; subX < SAMPLES_PER_PIXEL; subX++) {
					glm::vec2 screenSpace;

					if (SAMPLES_PER_PIXEL == 1) {
						screenSpace.x = (pixX + 0.5f) / IMAGE_WIDTH;
						screenSpace.y = (pixY + 0.5f) / IMAGE_HEIGHT;

						glm::vec3 screenPoint = cam.screenPos +
							(2 * screenSpace.x - 1) * cam.screenHoriz +
							(2 * screenSpace.y - 1) * cam.screenVert;

						glm::vec3 pixVec = glm::normalize(screenPoint - cam.pos);

						color = color + tracePixelVec(pixVec, cam.pos, scene) /
							((float)SAMPLES_PER_PIXEL * SAMPLES_PER_PIXEL);
					}
					else {
						screenSpace.x = (pixX + ((float)subX / SAMPLES_PER_PIXEL) + randomGen(generator) * invSampPix) / IMAGE_WIDTH;
						screenSpace.y = (pixY + ((float)subY / SAMPLES_PER_PIXEL) + randomGen(generator) * invSampPix) / IMAGE_HEIGHT;

						glm::vec3 screenPoint = cam.screenPos -
							(2 * screenSpace.x - 1) * cam.screenHoriz -
							(2 * screenSpace.y - 1) * cam.screenVert;

						glm::vec3 calcRay = glm::normalize(cam.lens - screenPoint);
						float d = glm::dot(focalPlane - screenPoint, cam.forward) / glm::dot(calcRay, cam.forward);
						glm::vec3 focalPlanePoint = screenPoint + calcRay * d;

						glm::vec3 lensPoint = cam.lens +
							cam.screenHoriz * (randomGen(generator) * 2 * lensSide - lensSide) +
							cam.screenVert * (randomGen(generator) * 2 * lensSide - lensSide);
						glm::vec3 pixVec = glm::normalize(focalPlanePoint - lensPoint);

						color = color + tracePixelVec(pixVec, lensPoint, scene) / ((float)SAMPLES_PER_PIXEL * SAMPLES_PER_PIXEL);
					}
				}
			}
			setPixel(pixX, pixY, color);

			if (numThreads == 1) {
				float percent = ((float)pixY * IMAGE_WIDTH + pixX) / (IMAGE_WIDTH * IMAGE_HEIGHT);
				if (percent * 100 > lastPercent + 10) {
					lastPercent += 10;
					cerr << lastPercent << "% complete" << endl;
				}
			}
		}
	}
	return 0;
}

void jacksRenderScene(SceneIO* scene) {
	for (LightIO *light = scene->lights; light != NULL; light = light->next) {
		lights.push_back(Light(light));
	}

	if (useAcceleration && numThreads > 1) {
		ThreadData* data = new ThreadData[numThreads];
		HANDLE* threads = new HANDLE[numThreads];
		DWORD* threadIds = new DWORD[numThreads];

		for (int i = 0; i < numThreads; i++) {
			data[i].threadNum = i;
			data[i].scene = scene;

			threads[i] = CreateThread(
				NULL,
				0,
				renderLoop,
				&(data[i]),
				0,
				&(threadIds[i])
			);
		}

		WaitForMultipleObjects(numThreads, threads, true, INFINITE);

		for (int i = 0; i < numThreads; i++) {
			CloseHandle(threads[i]);
		}

		delete[] data;
		delete[] threads;
		delete[] threadIds;
	}
	else {
		numThreads = 1;
		ThreadData data = { 0, scene };
		renderLoop(&data);
	}
}