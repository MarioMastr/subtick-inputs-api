#include "SIPlayerObject.hpp"

namespace subtickinputs::fields {

	double& getYDispField(PlayerObject* player) {
		return static_cast<SIPlayerObject*>(player)->m_fields->m_yDispAdjustment;
	}

	std::vector<PendingWaveInput>& getPendingWaveField(PlayerObject* player) {
		return static_cast<SIPlayerObject*>(player)->m_fields->m_pendingWaveInputs;
	}

} // namespace subtickinputs::fields