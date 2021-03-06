// This file is part of Glest <https://github.com/Glest>
//
// Copyright (C) 2018  The Glest team
//
// Glest is a fork of MegaGlest <https://megaglest.org/>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>

#include "logger.h"

#include "util.h"
#include "renderer.h"
#include "properties.h"
#include "sound_renderer.h"
#include "core_data.h"
#include "metrics.h"
#include "lang.h"
#include "graphics_interface.h"
#include "game_constants.h"
#include "game_util.h"
#include "platform_util.h"
#include "leak_dumper.h"

using namespace std;
using namespace Shared::Graphics;
using namespace Shared::Util;

namespace Game {
	// =====================================================
	//	class Logger
	// =====================================================

	const int Logger::logLineCount = 15;

	// ===================== PUBLIC ========================

	Logger::Logger() : buttonCancel("Logger", "buttonCancel"), buttonNextHint("Logger", "buttonCancel") {
		progress = 0;
		string logs_path = getGameReadWritePath(GameConstants::path_logs_CacheLookupKey);
		if (logs_path != "") {
			fileName = logs_path + "log.txt";
		} else {
			string userData = Config::getInstance().getString("UserData_Root", "");
			if (userData != "") {
				endPathWithSlash(userData);
			}
			fileName = userData + "log.txt";
		}
		loadingTexture = NULL;
		gameHintToShow = "";
		showProgressBar = false;

		displayColor = Vec4f(1.f, 1.f, 1.f, 0.1f);

		cancelSelected = false;
		buttonCancel.setEnabled(false);

		buttonNextHint.setEnabled(false);
	}

	Logger::~Logger() {
	}

	Logger & Logger::getInstance() {
		static Logger logger;
		return logger;
	}

	void Logger::add(const string str, bool renderScreen, const string statusText) {
#ifdef WIN32
		FILE *f = _wfopen(utf8_decode(fileName).c_str(), L"at+");
#else
		FILE *f = fopen(fileName.c_str(), "at+");
#endif
		if (f != NULL) {
			fprintf(f, "%s\n", str.c_str());
			fclose(f);
		}
		this->current = str;
		this->statusText = statusText;

		if (renderScreen == true && GlobalStaticFlags::getIsNonGraphicalModeEnabled() == false) {
			renderLoadingScreen();
		}
	}

	void Logger::clear() {
		string s = "Log file\n";

#ifdef WIN32
		FILE *f = _wfopen(utf8_decode(fileName).c_str(), L"wt+");
#else
		FILE *f = fopen(fileName.c_str(), "wt+");
#endif
		if (f == NULL) {
			throw game_runtime_error("Error opening log file" + fileName);
		}

		fprintf(f, "%s", s.c_str());
		fprintf(f, "\n");

		fclose(f);
	}

	void Logger::loadLoadingScreen(string filepath) {
		SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d]\n", __FILE__, __FUNCTION__, __LINE__);

