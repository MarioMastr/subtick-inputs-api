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

		g_physicsState.levelStartTimestamp = geode::utils::getInputTimestamp();
		g_physicsState.tickCount = 0;
		g_inputChecksCount = 0;
		g_physicsState.firstFrame = true;
		g_inputQueue.clear();

		g_physicsState.player1.lastEventTimestamp = g_physicsState.levelStartTimestamp;
		g_physicsState.player2.lastEventTimestamp = g_physicsState.levelStartTimestamp;

		return true;
	}

	void resetLevel() {
		PlayLayer::resetLevel();

		g_physicsState.firstFrame = true;
		g_physicsState.tickCount = 0;
		g_inputChecksCount = 0;
		g_physicsState.levelStartTimestamp = geode::utils::getInputTimestamp();
		g_inputQueue.clear();

		g_physicsState.player1.lastEventTimestamp = g_physicsState.levelStartTimestamp;
		g_physicsState.player2.lastEventTimestamp = g_physicsState.levelStartTimestamp;
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

		if (g_physicsState.firstFrame) {
			g_physicsState.firstFrame = false;
			GJBaseGameLayer::update(dt);
			return;
		}

		if (playLayer->m_playerDied) {
			g_physicsState.firstFrame = true;
			GJBaseGameLayer::update(dt);
			return;
		}

		g_inputQueue.insert(g_inputQueue.end(),
			playLayer->m_queuedButtons.begin(),
			playLayer->m_queuedButtons.end());
		playLayer->m_queuedButtons.clear();

		std::sort(g_inputQueue.begin(), g_inputQueue.end(),
			[](PlayerButtonCommand const& a, PlayerButtonCommand const& b) {
				return a.m_timestamp < b.m_timestamp;
			});

		GJBaseGameLayer::update(dt);

		double frameEnd = geode::utils::getInputTimestamp();
		PlayerObject* p1 = playLayer->m_player1;
		PlayerObject* p2 = playLayer->m_gameState.m_isDualMode
			? playLayer->m_player2
			: nullptr;

		if (!p1->m_isDashing) {
			advancePlayerToTimestamp(
				p1, frameEnd, g_physicsState.player1.lastEventTimestamp);
		}
		if (p2 && !p2->m_isDashing) {
			advancePlayerToTimestamp(
				p2, frameEnd, g_physicsState.player2.lastEventTimestamp);
		}

		g_inputQueue.clear();
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
