#ifndef TILED_MAP_H_
#define TILED_MAP_H_

#include "layer.h"
#include "tileset.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

class JSONNode;

namespace tiled {

	class Map {
	public:
		Map(const JSONNode& json);
		static std::unique_ptr<Map> ReadFromFile(const std::string&);

	private:
		int width_;
		int height_;
		int tile_width_;
		int tile_height_;
		std::vector<Layer> layers_;
		std::vector<std::tuple<Tileset, int>> tilesets_;
		std::unordered_map<std::string, std::string> properties_;
	};

} // namespace tiled

#endif // TILED_MAP_H_
