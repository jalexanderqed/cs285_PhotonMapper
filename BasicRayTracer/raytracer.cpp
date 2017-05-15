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

const int IMAGE_WIDTH = 400;
const int IMAGE_HEIGHT = 400;

using namespace std;

float *image;

SceneIO *scene = NULL;

static void loadScene(char *name) {
	/* load the scene into the SceneIO data structure using given parsing code */
	scene = readScene(name);

	/* hint: use the Visual Studio debugger ("watch" feature) to probe the
	   scene data structure and learn more about it for each of the given scenes */


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

	/* save out the image */
	
	LPCTSTR fileName = _T("test.png");
	imageObj.Save(fileName);
	wcout << "Image saved to " << fileName << endl;

	/* cleanup */
	imageObj.Destroy();
	delete[] image;

	return;
}



int main(int argc, char *argv[]) {
	Timer total_timer;
	total_timer.startTimer();

	loadScene("../Scenes/test5.scene");

	/* write your ray tracer here */
	render();

	/* cleanup */
	if (scene != NULL) {
		deleteScene(scene);
	}

	total_timer.stopTimer();
	fprintf(stderr, "Total time: %.5lf secs\n\n", total_timer.getTime());
	
	return 0;
}
