#ifndef MAVERICK_BACKEND_FRAMEINPUTSOURCE_H_
#define MAVERICK_BACKEND_FRAMEINPUTSOURCE_H_

#include "backend/frameinput.h"

namespace backend {

class FrameInputSource {
public:
	virtual ~FrameInputSource() = default;

	/// Fetches new data from the source.
	virtual FrameInput NextFrameInput() const = 0;
};

}

#endif // MAVERICK_BACKEND_FRAMEINPUTSOURCE_H_