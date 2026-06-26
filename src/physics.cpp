#include <SubtickInputs.hpp>

using namespace subtickinputs;

namespace subtickinputs::physics {
	bool useVanillaPhysics() {
		log::warn("update Superb Input Precision, a deprecated function has been called");
		return useVanilla();
	}
} // namespace subtickinputs::physics
