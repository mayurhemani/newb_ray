#include "newbray.h"

float clamp(float val, float min, float max) {
	if (val <= min) return min;
	if (val >= max) return max;
	return val;
}

namespace bray {

	namespace color {

		donkey::rgb_t phong(donkey::vector_t const& normalVec, 
							donkey::vector_t const& lightVec,
							donkey::vector_t const& cameraVec,
							donkey::rgb_t const& diffuse,
							donkey::rgb_t const& specular,
							float shininess) {

			float diffuseTerm = clamp(glm::dot(normalVec, lightVec), 0.f, 1.f);
			float specularBase = glm::dot( 
					glm::normalize(2.0f * glm::dot(lightVec, normalVec) * normalVec - lightVec),
					cameraVec );
			float specularTerm = clamp((float)std::pow((double)specularBase, (double)shininess), 0.f, 1.f);
			return diffuseTerm * diffuse + specularTerm * specular;
		}

		donkey::rgb_t mixLightColor(
					donkey::rgb_t const& lightColor, 
					float lightIntensity, 
					donkey::rgb_t const& materialColor) {
			donkey::rgb_t ltColor = lightIntensity * lightColor;
			donkey::rgb_t targetClr = donkey::rgb_t(
											ltColor.x * materialColor.x, 
											ltColor.y * materialColor.y, 
											ltColor.z * materialColor.z);
			return 0.9f * targetClr + 0.1f * materialColor;
		}
	}

	intersector_t::result_type intersector_t::findClosest(donkey::geom::ray_t const& ray) const {
		intersector_t::result_type result;
		for (auto object: sceneRef.objects) {
			donkey::points_v points;
			if (donkey::algo::raycast::on_object(object, ray, points)) {
				const donkey::point_t& pos = ray.point;
				for (auto point: points) {
					float distsq = (pos.x - point.x) * (pos.x - point.x) 
									+ (pos.y - point.y) * (pos.y - point.y)
									+ (pos.z - point.z) * (pos.z - point.z);

					if (distsq < result.distance) {
						result.distance = distsq;
						result.object = object;
						result.point = point;
						result.noHit = false;
					}
				}
			}
		}
		result.distance = std::sqrt(result.distance);
		return result;
	}


	// main ray-tracing routine
	donkey::rgb_t newbray_t::getColorForRay(donkey::geom::ray_t const& ray, donkey::scene_t const& scene) const {
		intersector_t raycaster(scene);
		intersector_t::result_type result = raycaster.findClosest(ray);
		if (result.noHit || !result.object)
			return donkey::rgb_t(0.0f, 0.0f, 0.0f);

		donkey::primitive_ptr object = 	std::dynamic_pointer_cast<donkey::primitive::primitive_t>(result.object);
		if (object) {
			donkey::vector_t normal = object->getNormalAt(result.point);
			donkey::vector_t cameraVec = glm::normalize(result.point); // result.point - [0, 0, 0]

			std::vector<donkey::rgb_t> lightColors;

			for (auto lightObj : scene.lights) {
				auto light = donkey::promote<donkey::object::point_light_t<float> >(lightObj);
				if (!light) continue;

				donkey::vector_t lightPos = light->position;
				donkey::vector_t lightVec = glm::normalize(result.point - lightPos);
				donkey::rgb_t lightColor = light->color.diffuse;


				donkey::rgb_t phColor = color::phong(
										normal, 
										lightVec, 
										cameraVec, 
										color::mixLightColor(lightColor, light->intensity, object->material.color.diffuse),
			 							object->material.color.specular, 
			 							object->material.color.shininess);

				donkey::rgb_t clr = object->material.color.ambient + phColor;

				lightColors.push_back(clr);
			}

			donkey::rgb_t color(0.f, 0.f, 0.f);
			
			if (!lightColors.empty()) {
				float factor = 1.f / lightColors.size();
				std::for_each(lightColors.begin(), lightColors.end(), [&factor, &color] (donkey::rgb_t const& c) {
					color += factor * c;
				});
			} 

			return color;
		} 
		return donkey::rgb_t(0.0f, 0.0f, 0.0f);
	}


	bool newbray_t::trace(donkey::scene_t const& scene, image::image_t& toImage) {
		cv::Mat& img = toImage.get();
		unsigned char* data = img.data;

		size_t c = 0;

		for (unsigned long i = 0; i < toImage.height; ++i) {
			for (unsigned long j = 0; j < toImage.width; ++j) {
				unsigned long pos = (i * toImage.width + j) * 3;
				donkey::point_t pixelPosition = camera.positionForPixel(j, i);
				donkey::geom::ray_t ray(camera.e, glm::normalize(pixelPosition));

				donkey::rgb_t clr = getColorForRay(ray, scene);

				float clrx = clamp(clr.x, 0.f, 1.f);
				float clry = clamp(clr.y, 0.f, 1.f);
				float clrz = clamp(clr.z, 0.f, 1.f);

				if (clry > clrx || clrz > clrx) {
					printf("%f %f %f\n", clrx, clry, clrz);
				}

				data[pos] = static_cast<unsigned char>(clrz * 255);
				data[pos+1] = static_cast<unsigned char>(clry * 255);
				data[pos+2] = static_cast<unsigned char>(clrx * 255);
			}
		}

		return true;
	}

}