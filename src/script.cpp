// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "script.h"

#include "configmanager.h"

#include <fmt/color.h>

extern LuaEnvironment g_luaEnvironment;

Scripts::Scripts() : scriptInterface("Scripts Interface") { scriptInterface.initState(); }

Scripts::~Scripts() { scriptInterface.reInitState(); }

bool Scripts::loadScripts(const std::string& folderName, bool isLib, bool reload)
{
	namespace fs = std::filesystem;

	const auto dir = fs::current_path() / "data" / folderName;
	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		std::cout << "[Warning - Scripts::loadScripts] Can not load folder '" << folderName << "'." << std::endl;
		return false;
	}

	bool scriptsConsoleLogs = getBoolean(ConfigManager::SCRIPTS_CONSOLE_LOGS);
	std::vector<std::string> disabled = {}, loaded = {}, reloaded = {};

	fs::recursive_directory_iterator endit;
	std::vector<fs::path> v;
	std::string disable = ("#");
	for (fs::recursive_directory_iterator it(dir); it != endit; ++it) {
		auto fn = it->path().parent_path().filename();
		if ((fn == "lib" && !isLib) || fn == "events") {
			continue;
		}
		if (fs::is_regular_file(*it) && it->path().extension() == ".lua") {
			size_t found = it->path().filename().string().find(disable);
			if (found != std::string::npos) {
				if (scriptsConsoleLogs) {
					const auto& scrName = it->path().filename().string();
					disabled.push_back(
					    fmt::format("\"{}\"", fmt::format(fg(fmt::color::yellow), "{}",
					                                      std::string_view(scrName.data(), scrName.size() - 4))));
				}
				continue;
			}
			v.push_back(it->path());
		}
	}
	sort(v.begin(), v.end());
	for (auto it = v.begin(); it != v.end(); ++it) {
		const std::string scriptFile = it->string();
		if (scriptInterface.loadFile(scriptFile) == -1) {
			std::cout << "> " << it->filename().string() << " [error]" << std::endl;
			std::cout << "^ " << scriptInterface.getLastLuaError() << std::endl;
			continue;
		}

		if (scriptsConsoleLogs) {
			const auto& scrName = it->filename().string();
			if (!reload) {
				loaded.push_back(fmt::format(
				    "\"{}\"",
				    fmt::format(fg(fmt::color::green), "{}", std::string_view(scrName.data(), scrName.size() - 4))));
			} else {
				reloaded.push_back(fmt::format(
				    "\"{}\"",
				    fmt::format(fg(fmt::color::green), "{}", std::string_view(scrName.data(), scrName.size() - 4))));
			}
		}
	}

	if (scriptsConsoleLogs) {
		if (!disabled.empty()) {
			std::string disabledStr = "{";
			for (size_t i = 0; i < disabled.size(); ++i) {
				if (i > 0) {
					disabledStr += ", ";
				}
				disabledStr += disabled[i];
			}
			disabledStr += "}";
			std::cout << disabledStr << std::endl;
		}

		if (!loaded.empty()) {
			std::string loadedStr = "{";
			for (size_t i = 0; i < loaded.size(); ++i) {
				if (i > 0) {
					loadedStr += ", ";
				}
				loadedStr += loaded[i];
			}
			loadedStr += "}";
			std::cout << loadedStr << std::endl;
		}

		if (!reloaded.empty()) {
			std::string reloadedStr = "{";
			for (size_t i = 0; i < reloaded.size(); ++i) {
				if (i > 0) {
					reloadedStr += ", ";
				}
				reloadedStr += reloaded[i];
			}
			reloadedStr += "}";
			std::cout << reloadedStr << std::endl;
		}
	}

	return true;
}
