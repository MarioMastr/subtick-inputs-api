#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace continuousphysics {

	namespace states {
		struct PlayerState {
			double m_lastEventTimestamp = 0.0;
		};

		struct ContinuousPhysicsState {
			double m_levelStartTimestamp = 0.0;
			bool m_firstFrame = true;
			uint64_t m_tickCount = 0;
			uint64_t m_inputChecksCount = 0;
			PlayerState m_player1;
			PlayerState m_player2;
		};

		inline ContinuousPhysicsState g_physicsState;

		/// @brief tries to get the corresponding PlayerState for a given PlayerObject
		/// @param player pointer to the player object
		/// @return pointer to &g_physicsState.m_player1 or &g_physicsState.m_player2, or nullptr if player is null
		inline PlayerState* tryGetPlayerState(PlayerObject* player) {
			if (!player) return nullptr;
			return player->isPlayer1() ? &g_physicsState.m_player1
									   : &g_physicsState.m_player2;
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

		/// @brief calculates the gravity acceleration for a player
		/// @param tps the tps to replicate the gravity of
		/// @return the gravity acceleration in yVels/sec
		/// where 1 yVel = 54 units/sec (and 1 unit = 1/30th of a block)
		float getGravityAcceleration(PlayerObject* player, float tps);

		/// @brief evaluates the y position of a player based on the time since the last event
		/// @param secondsSinceEvent seconds since the last "event"
		/// (input check, tick, collision, frame, etc.)
		/// @return the new y position of the player
		float evalYPosition(PlayerObject* player, double secondsSinceEvent);

		/// @brief evaluates the x position of a player based on the time since the last event
		/// @param secondsSinceEvent seconds since the last "event"
		/// (input check, tick, collision, frame, etc.)
		/// @return the new x position of the player
		float evalXPosition(PlayerObject* player, double secondsSinceEvent);

		/// @brief updates the player's position to where it should be at the given timestamp
		/// based on the time since the last event
		/// @param timestamp the timestamp to advance the player to
		/// @param lastEventTimestamp the timestamp of the last "event" (input check, tick, collision, frame, etc.)
		void advancePlayerToTimestamp(
			PlayerObject* player, double timestamp, double& lastEventTimestamp);

		/// @brief called after GJBaseGameLayer::checkCollisions
		void onPostCollision(PlayerObject* player);

		/// @brief sets g_tps to the given value
		/// (or 4x g_inputHz if subframes are enabled)
		/// @param tps the new tps value to set
		void updateTPS(float tps);

	} // namespace physics

	namespace input {

		/// @brief processes all inputs from PlayLayer.m_queuedButtons for a player at a tick,
		/// calling advancePlayerToTimestamp and handleInput for each input event
		void processInputs(PlayerObject* player);

		/// @brief handles a single input event for a player, updating the player's velocity and state accordingly
		/// @param input the input event to handle
		void handleInput(PlayerButtonCommand& input, PlayerObject* player,
			double& lastEventTimestamp);

	} // namespace input

	namespace tick {

		/// @brief whether to use vanilla PlayerObject::update(dt) and skip the continuous physics logic in preTick and postTick
		/// @return true if player is dashing or if playLayer is null or mod is disabled, false otherwise
		bool useVanillaTick(PlayerObject* player);

		/// @brief called before PlayerObject::update(dt) to process inputs and pre-compensate gravity
		void preTick(PlayerObject* player);

		/// @brief called after PlayerObject::update(dt) to update physics state
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