#ifndef NEWBRAY_H
#define NEWBRAY_H
#include "donkey.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <string>
#include <limits>

namespace bray  {

	namespace image {
		struct image_t {
			cv::Mat im;
			explicit image_t(const std::string& path);
			inline cv::Mat& get() { return im; }
		};
	}

	namespace color {

		donkey::rgb_t phong(donkey::vector_t const& normalVec, 
							donkey::vector_t const& lightVec,
							donkey::vector_t const& cameraVec,
							donkey::rgb_t const& diffuse,
							donkey::rgb_t const& specular,
							float shininess);

	}

	struct intersector_t {

		struct result_type {
			float 						distance;
			donkey::point_t 			point;
			donkey::scene_object_ptr 	object;
			bool						noHit;
			result_type():distance(std::numeric_limits<float>::max()), noHit(true){}
		};

		donkey::scene_t const& sceneRef;
		explicit intersector_t(donkey::scene_t const& scene):sceneRef(scene) {}
		result_type findClosest(donkey::geom::ray_t const& ray) const;
	};

	struct ray_generator_t {
		donkey::scene_t const& sceneRef;
		explicit ray_generator_t(donkey::scene_t const& scene): sceneRef(scene) {}
		donkey::geom::ray_t getRayForPixel(unsigned short x, unsigned short y) const;
		donkey::geom::ray_t getReflectedRay(donkey::geom::ray_t const& ray,
											donkey::point_t const& point,
											donkey::vector_t const&  normal) const;
		donkey::geom::ray_t getShadowRay(donkey::point_t const& point, donkey::point_t const& lightSrcPos) const;
	};

	struct tracer_t {
		short maxDepth;
		explicit tracer_t(short recDepth=2):maxDepth(recDepth){}
		bool trace(donkey::scene_t const& scene, image::image_t& toImage);
	};

	struct newbray_params_t {
		short xRes;
		short yRes;
		float planeDistance;
		float fieldOfViewY;
		float aspectRatio;
		donkey::vector_t cameraPosition;
		donkey::vector_t cameraUp;
		
	};
}



#endif