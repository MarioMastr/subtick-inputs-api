#include <SubtickInputs.hpp>

static bool s_apiDisabled = false;

namespace subtickinputs {

	Config& Config::get() {
		static Config instance;
		return instance;
	}

	bool Config::isApiDisabled() const {
		return s_apiDisabled;
	}

} // namespace subtickinputs

$on_mod(Loaded) {
	auto mod = Mod::get();
	s_apiDisabled = mod->getSettingValue<bool>("api-disabled");
	listenForSettingChanges<bool>("api-disabled", +[](bool val) { s_apiDisabled = val; });
}