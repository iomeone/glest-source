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

#include <cassert>
#include "logger.h"
#include "lang.h"
#include "renderer.h"
#include "leak_dumper.h"

using namespace Shared::Util;

namespace Game {
	ProjectileType::ProjectileType() {

		projectileParticleSystemType = NULL;
		attackStartTime = 0.0f;

		spawnUnit = "";
		spawnUnitcount = 0;
		spawnUnitAtTarget = false;

		shake = false;
		shakeIntensity = 0;
		shakeDuration = 0;

		shakeVisible = true;
		shakeInCameraView = true;
		shakeCameraDistanceAffected = false;
		damagePercentage = 100;
	}

	ProjectileType::~ProjectileType() {
		deleteValues(hitSounds.getSounds().begin(),
			hitSounds.getSounds().end());
		if (projectileParticleSystemType != NULL) {
			delete projectileParticleSystemType;
			projectileParticleSystemType = NULL;
		}
	}

	void ProjectileType::load(const XmlNode * projectileNode,
		const string & dir,
		const string & techtreepath, std::map < string,
		vector < pair < string,
		string > > >&loadedFileList,
		string parentLoader) {

		string currentPath = dir;
		endPathWithSlash(currentPath);

		if (projectileNode->hasAttribute("attack-start-time")) {
			attackStartTime =
				projectileNode->getAttribute("attack-start-time")->
				getFloatValue();
		} else {
			attackStartTime = 0.0f;
		}

		// damage percentage MUST be set!
		damagePercentage =
			projectileNode->getAttribute("damage-percentage")->getIntValue();


		// projectiles MUST have a particle system.
		const XmlNode *particleNode = projectileNode->getChild("particle");
		string path =
			particleNode->getAttribute("path")->getRestrictedValue();
		ParticleSystemTypeProjectile *projectileParticleSystemType =
			new ParticleSystemTypeProjectile();
		projectileParticleSystemType->load(particleNode, dir,
			currentPath + path,
			&Renderer::getInstance(),
			loadedFileList, parentLoader,
			techtreepath);
		loadedFileList[currentPath +
			path].push_back(make_pair(parentLoader,
				particleNode->
				getAttribute("path")->
				getRestrictedValue()));
		setProjectileParticleSystemType(projectileParticleSystemType);

		//spawnattack
		if (projectileNode->hasChild("unit")) {
			spawnUnit =
				projectileNode->getChild("unit")->getAttribute("value")->
				getValue();
			spawnUnitcount =
				projectileNode->getChild("unit")->getAttribute("amount")->
				getIntValue();
			if (projectileNode->getChild("unit")->hasAttribute("spawnAtTarget")) {
				spawnUnitAtTarget =
					projectileNode->getChild("unit")->
					getAttribute("spawnAtTarget")->getBoolValue();
			} else {
				spawnUnitAtTarget = false;
			}
		} else {
			spawnUnit = "";
			spawnUnitcount = 0;
			spawnUnitAtTarget = false;
		}


		if (projectileNode->hasChild("hitshake")) {
			const XmlNode *hitShakeNode = projectileNode->getChild("hitshake");
			shake = hitShakeNode->getAttribute("enabled")->getBoolValue();
			if (shake) {
				shakeIntensity =
					hitShakeNode->getAttribute("intensity")->getIntValue();
				shakeDuration =
					hitShakeNode->getAttribute("duration")->getIntValue();

				shakeVisible =
					hitShakeNode->getAttribute("visible")->getBoolValue();
				shakeInCameraView =
					hitShakeNode->getAttribute("in-camera-view")->getBoolValue();
				shakeCameraDistanceAffected =
					hitShakeNode->getAttribute("camera-distance-affected")->
					getBoolValue();
			}
		}

		if (projectileNode->hasChild("hitsound")) {
			const XmlNode *soundNode = projectileNode->getChild("hitsound");
			if (soundNode->getAttribute("enabled")->getBoolValue()) {

				hitSounds.resize((int) soundNode->getChildCount());
				for (int i = 0; i < (int) soundNode->getChildCount(); ++i) {
					const XmlNode *soundFileNode =
						soundNode->getChild("sound-file", i);
					string path =
						soundFileNode->getAttribute("path")->
						getRestrictedValue(currentPath, true);
					//printf("\n\n\n\n!@#$ ---> parentLoader [%s] path [%s] nodeValue [%s] i = %d",parentLoader.c_str(),path.c_str(),soundFileNode->getAttribute("path")->getRestrictedValue().c_str(),i);

					StaticSound *sound = new StaticSound();
					sound->load(path);
					loadedFileList[path].
						push_back(make_pair
						(parentLoader,
							soundFileNode->getAttribute("path")->
							getRestrictedValue()));
					hitSounds[i] = sound;
				}
			}
		}
	}
} //end namespace
