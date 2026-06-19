#include <Geode/modify/PlayerObject.hpp>
#include <SubtickInputs.hpp>

#include "internal.hpp"

using namespace subtickinputs::physics;
using namespace subtickinputs::internal;

static PlayerObject* s_lastUpdateJumpPlayer = nullptr;

namespace subtickinputs::inputs {

	void processInputs(PlayerObject* player, float dt) {
		PlayLayer* playLayer = PlayLayer::get();
		auto& config = Config::get();
		if (!playLayer || !player || !player->isVanillaPlayer()) return;

		double tickStart = playLayer->m_timestamp;
		double tickDuration = static_cast<double>(dt);
		if (tickDuration <= 0.0) return;

		bool isPlayer1 = player->isPlayer1();
		auto& inputQueue = playLayer->m_queuedButtons;

		double tps = 1.0 / dt;
		double scaledDt = 60.0 / tps * 0.9;
		double inputChecksPerTick = config.getInputHz() / tps;

		bool isWave = player->m_isDart;
		double waveScale =
			60.0 / tps * ((player->m_vehicleSize != 1.0f) ? 2.0 : 1.0);

		double adjustedYVel = 0.0;

		for (auto& input : inputQueue) {
			if ((!input.m_isPlayer2) != isPlayer1) continue;

			double rawRatio = std::clamp(
				(input.m_timestamp - tickStart) / tickDuration, 0.0, 1.0);

			double ratio = rawRatio;
			if (!config.isInstantInputsEnabled()) {
				ratio = inputChecksPerTick <= 1.0
					? 0.0
					: std::floor(rawRatio * inputChecksPerTick) /
						inputChecksPerTick;
			}

			if (isWave && !player->m_isDashing) {
				// clang-format off
				bool ringPending =
				input.m_isPush &&
				player->m_touchingRings &&
				player->m_touchingRings->count() > 0;
				// clang-format on

				if (!ringPending) {
					getPendingWaveField(player).push_back({
						ratio,
						input.m_isPush,
						static_cast<int>(input.m_button),
					});
					continue;
				}
			}

			double preVel = player->m_yVelocity;
			double preDv = getGravPerTick(player, tps);

			s_lastUpdateJumpPlayer = nullptr;
			playLayer->handleButton(
				input.m_isPush, static_cast<int>(input.m_button), isPlayer1);

			if (s_lastUpdateJumpPlayer != player) {
				// vanilla explicitly does updateJump(0) but only for certain gamemodes
				// the check avoids double calling for those gamemodes
				player->updateJump(0.0f);
			}

			double postVel = player->m_yVelocity;
			double postDv = getGravPerTick(player, tps);

			adjustedYVel += ratio * ((preVel - postVel) + (preDv - postDv));
		}

		getYDispField(player) = adjustedYVel * (isWave ? waveScale : scaledDt);
	}

} // namespace subtickinputs::inputs

class $modify(PlayerObject) {
	void updateJump(float dt) {
		s_lastUpdateJumpPlayer = this;
		PlayerObject::updateJump(dt);
	}

	// split like cbf for wave
	// doesn't cause gravity issues since wave velocity is always constant
	void update(float dt) {
		auto& pendingWaveInputs = getPendingWaveField(this);

		if (useVanillaPhysics() || pendingWaveInputs.empty() ||
			!this->m_isDart || this->m_isDashing) {
			pendingWaveInputs.clear();
			PlayerObject::update(dt);
			return;
		}

		PlayLayer* playLayer = PlayLayer::get();
		bool isPlayer1 = this->isPlayer1();
		double lastRatio = 0.0;

		CCPoint preTickPosition = this->getPosition();

		for (auto& input : pendingWaveInputs) {
			double segment = std::max(0.0, input.m_ratio - lastRatio);
			if (segment > 0.0) {
				PlayerObject::update(segment * dt);
			}
			if (playLayer) {
				playLayer->handleButton(
					input.m_isPush, input.m_button, isPlayer1);
			}
			lastRatio = std::max(lastRatio, input.m_ratio);
		}
		pendingWaveInputs.clear();

		PlayerObject::update((1.0 - lastRatio) * dt);

		this->m_lastPosition = preTickPosition;
	}
};