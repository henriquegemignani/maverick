#include "map.h"

#include <ugdk/filesystem/module.h>
#include <ugdk/filesystem/file.h>
#include <ugdk/system/exceptions.h>
#include <libjson.h>

namespace tiled {

Map::Map(const JSONNode& json_node) 
	//: width_(json_node["width"].)
{
	width_ = json_node["width"].as_int();
	height_ = json_node["height"].as_int();
	tile_width_ = json_node["tilewidth"].as_int();
	tile_height_ = json_node["tileheight"].as_int();
	if (json_node["orientation"].as_string() != "orthogonal")
		throw ugdk::system::BaseException("Map orientation not orthogonal.");
	if (json_node["renderorder"].as_string() != "right-down")
		throw ugdk::system::BaseException("Map renderorder not right-down.");

	{
		JSONNode layers_node = json_node["layers"];
		layers_.reserve(layers_node.size());
		for (const auto& layer_config : layers_node)
			layers_.emplace_back(*this, layer_config);
	}

	{
		JSONNode tilesets_node = json_node["tilesets"];
		tilesets_.reserve(tilesets_node.size());
		/*for (const auto& tilesets_config : tilesets_node)
			tilesets_.emplace_back(tilesets_config);*/
	}

	//json_node["properties"]
}

std::unique_ptr<Map> Map::ReadFromFile(const std::string& filepath) {
	auto json_file = ugdk::filesystem::manager()->OpenFile(filepath);
	if (!json_file)
		throw ugdk::system::BaseException("File not found: %s\n", filepath.c_str());

	auto contents = json_file->GetContents();
	if (!libjson::is_valid(contents))
		throw ugdk::system::BaseException("Invalid json: %s\n", filepath.c_str());

	auto json_root = libjson::parse(contents);
	return std::make_unique<Map>(json_root);
}

} // namespace shared