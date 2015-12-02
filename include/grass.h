#ifndef GRASS_FOOD_H
#define GRASS_FOOD_H
#include "rapidjson/rapidjson.h"
#include "rapidjson/Document.h"
#include "donkey.h"
#include "newbray.h"
#include <fstream>
#include <exception>

namespace grass {

	namespace parse_utils {
		template <typename func>
		void for_each_arr(rapidjson::Value const& arr, func callback) {
			for (int i = 0, e = arr.Size(); i < e; ++i) {
				callback(arr[i]);
			}
		}
	}

	struct model_parser_t {

		donkey::primitive_ptr object;
		
		void parseSphere(rapidjson::Value const& sphere) {
			donkey::point_t center(0.f, 0.f, 0.f);
			float radius = sphere["radius"].IsNumber() ? sphere["radius"].GetDouble() : 0.f;
			if (sphere["center"].IsArray()) {
				const rapidjson::Value& point = sphere["center"];
				center = donkey::point_t(
						point[0].GetDouble(),
						point[1].GetDouble(),
						point[2].GetDouble()
					);
			}
			object = std::make_shared<donkey::primitive::sphere_t>(radius, center);
		}

		void parseCube(rapidjson::Value const& cube) {

		}

		void parsePlane(rapidjson::Value const& plane) {

		}

		void parseTriangle(rapidjson::Value const& triangle) {

		}

		donkey::color::material_t parseMaterial(rapidjson::Value const& val) {
			donkey::color::material_t mat;

			auto toColor = [](rapidjson::Value const& val) {
				return donkey::rgb_t(
						val[0].GetDouble(),
						val[1].GetDouble(),
						val[2].GetDouble()
					);
			};

			if (val["color"].IsObject()) {
				if (val["color"]["diffuse"].IsArray()) 
					mat.color.diffuse = toColor(val["color"]["diffuse"]);
				if (val["color"]["specular"].IsArray())
					mat.color.specular = toColor(val["color"]["specular"]);
				if (val["color"]["ambient"].IsArray())
					mat.color.ambient = toColor(val["color"]["ambient"]);
				
			} else if (val["texture"].IsString()) {

			}

			return mat;
		}

		explicit model_parser_t(rapidjson::Value const& val) {
			const std::string type = val["type"].GetString();
			if (type == "sphere") {
				parseSphere(val);
			} else if (type == "cube") {
				parseCube(val);
			} else if (type == "triangle") {
				parseTriangle(val);
			} else if (type == "plane") {
				parsePlane(val);
			}
			object->material = parseMaterial(val["material"]);
		}

		donkey::primitive_ptr getModel() {
			return object;
		}
	};

	struct tracer_parser_t {
		
		std::shared_ptr<bray::newbray_params_t> params;

		explicit tracer_parser_t(rapidjson::Value const& paramsVal) :
		params(std::make_shared<bray::newbray_params_t>()) {
			if (paramsVal["xRes"].IsNumber()) params->xRes = paramsVal["xRes"].GetDouble();
			if (paramsVal["yRes"].IsNumber()) params->yRes = paramsVal["yRes"].GetDouble();
			if (paramsVal["planeDistance"].IsNumber()) params->planeDistance = paramsVal["planeDistance"].GetDouble();
			if (paramsVal["fieldOfViewY"].IsNumber()) params->fieldOfViewY = paramsVal["fieldOfViewY"].GetDouble();
			if (paramsVal["aspectRatio"].IsNumber()) params->aspectRatio = paramsVal["aspectRatio"].GetDouble();
			if (paramsVal["cameraPosition"].IsArray()) {
				params->cameraPosition = donkey::point_t(
						paramsVal["cameraPosition"][0].GetDouble(),
						paramsVal["cameraPosition"][1].GetDouble(),
						paramsVal["cameraPosition"][2].GetDouble()
					);
			}
			if (paramsVal["cameraUp"].IsArray()) {
				params->cameraUp = donkey::vector_t(
						paramsVal["cameraUp"][0].GetDouble(),
						paramsVal["cameraUp"][1].GetDouble(),
						paramsVal["cameraUp"][2].GetDouble()
					);
			}
			if (paramsVal["cameraTarget"].IsArray()) {
				params->cameraTarget = donkey::point_t(
						paramsVal["cameraTarget"][0].GetDouble(),
						paramsVal["cameraTarget"][1].GetDouble(),
						paramsVal["cameraTarget"][2].GetDouble()
					);
			}
			if (paramsVal["maxDepth"].IsNumber()) {
				params->maxDepth = paramsVal["maxDepth"].GetDouble();
			}
		}

		std::shared_ptr<bray::newbray_params_t> getParams() {
			return params;
		}
	};



	struct scene_parser_t {
		rapidjson::Document doc;
		explicit scene_parser_t(std::string const& json) {
			doc.Parse(json.c_str());
			if (!doc.IsObject()) {
				throw std::exception();
			}
		}

		void getScene(donkey::scene_t& scene, bray::newbray_params_t& params) {

			for (rapidjson::Value::ConstMemberIterator i = doc.MemberBegin(),
				e = doc.MemberEnd(); i != e; ++i) {

				std::string name = i->name.GetString();
				if (name == "models") {
					parse_utils::for_each_arr(i->value, [&scene](rapidjson::Value const& val) {
						model_parser_t parser(val);
						donkey::primitive_ptr obj = parser.getModel();
						scene.add(obj);
					});
				} else if (name == "params") {
					tracer_parser_t parser(i->value);
					params = *(parser.getParams());
				}
			}
		}
	};


	struct scene_file_t {
		
		donkey::scene_t scene;
		bray::newbray_params_t params;

		explicit scene_file_t(std::string const& file) {
			// read file
			std::ifstream fin(file.c_str());
			std::string modelText;
			std::string line;
			while (std::getline(fin, line)) {
				modelText += line;
			}

			// parse json and get the data 
			scene_parser_t parser(modelText);

			// get the scene and rendering params
			parser.getScene(scene, params);
		}
	};
}


#endif