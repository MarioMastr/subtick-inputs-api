#include <ContinuousPhysics.hpp>

using namespace continuousphysics::states;
using namespace continuousphysics::physics;
using namespace continuousphysics::config;

namespace continuousphysics::tick {

	bool useVanillaTick(PlayerObject* player) {
		PlayLayer* playLayer = PlayLayer::get();
		return !playLayer || !g_modActive || player->m_isDashing;
	}

	void postTick(PlayerObject* player) {
		auto* playerState = tryGetPlayerState(player);
		if (!playerState) {
			return;
		}

		double tickTimestamp = g_physicsState.levelStartTimestamp +
			g_physicsState.tickCount * (1.0 / g_tps);

		advancePlayerToTimestamp(
			player, tickTimestamp, playerState->lastEventTimestamp);

		if (player->isPlayer1()) {
			g_physicsState.tickCount++;
		}
	}
} // namespace continuousphysics::tick