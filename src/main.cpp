#include "donkey.h"
#include "newbray.h"
#include "grass.h"
#include <memory>
/*
bray::newbray_params_t params = {
		400,
		300,
		5.0,
		45.0,
		4.0f/3,
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f),
		glm::vec3(0.0, 0.0, 10.f),
		4
	};

	bray::newbray_t tracer(params);

	donkey::primitive_ptr obj = std::make_shared<donkey::primitive::sphere_t>(4.0f, donkey::point_t(0.f, 0.f, 10.f));
	obj->material.color.diffuse = donkey::rgb_t(1.f, 0.f, 0.f);
	obj->material.color.specular = donkey::rgb_t(0.5f, 0.5f, 0.5f);
	obj->material.color.ambient = donkey::rgb_t(0.1f, 0.1f, 0.1f);
	
	donkey::scene_t scene;
	scene.add(obj);

	donkey::primitive_ptr obj2 = std::make_shared<donkey::primitive::sphere_t>(2.0f, donkey::point_t(0.f, 2.f, 4.f));
	obj2->material.color.diffuse = donkey::rgb_t(0.f, 1.f, 1.f);
	obj2->material.color.specular = donkey::rgb_t(0.5f, 0.5f, 0.5f);
	obj2->material.color.ambient = donkey::rgb_t(0.1f, 0.1f, 0.1f);
	
	scene.add(obj2);
*/

int main(int argc, char* argv[]) {

	std::string inputFile;
	std::string outputFile;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-i" && (i+1) < argc) {
			inputFile = argv[i+1];
		} else if (arg == "-o" && (i+1) < argc) {
			outputFile = argv[i+1] + std::string(".jpg");
		}
	}

	if (inputFile.empty()) {
		printf("Usage: %s -i inputFile [-o outputFile]\n", argv[0]);
		return -1;
	}

	grass::scene_file_t data(inputFile);


	bray::image::image_t image(data.params.xRes, data.params.yRes);

	bray::newbray_t tracer(data.params);
	tracer.trace(data.scene, image);

	if (outputFile.empty()) {
		cv::imshow("Result", image.get());
		cv::waitKey(0);
	} else {
		cv::imwrite(outputFile.c_str(), image.get());
	}

	return 0;
}