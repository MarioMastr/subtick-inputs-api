#include <ContinuousPhysics.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace continuousphysics::states;
using namespace continuousphysics::config;
using namespace continuousphysics::input;
using namespace continuousphysics::physics;

class $modify(PlayLayer) {
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		bool result = PlayLayer::init(level, useReplay, dontCreateObjects);
		if (!result) return false;

		this->m_clickBetweenSteps = false;
		this->m_clickOnSteps = false;

		g_physicsState.m_levelStartTimestamp =
			geode::utils::getInputTimestamp();
		g_physicsState.m_tickCount = 0;
		g_physicsState.m_inputChecksCount = 0;
		g_physicsState.m_firstFrame = true;

		g_physicsState.m_player1.m_lastEventTimestamp =
			g_physicsState.m_levelStartTimestamp;
		g_physicsState.m_player2.m_lastEventTimestamp =
			g_physicsState.m_levelStartTimestamp;

		return true;
	}

	void resetLevel() {
		PlayLayer::resetLevel();

		g_physicsState.m_firstFrame = true;
		g_physicsState.m_tickCount = 0;
		g_physicsState.m_inputChecksCount = 0;
		g_physicsState.m_levelStartTimestamp =
			geode::utils::getInputTimestamp();

		g_physicsState.m_player1.m_lastEventTimestamp =
			g_physicsState.m_levelStartTimestamp;
		g_physicsState.m_player2.m_lastEventTimestamp =
			g_physicsState.m_levelStartTimestamp;
	}

	void onQuit() {
		g_modActive = false;
		PlayLayer::onQuit();
	}
};

class $modify(GJBaseGameLayer) {
	int checkCollisions(PlayerObject* object, float dt, bool ignoreDamage) {
		int result = GJBaseGameLayer::checkCollisions(object, dt, ignoreDamage);

		if (g_modActive) {
			PlayLayer* playLayer = PlayLayer::get();
			if (playLayer) {
				onPostCollision(object);
			}
		}

		return result;
	}

	void update(float dt) {
		PlayLayer* playLayer = PlayLayer::get();

		if (!g_modActive || !playLayer || !playLayer->m_player1 ||
			this->m_isPlatformer || this->m_useReplay) {
			GJBaseGameLayer::update(dt);
			return;
		}

		if (g_physicsState.m_firstFrame) {
			g_physicsState.m_firstFrame = false;
			GJBaseGameLayer::update(dt);
			return;
		}

		if (playLayer->m_playerDied) {
			g_physicsState.m_firstFrame = true;
			GJBaseGameLayer::update(dt);
			return;
		}

		GJBaseGameLayer::update(dt);

		double frameEnd = geode::utils::getInputTimestamp();
		PlayerObject* p1 = playLayer->m_player1;
		PlayerObject* p2 = playLayer->m_gameState.m_isDualMode
			? playLayer->m_player2
			: nullptr;

		advancePlayerToTimestamp(
			p1, frameEnd, g_physicsState.m_player1.m_lastEventTimestamp);
		if (p2) {
			advancePlayerToTimestamp(
				p2, frameEnd, g_physicsState.m_player2.m_lastEventTimestamp);
		}
	}

	void processCommands(float dt, bool isHalfTick, bool isLastTick) {
		if (g_modActive) return;
		GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
	}
};

class $modify(PlayerObject) {
	void setYVelocity(double velocity, int type) {
		if (g_velocityUnroundingEnabled) {
			this->m_yVelocity = velocity;
			return;
		}
		PlayerObject::setYVelocity(velocity, type);
	}
};