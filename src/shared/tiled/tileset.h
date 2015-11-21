#ifndef TILED_TILESET_H_
#define TILED_TILESET_H_

class JSONNode;

namespace tiled {

class Tileset {
public:
	Tileset(const JSONNode& json);
};

} // namespace tiled

#endif // TILED_TILESET_H_
