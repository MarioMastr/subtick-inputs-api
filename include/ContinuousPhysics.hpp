#pragma once

#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

namespace continuousphysics {

	namespace states {
		struct PlayerState {
			double lastEventTimestamp = 0.0;
		};

		struct ContinuousPhysicsState {
			double levelStartTimestamp = 0.0;
			bool firstFrame = true;
			uint64_t tickCount = 0;
			PlayerState player1;
			PlayerState player2;
		};

		inline ContinuousPhysicsState g_physicsState;
		inline uint64_t g_inputChecksCount = 0;

		/// @brief
		/// @param player pointer to the player object
		/// @return pointer to &g_physicsState.player1 or &g_physicsState.player2, or nullptr if player is null
		inline PlayerState* tryGetPlayerState(PlayerObject* player) {
			if (!player) return nullptr;
			return player->isPlayer1() ? &g_physicsState.player1 : &g_physicsState.player2;
		}
	} // namespace states

	namespace config {

		inline float g_tps = 240.0f;
		inline float g_inputHz = 240.0f;
		inline bool g_modActive = false;
		/// @brief patches that are required for velocity unrounding
		inline bool g_velocityUnroundingEnabled = false;
		/// @brief recreates 2.1 subframes by locking tps to be 4x input hz
		inline bool g_subframesEnabled = false;

	} // namespace config

	namespace physics {

		/// @brief rounds the velocity if velocity unrounding is disabled
		/// @return same value if velocity unrounding is enabled, otherwise the value rounded 3 decimals
		double quantizeYVelocity(double velocity);
		float evalYPosition(PlayerObject* player, double secondsSinceEvent);
		float evalXPosition(PlayerObject* player, double secondsSinceEvent);
		void advancePlayerToTimestamp(
			PlayerObject* player, double timestamp, double& lastEventTimestamp);
		void onPostCollision(PlayerObject* player);
		void updateTPS(float tps);

	} // namespace physics

	namespace input {

		/// @brief global input queue, sorted by timestamp
		inline std::vector<PlayerButtonCommand> g_inputQueue;

		void processInputsUpToTimestamp(PlayerObject* player);

		void handleInput(PlayerButtonCommand& input, PlayerObject* player,
			double& lastEventTimestamp);

	} // namespace input

	namespace tick {

		/// @brief whether to use vanilla PlayerObject::update(dt)
		/// @param player pointer to the player object
		/// @return true if player is dashing or if playLayer is null or mod is disabled, false otherwise
		bool useVanillaTick(PlayerObject* player);

		/// @brief called at the end of a tick to update player state
		/// @param player pointer to the player object
		void postTick(PlayerObject* player);

	} // namespace tick

	namespace patches {

		void toggleVelocityUnroundingPatches(bool enable);

	} // namespace patches

	namespace prelude {
		using namespace states;
		using namespace config;
		using namespace physics;
		using namespace input;
		using namespace tick;
		using namespace patches;
	} // namespace prelude
} // namespace continuousphysics