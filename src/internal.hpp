#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace subtickinputs::physics {

	double getGravPerTick(PlayerObject* player, float tps);

} // namespace subtickinputs::physics

namespace subtickinputs::patches {

	void toggleVelocityUnroundingPatches(bool enable);

} // namespace subtickinputs::patches

namespace subtickinputs::internal {

	inline bool g_firstFrame = true;

	struct PendingWaveInput {
		double m_ratio;
		bool m_isPush;
		int m_button;
	};

	double& getYDispField(PlayerObject* player);
	std::vector<PendingWaveInput>& getPendingWaveField(PlayerObject* player);

} // namespace subtickinputs::internal