#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// clang-format off
#ifdef GEODE_IS_WINDOWS
	#ifdef ContinuousPhysicsAPI_EXPORTING
		#define CP_API __declspec(dllexport)
	#else
		#define CP_API __declspec(dllimport)
	#endif
#else
	#define CP_API __attribute__((visibility("default")))
#endif
// clang-format on

namespace continuousphysics {

	class CP_API ContinuousPhysicsState {
		public:
		// lowk not sure how to add my own members to a class
		struct PlayerState {
			/// @brief accumulated Y displacement adjustment for sub-tick precision.
			/// computed in processInputs, consumed by the Y displacement midhook.
			double m_yDispAdjustment = 0.0;
		};

		static ContinuousPhysicsState& get();

		/// @brief tries to get the corresponding PlayerState for a given PlayerObject
		/// @return pointer to &g_physicsState.m_player1 or &g_physicsState.m_player2, or nullptr if player is null
		PlayerState* tryGetPlayerState(PlayerObject* player) {
			if (!player || !player->isVanillaPlayer()) return nullptr;
			auto& physicsState = ContinuousPhysicsState::get();
			return player->isPlayer1() ? &physicsState.m_player1
									   : &physicsState.m_player2;
		}

		bool m_firstFrame = true;
		PlayerState m_player1;
		PlayerState m_player2;

		ContinuousPhysicsState(const ContinuousPhysicsState&) = delete;
		ContinuousPhysicsState& operator=(
			const ContinuousPhysicsState&) = delete;

		private:
		ContinuousPhysicsState() = default;
	};

	class CP_API Config {
		public:
		static Config& get();

		bool isApiDisabled() const;

		float getTPS() const {
			return m_tps;
		}
		void setTPS(float v) {
			m_tps = v;
		}

		float getInputHz() const {
			return m_inputHz;
		}
		void setInputHz(float v) {
			m_inputHz = v;
		}

		bool isVelocityUnroundingEnabled() const {
			return m_velocityUnroundingEnabled;
		}
		void setVelocityUnroundingEnabled(bool v) {
			m_velocityUnroundingEnabled = v;
		}

		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;

		private:
		Config() = default;

		float m_tps = 240.0f;
		float m_inputHz = 240.0f;
		bool m_velocityUnroundingEnabled = false;
	};

	namespace physics {
		/// @brief whether to skip continuous physics logic and use vanilla behavior
		/// @return true if playLayer is null, mod is disabled, first frame after
		/// pause/death/init, player died, platformer mode, or robtop's replay mode
		CP_API bool useVanillaPhysics();

		CP_API float getBaseGravity(PlayerObject* player);

		CP_API float getGravityCoefficient(PlayerObject* player);

		CP_API double getGravPerTick(PlayerObject* player, float tps);

	} // namespace physics

	namespace inputs {

		/// @brief processes this player's inputs from PlayLayer.m_queuedButtons
		/// for the current tick: dispatches each via handleButton + updateJump(0)
		/// and accumulates the sub-tick Y displacement adjustment
		/// (impulse + accel terms) into PlayerState for the midhook to apply
		/// @param dt the tick duration (the dt passed to processQueuedButtons)
		CP_API void processInputs(PlayerObject* player, float dt);

	} // namespace inputs

	namespace patches {

		/// @brief sets whether to apply nop patches to remove manual velocity rounding in various places
		CP_API void toggleVelocityUnroundingPatches(bool enable);

	} // namespace patches

	namespace prelude {
		using namespace continuousphysics;
		using namespace continuousphysics::inputs;
		using namespace continuousphysics::patches;
		using namespace continuousphysics::physics;
	} // namespace prelude

} // namespace continuousphysics
