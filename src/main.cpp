#include "donkey.h"
#include "newbray.h"
#include <memory>

int main() {

/*
		short xRes;
		short yRes;
		float planeDistance;
		float fieldOfViewY;
		float aspectRatio;
		donkey::point_t cameraPosition;
		donkey::point_t cameraUp;
		donkey::point_t cameraTarget;
		short maxDepth;
*/
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
	obj->material.color.diffuse = donkey::rgb_t(0.f, 0.f, 1.f);
	
	donkey::scene_t scene;
	scene.add(obj);

	
	bray::image::image_t image(400, 300);

	tracer.trace(scene, image);

	cv::imshow("Result", image.get());

	cv::waitKey(0);

	return 0;
}