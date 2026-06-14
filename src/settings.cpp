#include <ContinuousPhysics.hpp>

using namespace continuousphysics;

static bool s_apiDisabled = false;

bool Config::isApiDisabled() const {
	return s_apiDisabled;   
}

$on_mod(Loaded) {
	auto mod = Mod::get();
	s_apiDisabled = mod->getSettingValue<bool>("api-disabled");
	listenForSettingChanges<bool>(
		"api-disabled", +[](bool val) { s_apiDisabled = val; });
}