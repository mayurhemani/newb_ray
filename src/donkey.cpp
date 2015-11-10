#include "donkey.h"

namespace donkey {


	namespace primitive {

		cube_t::cube_t(float size, point_t origin): 
			primitive_t(object::kCube),
			halfSize(size/2.0f),
			planes({
				plane_t {vector_t(0, 1, 0), origin + vector_t(0, 1, 0) * halfSize},
				plane_t {vector_t(0, -1, 0), origin + vector_t(0, -1, 0) * halfSize},
				plane_t {vector_t(-1, 0, 0), origin + vector_t(-1, 0, 0) * halfSize},
				plane_t {vector_t(1, 0, 0), origin + vector_t(1, 0, 0) * halfSize},
				plane_t {vector_t(0, 0, 1), origin + vector_t(0, 0, 1) * halfSize},
				plane_t {vector_t(0, 0, -1), origin + vector_t(0, 0, -1) * halfSize}
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

			bool on_sphere(primitive::sphere_t const& sphere, geom::ray_t const& ray, point_t& p1, point_t& p2) {
				float dd = glm::dot(ray.direction, ray.direction);
				float ec = 2.0f * glm::dot(ray.direction, (ray.point - sphere.center));
				float r2 = sphere.radius  * sphere.radius;

				float dt = ec*ec + 4*dd*r2;

				if (dt < 0) return false;
				if (donkey::utils::equal(dd, 0.f)) return false;

				dt = std::sqrt(dt);
				dd = 1/(2 * dd);
				float t0 = dd * (-ec + dt);
				float t1 = dd * (-ec - dt);

				p1 = ray.point + t0 * ray.direction;
				p2 = ray.point + t1 * ray.direction;

				return true; 
			}

			bool on_object(scene_object_ptr object, geom::ray_t const& ray, points_v& points) {
				if (!(object)) return false;

				bool b = false;
				switch (object->type) {
					case object::kPlane: {
						point_t pt;
						if (true == 
							(b = on_plane(
									*(std::dynamic_pointer_cast<primitive::plane_t>(object)), 
									ray, 
									pt))
							) {
							points.push_back(pt);
						}
						break;
					}

					case object::kSphere: {
						point_t p1, p2;
						if (true == 
							(b = on_sphere(
								*(std::dynamic_pointer_cast<primitive::sphere_t>(object)), 
								ray, 
								p1, 
								p2))
							) {
							points.push_back(p1);
							points.push_back(p2);
						} 
						break;
					}

					default:;
				}
				return b;
			}

		}

	}
}