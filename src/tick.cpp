#include <ContinuousPhysics.hpp>

using namespace continuousphysics::states;
using namespace continuousphysics::config;

namespace continuousphysics::tick {

	bool useVanillaTick(PlayerObject* player) {
		PlayLayer* playLayer = PlayLayer::get();
		return !playLayer || !g_modActive || player->m_isDashing;
	}

	void preTick(PlayerObject* player) {
		input::processInputs(player);

		/* 
			pre-compensate gravity: add what vanilla will subtract,
		 	so vanilla's gravity application nets to zero.
		 	the formula handles gravity interpolation instead.
		*/
		if (!player->m_isDart && !player->m_isDashing) {
			// gravity is part of robot hold mechanic
			// let vanilla handle it without pre-compensation
			if (!(player->m_isRobot && player->m_jumpBuffered)) {
				float gravPerTick =
					physics::getGravityAcceleration(player, g_tps) / g_tps;
				int dir = player->flipMod();
				player->m_yVelocity += static_cast<double>(dir * gravPerTick);
			}
		}
	}

	void postTick(PlayerObject* player) {
		auto* playerState = tryGetPlayerState(player);
		if (!playerState) return;

		double tickTimestamp = g_physicsState.m_levelStartTimestamp +
			g_physicsState.m_tickCount * (1.0 / g_tps);

		physics::advancePlayerToTimestamp(
			player, tickTimestamp, playerState->m_lastEventTimestamp);

		if (player->isPlayer1()) {
			g_physicsState.m_tickCount++;
		}
	}
} // namespace continuousphysics::tick