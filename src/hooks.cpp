#include <ContinuousPhysics.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "api/private.hpp"

using namespace continuousphysics;
using namespace continuousphysics::physics;

class $modify(CCEGLView) {
	void pollEvents() {
		PlayLayer* playLayer = PlayLayer::get();
		CCNode* parent;

		// clang-format off
		if (!GetFocus() || !playLayer
			|| !(parent = playLayer->getParent())
			|| parent->getChildByType<PauseLayer>(0)
			|| playLayer->getChildByType<EndLevelLayer>(0)
			|| playLayer->m_playerDied)
		{
			ContinuousPhysicsState::get().m_firstFrame = true;
		}
		// clang-format on

		CCEGLView::pollEvents();
	}
};

class $modify(PlayLayer) {
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		bool result = PlayLayer::init(level, useReplay, dontCreateObjects);
		if (!result) return false;

		this->m_clickBetweenSteps = false;
		this->m_clickOnSteps = false;

		auto& state = ContinuousPhysicsState::get();
		state.m_firstFrame = true;
		state.m_player1.m_lastEventTimestamp = this->m_timestamp;
		state.m_player2.m_lastEventTimestamp = this->m_timestamp;

		return true;
	}

	void resetLevel() {
		PlayLayer::resetLevel();

		auto& state = ContinuousPhysicsState::get();
		state.m_firstFrame = true;
		state.m_player1.m_lastEventTimestamp = this->m_timestamp;
		state.m_player2.m_lastEventTimestamp = this->m_timestamp;
	}
};

class $modify(GJBaseGameLayer) {
	int checkCollisions(PlayerObject* object, float dt, bool ignoreDamage) {
		int result = GJBaseGameLayer::checkCollisions(object, dt, ignoreDamage);

		if (Config::get().isModActive()) {
			PlayLayer* playLayer = PlayLayer::get();
			if (playLayer) {
				onPostCollision(object);
			}
		}

		return result;
	}

	void update(float dt) {
		PlayLayer* playLayer = PlayLayer::get();
		auto& physicsState = ContinuousPhysicsState::get();

		if (!Config::get().isModActive() || !playLayer ||
			!playLayer->m_player1 || this->m_isPlatformer ||
			this->m_useReplay) {
			GJBaseGameLayer::update(dt);
			return;
		}

		if (physicsState.m_firstFrame) {
			physicsState.m_firstFrame = false;
			GJBaseGameLayer::update(dt);
			physicsState.m_player1.m_lastEventTimestamp = this->m_timestamp;
			physicsState.m_player2.m_lastEventTimestamp = this->m_timestamp;
			return;
		}

		if (playLayer->m_playerDied) {
			physicsState.m_firstFrame = true;
			GJBaseGameLayer::update(dt);
			return;
		}

		GJBaseGameLayer::update(dt);

		double frameEnd = this->m_timestamp;
		PlayerObject* p1 = playLayer->m_player1;
		PlayerObject* p2 = playLayer->m_gameState.m_isDualMode
			? playLayer->m_player2
			: nullptr;

		advancePlayerToTimestamp(
			p1, frameEnd, physicsState.m_player1.m_lastEventTimestamp);
		if (p2) {
			advancePlayerToTimestamp(
				p2, frameEnd, physicsState.m_player2.m_lastEventTimestamp);
		}
	}
};

class $modify(PlayerObject) {
	void setYVelocity(double velocity, int type) {
		if (Config::get().isVelocityUnroundingEnabled()) {
			this->m_yVelocity = velocity;
			return;
		}
		PlayerObject::setYVelocity(velocity, type);
	}
};