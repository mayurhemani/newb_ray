#include "newbray.h"

namespace bray {

	namespace color {

		donkey::rgb_t phong(donkey::vector_t const& normalVec, 
							donkey::vector_t const& lightVec,
							donkey::vector_t const& cameraVec,
							donkey::rgb_t const& diffuse,
							donkey::rgb_t const& specular,
							float shininess) {

			float diffuseTerm = glm::dot(normalVec, lightVec);
			float specularBase = glm::dot( 
					(2.0f * glm::dot(lightVec, normalVec) * normalVec - lightVec),
					cameraVec );
			float specularTerm = (float)std::pow((double)specularBase, (double)shininess);
			return diffuseTerm * diffuse + specularTerm * specular;
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

	

}