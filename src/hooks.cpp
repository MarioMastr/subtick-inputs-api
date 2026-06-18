// there's a couple hooks in inputs.cpp as well

#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#ifdef GEODE_IS_WINDOWS
#include <safetyhook.hpp>
#endif
#include <SubtickInputs.hpp>

#include "internal.hpp"

using namespace subtickinputs;
using namespace subtickinputs::physics;
using namespace subtickinputs::internal;

#ifdef GEODE_IS_WINDOWS
static SafetyHookMid s_yDispMidHook;
#endif

class $modify(CCEGLView) {
	void pollEvents() {
		PlayLayer* playLayer = PlayLayer::get();
		CCNode* parent;

		// clang-format off
		if (
		#ifdef GEODE_IS_WINDOWS
			!GetFocus()
		#endif
			|| !playLayer
			|| !(parent = playLayer->getParent())
			|| parent->getChildByType<PauseLayer>(0)
			|| playLayer->getChildByType<EndLevelLayer>(0)
			|| playLayer->m_playerDied)
		{
			g_firstFrame = true;
		}
		// clang-format on

		CCEGLView::pollEvents();
	}
};

class $modify(GJBaseGameLayer) {
	void update(float dt) {
		if (PlayLayer::get() && PlayLayer::get()->m_playerDied) {
			g_firstFrame = true;
		}

		GJBaseGameLayer::update(dt);

		if (g_firstFrame) {
			g_firstFrame = false;
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

#ifdef GEODE_IS_WINDOWS
$on_mod(Loaded) {
	auto addr = reinterpret_cast<void*>(base::get() + 0x3895a3);
	s_yDispMidHook = safetyhook::create_mid(addr, [](SafetyHookContext& ctx) {
		if (useVanillaPhysics()) return;

		auto* player = reinterpret_cast<PlayerObject*>(ctx.r15);
		if (!player || !player->isVanillaPlayer()) return;
		if (player->m_isDashing) return;

		ctx.xmm6.f64[0] += getYDispField(player);
		getYDispField(player) = 0.0;
	});
}
#endif
