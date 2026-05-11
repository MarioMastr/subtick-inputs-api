#include <ContinuousPhysics.hpp>

namespace continuousphysics {

	ContinuousPhysicsState& ContinuousPhysicsState::get() {
		static ContinuousPhysicsState instance;
		return instance;
	}

	Config& Config::get() {
		static Config instance;
		return instance;
	}

} // namespace continuousphysics
