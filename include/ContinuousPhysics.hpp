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
		struct PlayerState {
			double m_lastEventTimestamp = 0.0;
		};

		static ContinuousPhysicsState& get();

		/// @brief tries to get the corresponding PlayerState for a given PlayerObject
		/// @return pointer to &g_physicsState.m_player1 or &g_physicsState.m_player2, or nullptr if player is null
		PlayerState* tryGetPlayerState(PlayerObject* player) {
			if (!player) return nullptr;
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

		bool isModActive() const {
			return m_modActive;
		}
		void setModActive(bool v) {
			m_modActive = v;
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
		bool m_modActive = false;
		bool m_velocityUnroundingEnabled = false;
	};

} // namespace continuousphysics