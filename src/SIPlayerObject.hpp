#pragma once

#include <Geode/modify/PlayerObject.hpp>

namespace subtickinputs::fields {
	struct PendingWaveInput {
		double m_ratio;
		bool m_isPush;
		int m_button;
	};

	double& getYDispField(PlayerObject* player);
	std::vector<PendingWaveInput>& getPendingWaveField(PlayerObject* player);
} // namespace subtickinputs::fields

class $modify(SIPlayerObject, PlayerObject) {
	struct Fields {
		double m_yDispAdjustment = 0.0;
		std::vector<subtickinputs::fields::PendingWaveInput> m_pendingWaveInputs;
	};

	void update(float dt);
	void setYVelocity(double velocity, int type);
	void processPlayerTick(float dt);
};