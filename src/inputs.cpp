#include <ContinuousPhysics.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace continuousphysics::physics;

static PlayerObject* s_lastUpdateJumpPlayer = nullptr;

struct PendingWaveInput {
	double m_ratio;
	bool m_isPush;
	int m_button;
};

static std::vector<PendingWaveInput> s_pendingWaveInputs[2]; // [0]=p1 [1]=p2

static std::vector<PendingWaveInput>& getPendingWaveInputsForPlayer(
	PlayerObject* player) {
	return s_pendingWaveInputs[player->isPlayer1() ? 0 : 1];
}

namespace continuousphysics::inputs {

	void processInputs(PlayerObject* player, float dt) {
		PlayLayer* playLayer = PlayLayer::get();
		auto& config = Config::get();
		auto* playerState =
			ContinuousPhysicsState::get().tryGetPlayerState(player);
		if (!playLayer || !playerState) return;

		double tickStart = playLayer->m_timestamp;
		double tickDuration = static_cast<double>(dt);
		if (tickDuration <= 0.0) return;

		bool isPlayer1 = player->isPlayer1();
		auto& inputQueue = playLayer->m_queuedButtons;

		double tps = config.getTPS();
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
			double ratio = (inputChecksPerTick <= 1.0)
				? 0.0
				: std::floor(rawRatio * inputChecksPerTick) /
					inputChecksPerTick;

			if (isWave && !player->m_isDashing) {
				// clang-format off
				bool ringPending =
				input.m_isPush &&
				player->m_touchingRings &&
				player->m_touchingRings->count() > 0;
				// clang-format on

				if (!ringPending) {
					getPendingWaveInputsForPlayer(player).push_back({
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

		playerState->m_yDispAdjustment =
			adjustedYVel * (isWave ? waveScale : scaledDt);
	}

} // namespace continuousphysics::inputs

class $modify(PlayerObject) {
	void updateJump(float dt) {
		s_lastUpdateJumpPlayer = this;
		PlayerObject::updateJump(dt);
	}

	// split like legacy cbf for wave trail stuff
	// doesn't cause gravity issues since wave velocity is always constant
	void update(float dt) {
		auto& pendingWaveInputs = getPendingWaveInputsForPlayer(this);

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