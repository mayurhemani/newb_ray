#include "donkey.h"

namespace donkey {
	namespace primitive {

		cube_t::cube_t(float size, point_t origin): halfSize(size/2.0f),
			planes({
				{vector_t(0, 1, 0), origin + vector_t(0, 1, 0) * halfSize},
				{vector_t(0, -1, 0), origin + vector_t(0, -1, 0) * halfSize},
				{vector_t(-1, 0, 0), origin + vector_t(-1, 0, 0) * halfSize},
				{vector_t(1, 0, 0), origin + vector_t(1, 0, 0) * halfSize},
				{vector_t(0, 0, 1), origin + vector_t(0, 0, 1) * halfSize},
				{vector_t(0, 0, -1), origin + vector_t(0, 0, -1) * halfSize}
			}) {

			// compute bounds
			const std::vector< std::vector<int> > diffs = { {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
			for (int i = 0; i < kNumFaceIds; ++i) {
				const glm::vec2 tpt = vecBound(face_id(i), planes[i].point);
				for (auto sc: diffs) {
					glm::vec2 bound = tpt + glm::vec2(sc[0] * halfSize, sc[1] * halfSize);
					bounds[i].update({ bound.x, bound.y });
				}
			}
		}

		bool cube_t::inFace(face_id id, point_t const& pt) const {
			glm::vec2 coords = vecBound(id, pt);
			return (bounds[id].mins[0] <= coords.x && coords.x <= bounds[id].maxs[0])
				 && (bounds[id].mins[1] <= coords.y && coords.y <= bounds[id].maxs[1]);
		}
	}

	namespace algo {

		/** 
		* Compute barycentric coordinates for a given triangle and a given point
		*/
		point_t barycentric(primitive::triangle_t const& tri, point_t const& point) {
			vector_t v0 = tri.v1 - tri.v0,
					 v1 = tri.v2 - tri.v0,
					 v2 = point - tri.v0;
			float d00 = glm::dot(v0, v0);
			float d01 = glm::dot(v0, v1);
			float d11 = glm::dot(v1, v1);
			float d20 = glm::dot(v2, v0);
			float d21 = glm::dot(v2, v1);
			float d = d00 * d11 - d01 * d01;
			
			point_t uvw(0, 0, 0);
			uvw.y = (d11 * d20 - d01 * d21) / d;
			uvw.z = (d00 * d21 - d01 * d20) / d;
			uvw.x = 1.0f - uvw.y - uvw.z;
			return uvw;
		}

		/**
		* Basic ray-casting algorithms
		*/
		namespace raycast {

			bool on_plane(primitive::plane_t const& plane, geom::ray_t const& ray, point_t& point) {

				float d = glm::dot(plane.normal, ray.direction);

				// no intersection if the normal and the ray direction are perpendicular
				if (utils::equal(d, 0.0f))
					return false;

				float n = glm::dot(plane.normal, plane.point - ray.point);
				float t = n / d;

				if (t < 0) return false;

				point = ray.point + t * ray.direction;

				return true;
			}


			bool on_triangle(primitive::triangle_t const& tri, geom::ray_t const& ray, point_t& point) {
				
				bool b = on_plane(tri, ray, point);

				if (b) {
					point_t bcent = donkey::algo::barycentric(tri, point);
					if (bcent.x < 0 || bcent.y < 0 || bcent.z < 0) return false;
				}

				return true;
			}

			bool on_cube(
						primitive::cube_t const& cube, 
						geom::ray_t const& ray, 
						point_t& point, 
						primitive::cube_t::face_id& faceid
			) {
				bool b = false;
				for (int i = 0; i < primitive::cube_t::kNumFaceIds; ++i) {
					primitive::cube_t::face_id fid = primitive::cube_t::face_id(i);
					point_t p;
					if (on_plane(cube.planes[i], ray, p) && cube.inFace(fid, p)) {
						point = p;
						faceid = fid;
						b = true;
						break;
					}
				}
				return b;
			}


		}

	}
}