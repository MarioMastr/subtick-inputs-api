#include <ContinuousPhysics.hpp>

using namespace continuousphysics::states;
using namespace continuousphysics::config;

namespace continuousphysics::physics {

	void updateTPS(float tps) {
		if (g_subframesEnabled) {
			g_tps = g_inputHz * 4.0f;
		} else {
			g_tps = tps;
		}
	}

	double quantizeYVelocity(double velocity) {
		velocity = std::clamp(velocity, -1000.0, 1000.0);

		if (g_velocityUnroundingEnabled) {
			return velocity;
		}

		double wholePart = static_cast<double>(static_cast<int>(velocity));
		if (velocity != wholePart) {
			double frac = std::round((velocity - wholePart) * 1000.0);
			return frac / 1000.0 + wholePart;
		}

		return velocity;
	}

	float evalYPosition(PlayerObject* player, double secondsSinceEvent) {
		float yPos = player->getPositionY();
		double yVel = player->m_yVelocity;
		double t = secondsSinceEvent;

		return yPos + static_cast<float>(yVel * t * 60.0);
	}

	float evalXPosition(PlayerObject* player, double secondsSinceEvent) {
		float xPos = player->getPositionX();
		double xSpeed = player->getCurrentXVelocity();
		int dir = player->reverseMod();

		return static_cast<float>(
			xPos + (xSpeed * dir * secondsSinceEvent * 60.0));
	}

	void advancePlayerToTimestamp(
		PlayerObject* player, double timestamp, double& lastEventTimestamp) {
		if (player->m_isDashing) {
			lastEventTimestamp = timestamp;
			return;
		}

		double secondsSinceLastEvent = timestamp - lastEventTimestamp;
		if (secondsSinceLastEvent <= 0.0) return;

		float newX, newY;
		if (!player->m_isSideways) {
			newX = evalXPosition(player, secondsSinceLastEvent);
			newY = evalYPosition(player, secondsSinceLastEvent);
		} else {
			newX = evalYPosition(player, secondsSinceLastEvent);
			newY = evalXPosition(player, secondsSinceLastEvent);
		}

		player->setPosition({newX, newY});
		lastEventTimestamp = timestamp;
	}

	void onPostCollision(PlayerObject* player) {
		auto* playerState = tryGetPlayerState(player);
		if (!playerState) return;
		double& lastEventTimestamp = playerState->lastEventTimestamp;

		lastEventTimestamp = g_physicsState.levelStartTimestamp +
			(g_physicsState.tickCount - 1) * (1.0 / g_tps);

		player->m_yVelocity = quantizeYVelocity(player->m_yVelocity);
	}

} // namespace continuousphysics::physics