		if (filepath == "") {
			loadingTexture = NULL;
		} else {
			SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d] filepath = [%s]\n", __FILE__, __FUNCTION__, __LINE__, filepath.c_str());
			loadingTexture = Renderer::findTexture(filepath);
			SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d]\n", __FILE__, __FUNCTION__, __LINE__);
		}

		Lang &lang = Lang::getInstance();
		buttonCancel.setText(lang.getString("Cancel"));
	}

	void Logger::loadGameHints(string filePathEnglish, string filePathTranslation, bool clearList) {
		SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d]\n", __FILE__, __FUNCTION__, __LINE__);

		if ((filePathEnglish == "") || (filePathTranslation == "")) {
			return;
		} else {
			SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d] filePathEnglish = [%s]\n filePathTranslation = [%s]\n", __FILE__, __FUNCTION__, __LINE__, filePathEnglish.c_str(), filePathTranslation.c_str());
			gameHints.load(filePathEnglish, clearList);
			gameHintsTranslation.load(filePathTranslation, clearList);
			showNextHint();

			Lang &lang = Lang::getInstance();
			buttonNextHint.setText(lang.getString("ShowNextHint"));
			buttonCancel.setText(lang.getString("Cancel"));

			GraphicComponent::applyAllCustomProperties("Loading");
			SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d]\n", __FILE__, __FUNCTION__, __LINE__);
		}
	}

	void Logger::showNextHint() {
		string key = gameHints.getRandomKey(true);
		string tmpString = gameHintsTranslation.getString(key, "");
		if (tmpString != "") {
			gameHintToShow = tmpString;
		} else {
			SystemFlags::OutputDebug(SystemFlags::debugError, "In [%s::%s Line: %d] key [%s] not found for [%s] hint translation\n", __FILE__, __FUNCTION__, __LINE__, key.c_str(), Lang::getInstance().getLanguage().c_str());
			tmpString = gameHints.getString(key, "");
			if (tmpString != "") {
				gameHintToShow = tmpString;
			} else {
				gameHintToShow = "Problems to resolve hint key '" + key + "'";
			}
		}
		replaceAll(gameHintToShow, "\\n", "\n");

		Config &configKeys = Config::getInstance(std::pair<ConfigType, ConfigType>(cfgMainKeys, cfgUserKeys));

		vector<pair<string, string> > mergedKeySettings = configKeys.getMergedProperties();
		for (unsigned int j = 0; j < mergedKeySettings.size(); ++j) {
			pair<string, string> &property = mergedKeySettings[j];
			replaceAll(gameHintToShow, "#" + property.first + "#", property.second);
		}
		SystemFlags::OutputDebug(SystemFlags::debugSystem, "In [%s::%s Line: %d]\n", __FILE__, __FUNCTION__, __LINE__);
	}

	void Logger::clearHints() {
		gameHintToShow = "";
		gameHints.clear();
		gameHintsTranslation.clear();
	}

	void Logger::handleMouseClick(int x, int y) {
		if (buttonCancel.getEnabled() == true) {
			if (buttonCancel.mouseClick(x, y)) {
				cancelSelected = true;
			}
		}
		if (buttonNextHint.getEnabled() == true) {
			if (buttonNextHint.mouseClick(x, y) == true) {
				showNextHint();
				//buttonNextHint.setLighted(false);
				SoundRenderer &soundRenderer = SoundRenderer::getInstance();
				CoreData &coreData = CoreData::getInstance();
				soundRenderer.playFx(coreData.getClickSoundC());
			}
		}
	}

	// ==================== PRIVATE ====================

	void Logger::renderLoadingScreen() {

		Renderer &renderer = Renderer::getInstance();
		CoreData &coreData = CoreData::getInstance();
		const Metrics &metrics = Metrics::getInstance();

		//3d
		//renderer.reset3d();
		//renderer.clearZBuffer();

		renderer.reset2d();
		renderer.clearBuffers();
		if (loadingTexture == NULL) {
			renderer.renderBackground(CoreData::getInstance().getBackgroundTexture());
		} else {
			renderer.renderBackground(loadingTexture);
		}

		if (showProgressBar == true) {
			if (Renderer::renderText3DEnabled) {
				renderer.renderProgressBar3D(
					progress,
					metrics.getVirtualW() / 4,
					59 * metrics.getVirtualH() / 100,
					coreData.getDisplayFontSmall3D(),
					500, ""); // no string here, because it has to be language specific and does not give much information
			} else {
				renderer.renderProgressBar(
					progress,
					metrics.getVirtualW() / 4,
					59 * metrics.getVirtualH() / 100,
					coreData.getDisplayFontSmall(),
					500, ""); // no string here, because it has to be language specific and does not give much information
			}
		}

		int xLocation = metrics.getVirtualW() / 4;
		if (Renderer::renderText3DEnabled) {

			renderer.renderTextShadow3D(
				state, coreData.getMenuFontBig3D(), displayColor,
				xLocation,
				65 * metrics.getVirtualH() / 100, false);

			renderer.renderTextShadow3D(
				current, coreData.getMenuFontNormal3D(), displayColor,
				xLocation,
				62 * metrics.getVirtualH() / 100, false);

			if (this->statusText != "") {
				renderer.renderTextShadow3D(
					this->statusText, coreData.getMenuFontNormal3D(), displayColor,
					xLocation,
					56 * metrics.getVirtualH() / 100, false);
			}
		} else {
			renderer.renderTextShadow(
				state, coreData.getMenuFontBig(), displayColor,
				xLocation,
				65 * metrics.getVirtualH() / 100, false);

			renderer.renderTextShadow(
				current, coreData.getMenuFontNormal(), displayColor,
				xLocation,
				62 * metrics.getVirtualH() / 100, false);

			if (this->statusText != "") {
				renderer.renderTextShadow(
					this->statusText, coreData.getMenuFontNormal(), displayColor,
					xLocation,
					56 * metrics.getVirtualH() / 100, false);
			}
		}

		Lang &lang = Lang::getInstance();
		if (gameHintToShow != "") {
			string hintText = lang.getString("Hint", "");
			char szBuf[8096] = "";
			snprintf(szBuf, 8096, hintText.c_str(), gameHintToShow.c_str());
			hintText = szBuf;
			int yLocationHint = 90 * metrics.getVirtualH() / 100;
			if (Renderer::renderText3DEnabled) {
				int xLocationHint = (metrics.getVirtualW() / 2) - (coreData.getMenuFontBig3D()->getMetrics()->getTextWidth(hintText) / 2);

				renderer.renderTextShadow3D(
					hintText, coreData.getMenuFontBig3D(), displayColor,
					//xLocation*1.5f,
					xLocationHint,
					yLocationHint, true);
			} else {
				int xLocationHint = (metrics.getVirtualW() / 2) - (coreData.getMenuFontBig()->getMetrics()->getTextWidth(hintText) / 2);

				renderer.renderTextShadow(
					hintText, coreData.getMenuFontBig(), displayColor,
					//xLocation*1.5f,
					xLocationHint,
					yLocationHint, true);

			}
			//Show next Hint
			if (buttonNextHint.getEnabled() == false) {
				buttonNextHint.init((metrics.getVirtualW() / 2) - (175 / 2), yLocationHint + 20, 175);
				buttonNextHint.setText(lang.getString("ShowNextHint"));
				buttonNextHint.setEnabled(true);
				buttonNextHint.setVisible(true);
				buttonNextHint.setEditable(true);
			}

			renderer.renderButton(&buttonNextHint);

			/*
					if(Renderer::renderText3DEnabled) {
						int xLocationHint =  (metrics.getVirtualW() / 2) - (coreData.getMenuFontBig3D()->getMetrics()->getTextWidth(hintText) / 2);

						renderer.renderText3D(
								lang.getString("ShowNextHint","",true), coreData.getMenuFontNormal3D(), nextHintTitleColor,
								//xLocation*1.5f,
								xLocationHint,
								93 * metrics.getVirtualH() / 100, false);
					}
					else {
						int xLocationHint =  (metrics.getVirtualW() / 2) - (coreData.getMenuFontBig()->getMetrics()->getTextWidth(hintText) / 2);

						renderer.renderText(
								lang.getString("ShowNextHint","",true), coreData.getMenuFontNormal(), nextHintTitleColor,
							//xLocation*1.5f,
							xLocationHint,
							93 * metrics.getVirtualH() / 100, false);

					}
			*/

		}

		if (Config::getInstance().getBool("EnableLoadCancel", "false")) {
			if (buttonCancel.getEnabled() == false) {
				buttonCancel.init((metrics.getVirtualW() / 2) - (125 / 2), 50 * metrics.getVirtualH() / 100, 125);
				buttonCancel.setText(lang.getString("Cancel"));
				buttonCancel.setEnabled(true);
				buttonCancel.setVisible(true);
				buttonCancel.setEditable(true);
			}
			renderer.renderButton(&buttonCancel);
		}

		renderer.swapBuffers();
	}
} //end namespace
