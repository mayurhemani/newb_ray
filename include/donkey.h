#ifndef DONKEY_H
#define DONKEY_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <functional>

namespace donkey {

	typedef glm::vec3 	point_t;
	typedef glm::vec3 	vector_t;
	typedef glm::vec3 	rgb_t;
	typedef glm::ivec3 	ipoint_t;
	typedef std::vector<point_t> points_v;

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
			float 	shininess;
			color_desc_t():shininess(1.0f) {}
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

		enum object_type {
			kMesh,
			kCube,
			kSphere,
			kTriangle,
			kPlane,
			kPointLight,
			kDirectionalLight,
			kCamera,
			kNumObjectTypes
		};

		struct scene_object_t  {
			point_t position;
			object_type type;
			explicit scene_object_t(object_type otype):type(otype) {}
			virtual ~scene_object_t(){}
		};

		template <typename PrecisionType>
		struct point_light_t: public scene_object_t {
			color::color_desc_t 	color;
			PrecisionType			intensity;
			point_t 				position;
			point_light_t():scene_object_t(kPointLight) {}
		};

		template <typename PrecisionType>
		struct  directional_light_t: public scene_object_t {
			vector_t		direction;
			color::color_desc_t	color;
			PrecisionType	intensity;
			directional_light_t():scene_object_t(kDirectionalLight) {}
		};


		template<typename VertexAttrib, typename FaceAttrib, typename PrecisionType, int FaceDims> 
		struct mesh_t: scene_object_t {
			typedef geom::geometry_t<VertexAttrib, FaceAttrib, FaceDims> geometry_type;
			typedef color::material_t color_type;
			geometry_type  	geometry;
			color_type		material;

			mesh_t():scene_object_t(kMesh) {}
		};


		typedef mesh_t<attrib::vtx_attrib_t, attrib::face_attrib_t, float, 3> trimesh_t;

		namespace camera {
			struct camera_t: public scene_object_t {
				glm::mat4 matrix;
				camera_t(): scene_object_t(kCamera) {}
			};

			struct perspective_t: public camera_t {
				glm::mat4 viewMatrix;

				perspective_t(const float fovy, const float aspect, const float near, const float far) {
					matrix = glm::perspective(fovy, aspect, near, far);
				}

				inline void lookAt(vector_t const& eye, vector_t const& up, vector_t const& target) {
					// perspective (T const &fovy, T const &aspect, T const &near, T const &far)
					viewMatrix = glm::lookAt(eye, target, up);
				}

				inline point_t transformPoint(point_t const& point) const {
					glm::vec4 pt = glm::vec4(point.x, point.y, point.z, 1.0);
					glm::vec4 m = matrix * viewMatrix * pt;
					return glm::vec3(m);
				}
			};
		}
	}

	namespace primitive {
		struct primitive_t: public object::scene_object_t {
			color::material_t  material;
			primitive_t(object::object_type type): scene_object_t(type) {}
			virtual vector_t getNormalAt(point_t const& point) const { 
				return glm::vec3(0.f, 0.f, 0.0f); 
			}
		};

		struct plane_t : public primitive_t {
			vector_t	normal;
			point_t 	point;
			plane_t(): primitive_t(object::kPlane) {}
			plane_t(vector_t const& planeNormal, vector_t const& planePoint):
				primitive_t(object::kPlane),
				normal(planeNormal), point(planePoint) {}

			vector_t getNormalAt(point_t const&) const {
				return normal;
			}
		};

		struct triangle_t: public plane_t, public primitive_t {
			point_t 	v0;
			point_t 	v1;
			point_t 	v2;

			triangle_t(point_t p1, point_t p2, point_t p3):
			primitive_t(object::kTriangle),
			v0(p1), v1(p2), v2(p3)
			{
				normal = glm::cross(v0 - v1, v0 - v2);
				point = v0;
			}

			vector_t getNormalAt(point_t const& ) const {
				return normal;
			}
		};

		struct cube_t : public primitive_t {
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

		struct sphere_t: public primitive_t {
			float 	radius;
			point_t center;
			sphere_t(float rad, point_t const& cen): primitive_t(object::kSphere), radius(rad), center(cen) {}
			vector_t getNormalAt(point_t const& point) const {
				return glm::normalize(point - center);
			}
		};
	}

	typedef std::shared_ptr<object::scene_object_t> scene_object_ptr;
	typedef std::vector<scene_object_ptr> scene_object_list;
	typedef std::shared_ptr<primitive::primitive_t> primitive_ptr;

	struct scene_t {
		scene_object_list objects;
		scene_object_list lights;

		
		void add(scene_object_ptr obj) {
			objects.push_back(obj);
		}

		void addLight(scene_object_ptr light) {
			lights.push_back(light);
		}
	};

	namespace algo {
		point_t barycentric(primitive::triangle_t const& tri, point_t const& point);

		namespace raycast {
			bool on_plane(primitive::plane_t const& plane, geom::ray_t const& ray, point_t& point);
			bool on_triangle(primitive::triangle_t const& tri, geom::ray_t const& ray, point_t& point);
			bool on_cube(primitive::cube_t const& cube, geom::ray_t const& ray, 
						 point_t& point, primitive::cube_t::face_id& faceid);
			bool on_sphere(primitive::sphere_t const& sphere, geom::ray_t const& ray,
						point_t& p1, point_t& p2);
			bool on_object(scene_object_ptr object, geom::ray_t const& ray, points_v& points);
		}
	}

	template <typename PtrType>
	inline scene_object_ptr demote(PtrType ptr) {
		return std::dynamic_pointer_cast<donkey::object::scene_object_t>(ptr);
	}

	template <typename ElemType>
	inline std::shared_ptr<ElemType> promote(scene_object_ptr ptr) {
		return std::dynamic_pointer_cast<ElemType>(ptr);
	}

	template <typename PtrType>
	inline PtrType promotePtr(scene_object_ptr ptr) {
		return std::dynamic_pointer_cast<typename PtrType::element_type>(ptr);
	}
}

inline void printVector(glm::vec3 v) {
	printf("[%f %f %f]\n", v.x, v.y, v.z);
}

#endif