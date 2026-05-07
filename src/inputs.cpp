#include <ContinuousPhysics.hpp>

using namespace continuousphysics::states;
using namespace continuousphysics::config;
using namespace continuousphysics::physics;

namespace continuousphysics::input {

	void processInputs(PlayerObject* player) {
		PlayLayer* playLayer = PlayLayer::get();
		auto* playerState = tryGetPlayerState(player);
		if (!playLayer) return;
		if (!playerState) return;

		double& lastEventTimestamp = playerState->m_lastEventTimestamp;
		double tickTimestamp = g_physicsState.m_levelStartTimestamp +
			g_physicsState.m_tickCount * (1.0 / g_tps);

		double inputCheckInterval = 1.0 / g_inputHz;
		double nextInputCheck = g_physicsState.m_levelStartTimestamp +
			g_physicsState.m_inputChecksCount * inputCheckInterval;

		while (nextInputCheck < lastEventTimestamp) {
			g_physicsState.m_inputChecksCount++;
			nextInputCheck += inputCheckInterval;
		}

		auto& inputQueue = playLayer->m_queuedButtons;
		bool isPlayer1 = player->isPlayer1();
		int inputIdx = 0;

		while (nextInputCheck <= tickTimestamp) {
			while (inputIdx < static_cast<int>(inputQueue.size())) {
				auto& input = inputQueue[inputIdx];

				bool inputIsP1 = !input.m_isPlayer2;
				if (inputIsP1 != isPlayer1) {
					inputIdx++;
					continue;
				}

				if (input.m_timestamp >= nextInputCheck) break;

				advancePlayerToTimestamp(
					player, input.m_timestamp, lastEventTimestamp);

				playLayer->handleButton(input.m_isPush,
					static_cast<int>(input.m_button), isPlayer1);

				if (!input.m_isPush && player->m_isDashing) {
					advancePlayerToTimestamp(
						player, nextInputCheck, lastEventTimestamp);
					player->stopDashing();
					lastEventTimestamp = nextInputCheck;
					inputIdx++;
					continue;
				}

				double originalTimestamp = input.m_timestamp;
				input.m_timestamp = nextInputCheck;
				handleInput(input, player, lastEventTimestamp);
				input.m_timestamp = originalTimestamp;

				inputIdx++;
			}

			g_physicsState.m_inputChecksCount++;
			nextInputCheck += inputCheckInterval;
		}
	}

	void handleInput(PlayerButtonCommand& input, PlayerObject* player,
		double& lastEventTimestamp) {
		bool isMini = std::abs(player->m_vehicleSize - 1.0f) > 0.01f;
		float generalSizeScale = isMini ? 0.8f : 1.0f;
		int dir = player->flipMod();

		if (!input.m_isPush) {
			if (player->m_isShip || player->m_isBird) {
				// Ship/UFO: holding state already updated by handleButton
			} else if (player->m_isDart) {
				double baseVel = player->getCurrentXVelocity();
				double yVel = baseVel * -1.0 * dir;
				player->m_yVelocity = quantizeYVelocity(yVel);
			}

			lastEventTimestamp = input.m_timestamp;
			return;
		}

		if (player->m_isShip) {
			// Ship: holding state change handled by handleButton

		} else if (player->m_isBird) {
			if (player->m_isOnGround || player->m_stateRingJump) {
				float impulse = isMini ? 8.0f : 7.0f;
				impulse *= generalSizeScale;
				player->m_yVelocity = quantizeYVelocity(dir * impulse);
				player->m_isOnGround = false;
				player->m_isOnGround2 = false;
				player->m_stateRingJump = false;
				player->m_touchedPad = false;
			}

		} else if (player->m_isDart) {
			double baseVel = player->getCurrentXVelocity();
			double yVel = baseVel * 1.0 * dir;
			player->m_yVelocity = quantizeYVelocity(yVel);

		} else if (player->m_isBall) {
			if (player->m_isOnGround) {
				player->flipGravity(!player->m_isUpsideDown, true);
				player->m_yVelocity =
					quantizeYVelocity(player->m_yVelocity * 0.6);
				player->m_jumpBuffered = false;
				player->m_isOnGround = false;
			}

		} else if (player->m_isSwing) {
			double savedVel = player->m_yVelocity;
			player->flipGravity(!player->m_isUpsideDown, true);
			player->m_yVelocity = quantizeYVelocity(savedVel * 0.8);
			player->m_jumpBuffered = false;
			player->m_stateRingJump = false;
			player->m_isOnGround = false;

		} else if (player->m_isSpider) {
			if (player->m_isOnGround) {
				player->spiderTestJump(player->m_isUpsideDown);
				player->m_jumpBuffered = false;
			}

		} else if (player->m_isRobot) {
			if (player->m_isOnGround) {
				float impulse = player->m_yStart * 0.5f * generalSizeScale;
				player->m_yVelocity = quantizeYVelocity(dir * impulse);
				player->m_isOnGround = false;
				player->m_isOnGround2 = false;
				player->m_stateRingJump = false;
				player->m_touchedPad = false;
				player->m_accelerationOrSpeed = 0.0f;
				player->m_maybeIsBoosted = true;
			}

		} else {
			if (player->m_isOnGround) {
				float impulse = player->m_yStart * generalSizeScale;
				player->m_yVelocity = quantizeYVelocity(dir * impulse);
				player->m_isOnGround = false;
				player->m_isOnGround2 = false;
				player->m_stateRingJump = false;
				player->m_touchedPad = false;
				player->m_accelerationOrSpeed = 0.0f;
				player->m_maybeIsBoosted = true;
			}
		}

		lastEventTimestamp = input.m_timestamp;
	}

} // namespace continuousphysics::input