#pragma once

#include <ContinuousPhysics.hpp>

namespace continuousphysics {

	namespace physics {

		double quantizeYVelocity(double velocity);
		float getGravityAcceleration(PlayerObject* player, float tps);
		float evalYPosition(PlayerObject* player, double secondsSinceEvent);
		float evalXPosition(PlayerObject* player, double secondsSinceEvent);
		void advancePlayerToTimestamp(
			PlayerObject* player, double timestamp, double& lastEventTimestamp);
		void onPostCollision(PlayerObject* player);

	} // namespace physics

	namespace input {
		void processInputs(PlayerObject* player, double tickEnd);
	} // namespace input

	namespace tick {

		bool useVanillaTick(PlayerObject* player);
		void preTick(PlayerObject* player);
		void postTick(PlayerObject* player, float dt);

	} // namespace tick

	namespace patches {
		void toggleVelocityUnroundingPatches(bool enable);
	} // namespace patches

} // namespace continuousphysics
