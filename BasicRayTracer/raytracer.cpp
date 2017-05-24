#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <atlimage.h>
#include <algorithm>
#include "scene_io.h"
#include "Timer.h"
#include "util.h"
#include "Tracer.h"
#include "lib\glm\glm.hpp"
#include "lib\glm\ext.hpp"
#include "SceneStructure.h"
#include <list>
#include "PolyBound.h"

const int IMAGE_WIDTH = 400;
const int IMAGE_HEIGHT = 400;
const float EPSILON = 0.00001f;

using namespace std;

float *image;

SceneIO *scene = NULL;
list<ObjBound*> boundBoxes;
LPCTSTR fileName;
bool useAcceleration = true;

static void loadScene(char *name) {
	/* load the scene into the SceneIO data structure using given parsing code */
	scene = readScene(name);

	/* hint: use the Visual Studio debugger ("watch" feature) to probe the
	   scene data structure and learn more about it for each of the given scenes */
	jacksBuildBounds(scene);

	/* write any code to transfer from the scene data structure to your own here */
	/* */

	return;
}


/* just a place holder, feel free to edit */
void render(void) {
	image = new float[sizeof(float) * IMAGE_HEIGHT * IMAGE_WIDTH * 3];

	jacksRenderScene(scene);

	CImage imageObj;
	imageObj.Create(IMAGE_WIDTH, IMAGE_HEIGHT, 24);
	BYTE* imagePtr = (BYTE*)imageObj.GetBits();
	int pitch = imageObj.GetPitch();

	for (int j = 0; j < IMAGE_HEIGHT; j++) {
		for (int i = 0; i < IMAGE_WIDTH; i++) {
			glm::vec3 pixel = getPixel(i, IMAGE_HEIGHT - j - 1);
			*(imagePtr + pitch * j + i * 3 + 0) = (BYTE)(min(pixel.b * 255, 255.0f));
			*(imagePtr + pitch * j + i * 3 + 1) = (BYTE)(min(pixel.g * 255, 255.0f));
			*(imagePtr + pitch * j + i * 3 + 2) = (BYTE)(min(pixel.r * 255, 255.0f));
		}
	}

	imageObj.Save(fileName);
	wcout << "Image saved to " << fileName << endl;

	imageObj.Destroy();
	delete[] image;

	return;
}

inline void cross(const glm::vec3& v1, float* v2, float* res) {
	res[0] = v1[1] * v2[2] - v1[2] * v2[1];
	res[1] = v1[2] * v2[0] - v1[0] * v2[2];
	res[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

int main(int argc, char *argv[]) {
	/*
	BoundingBox box;
	PolygonIO poly;
	poly.vert = new VertexIO[3];
	poly.vert[0].pos[0] = 1;
	poly.vert[0].pos[1] = 0;
	poly.vert[0].pos[2] = 0;

	poly.vert[1].pos[0] = 0;
	poly.vert[1].pos[1] = 1;
	poly.vert[1].pos[2] = 0;

	poly.vert[2].pos[0] = 0;
	poly.vert[2].pos[1] = 0;
	poly.vert[2].pos[2] = 1;

	box.apply(BoundingBox(&poly));

	cout << glm::to_string(box.vMin) << endl;
	cout << glm::to_string(box.vMax) << endl;

	glm::vec3 res;
	box.intersect(
		glm::vec3(0.5f, 0.5f, 0.5f),
		-1 * glm::vec3(-1, 0.5f, 0.5f),
		res);
	cout << "Res: " << glm::to_string(res) << endl;

	if (true) return 0;*/

	Timer total_timer;
	total_timer.startTimer();

	if (argc < 2) {
		cout << "Usage:\tBasicRayTracer.exe <input file> [<output file>]" << endl;
		exit(1);
	}

	if (argc >= 3) {
		fileName = CA2W(argv[2]);
	}
	else {
		fileName = _T("output.png");
	}
	
	loadScene(argv[1]);

	render();

	if (scene != NULL) {
		deleteScene(scene);
	}

	jacksCleanupBounds();
	
	total_timer.stopTimer();
	fprintf(stderr, "Total time: %.5lf secs\n\n", total_timer.getTime());
	
	return 0;
}
