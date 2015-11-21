#include "layer.h"

#include <libjson.h>

namespace tiled {

Layer::Layer(const Map& map, const JSONNode& json) {
	throw "NYI";
}

int Layer::tile_at(int col, int row) const {
	return data_[row * width_ + col];
}

} // namespace tiled