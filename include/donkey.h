#ifndef DONKEY_H
#define DONKEY_H

#include "glm/glm.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <functional>

namespace donkey {

	typedef glm::vec3 	point_t;
	typedef glm::vec3 	vector_t;
	typedef glm::vec3 	rgb_t;
	typedef glm::ivec3 	ipoint_t;

	namespace utils {

		template <typename PrecisionType> 
		bool equal(PrecisionType p1, PrecisionType p2) {
			PrecisionType p = (p1 - p2);
			if (p < 0) p = -p; 
			return p < (PrecisionType)0.00001;
		}
	}

	namespace attrib {
		struct vtx_attrib_t {
			std::vector<point_t> normals;
			std::vector<point_t> colors;
		};

		struct face_attrib_t {
			unsigned short materialIdx;
		};
	}

	namespace color {
		struct color_desc_t {
			rgb_t	diffuse;
			rgb_t	specular;
			rgb_t	ambient;
		};

		struct texture_t {
			std::string name;
			std::vector<unsigned char> data;
		};

		struct material_t {
			color_desc_t			color;
			std::vector<texture_t>	textures;
		};

	}

	namespace geom {

		template <typename VtxAttributes>
		struct vertex_t {
			point_t			position;
			VtxAttributes 	attrib;
		};

		template <typename FaceAttributes, int Dims> 
		struct face_t {
			uint32_t index[Dims];
			FaceAttributes attrib;
		};

		template <typename VtxAttributes, typename FaceAttributes, int FaceDims>
		struct geometry_t {
			typedef vertex_t<VtxAttributes>				vtx_type;
			typedef face_t<FaceAttributes, FaceDims> 	face_type;

			std::vector<vtx_type>	vertices;
			std::vector<face_type>	faces;
		};

		struct ray_t {
			point_t		point;
			vector_t	direction;
			ray_t(point_t const& from, point_t const& towards):point(from), direction(towards - point) {}
		};

		template <int Dims, typename Precision>
		struct bbox_t {
			std::vector<Precision> mins;
			std::vector<Precision> maxs;
			bool dirty;

			bbox_t():mins(Dims), maxs(Dims), dirty(true) {}
			Precision min(int dim) const { return mins[dim]; }
			Precision max(int dim) const  { return maxs[dim]; }
			Precision avg(int dim) const { return (mins[dim] + maxs[dim]) / static_cast<Precision>(2); }

			void update(std::vector<Precision> const& p) { 
				if (dirty) { 
					mins = p; 
					maxs = p; 
				} else {
					for (int i = 0; i < Dims; ++i) {
						mins[i] = std::min(mins[i], p[i]);
						maxs[i] = std::max(maxs[i], p[i]);
					}
				}
			}
		};
	}

	namespace object {

		template <typename PrecisionType>
		struct point_light_t {
			point_t 		position;
			color::color_desc_t 	color;
			PrecisionType	intensity;
		};

		template <typename PrecisionType>
		struct  directional_light_t {
			vector_t		direction;
			color::color_desc_t	color;
			PrecisionType	intensity;
		};


		template<typename VertexAttrib, typename FaceAttrib, typename PrecisionType, int FaceDims> 
		struct mesh_t {
			typedef geom::geometry_t<VertexAttrib, FaceAttrib, FaceDims> geometry_type;
			typedef color::material_t color_type;
			geometry_type  	geometry;
			color_type		material;
		};


		typedef mesh_t<attrib::vtx_attrib_t, attrib::face_attrib_t, float, 3> trimesh_t;

	}

	namespace primitive {
		struct plane_t {
			vector_t	normal;
			point_t 	point;
		};

		struct triangle_t: public plane_t {
			point_t 	v0;
			point_t 	v1;
			point_t 	v2;

			triangle_t(point_t p1, point_t p2, point_t p3):
			v0(p1), v1(p2), v2(p3)
			{
				normal = glm::cross(v0 - v1, v0 - v2);
				point = v0;
			}
		};

		struct cube_t {
		public:	
			enum face_id {
				kFaceTop,
				kFaceBottom,
				kFaceLeft,
				kFaceRight,
				kFaceFront,
				kFaceBack,
				kNumFaceIds
			};
			
		private:
			inline glm::vec2 vecBound(face_id type, point_t const& p) const {
				switch (type) {
					case kFaceTop:
					case kFaceBottom:
						return glm::vec2(p.x, p.z);
					case kFaceBack:
					case kFaceFront: 
						return glm::vec2(p.x, p.y);
					case kFaceLeft:
					case kFaceRight:
						return glm::vec2(p.y, p.z);
					default:;
				}
				return glm::vec2(0.0f, 0.0f);
			}

		public:	
			float halfSize;
			std::vector<plane_t> planes;
			geom::bbox_t<2, float> bounds[kNumFaceIds];
			cube_t(float size, point_t origin = point_t(0, 0, 0));
			bool inFace(face_id id, point_t const& p) const;
		};

		struct sphere_t {
			float 	radius;
			point_t center;
			sphere_t(float rad, point_t const& cen): radius(rad), center(cen) {}
		};
	}

	namespace algo {
		point_t barycentric(primitive::triangle_t const& tri, point_t const& point);

		namespace raycast {
			bool on_plane(primitive::plane_t const& plane, geom::ray_t const& ray, point_t& point);
			bool on_triangle(primitive::triangle_t const& tri, geom::ray_t const& ray, point_t& point);
			bool on_cube(primitive::cube_t const& cube, geom::ray_t const& ray, 
						 point_t& point, primitive::cube_t::face_id& faceid);
		}

	}

}

#endif