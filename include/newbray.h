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
			unsigned long width;
			unsigned long height;
			cv::Mat 	im;
			image_t(unsigned long w, unsigned long h):
			width(w), height(h),
			im(height, width, CV_8UC3) {}

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

	struct newbray_params_t {
		short xRes;
		short yRes;
		float planeDistance;
		float fieldOfViewY;
		float aspectRatio;
		donkey::point_t cameraPosition;
		donkey::point_t cameraUp;
		donkey::point_t cameraTarget;
		short maxDepth;
	};


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

	struct camera_t {
		donkey::vector_t n;
		donkey::vector_t u;
		donkey::vector_t v;
		donkey::vector_t e;
		donkey::point_t  bottomLeft;
		donkey::point_t  topLeft;
		float pixelSizeX;
		float pixelSizeY;
		float imWidth;
		float imHeight;


		camera_t(newbray_params_t params): e(params.cameraPosition) {
			n = params.cameraTarget - e;
			n = glm::normalize(n);
			u = glm::cross(params.cameraUp, n);
			u = glm::normalize(u);
			v = glm::cross(n, u);
			imHeight = 2 * params.planeDistance * std::tan(params.fieldOfViewY/2.0);
			imWidth = imHeight * params.aspectRatio;
			donkey::vector_t cpos = e + params.planeDistance * n;
			bottomLeft = cpos - (imWidth/2)*u - (imHeight/2)*v;
			topLeft = cpos - (imWidth/2)*u - (imHeight/2) * v;
			pixelSizeX = imWidth / params.xRes;
			pixelSizeY = imHeight / params.yRes;
		}

		inline donkey::point_t positionForPixel(float p, float q) const {
			return bottomLeft  + p * pixelSizeX * u + q * pixelSizeY * v;
		}

		inline donkey::point_t transformPoint(donkey::point_t const& p) const {
			return e - p;
		}
	};

	
	struct newbray_t {
	private:
		newbray_params_t 	params;
		camera_t 			camera;

	public:
		explicit newbray_t(newbray_params_t const& rayTraceParams):
			params(rayTraceParams),
			camera(rayTraceParams) {}

		bool trace(donkey::scene_t const& scene, image::image_t& toImage);

		inline camera_t const& getCamera() const { return camera; };

		donkey::rgb_t getColorForRay(donkey::geom::ray_t const& ray,
									 donkey::scene_t const& scene) const;

	private:
		void transformObjects(donkey::scene_t& scene);

		donkey::geom::ray_t getRayForPixel(unsigned short x, unsigned short y) const;
		donkey::geom::ray_t getReflectedRay(donkey::geom::ray_t const& ray,
											donkey::point_t const& point,
											donkey::vector_t const&  normal) const;
		donkey::geom::ray_t getShadowRay(donkey::point_t const& point, donkey::point_t const& lightSrcPos) const;
	};

	
}



#endif