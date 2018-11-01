// This file is part of ZetaGlest <https://github.com/ZetaGlest>
//
// Copyright (C) 2018  The ZetaGlest team
//
// ZetaGlest is a fork of MegaGlest <https://megaglest.org>
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

#ifndef _SHARED_GRAPHICS_GL_GRAPHICSFACTORYGL_H_
#define _SHARED_GRAPHICS_GL_GRAPHICSFACTORYGL_H_

#include "texture_manager.h"
#include "model_manager.h"
#include "particle.h"
#include "font_manager.h"
#include "graphics_factory.h"
#include "text_renderer_gl.h"
#include "model_renderer_gl.h"
#include "particle_renderer_gl.h"
#include "context_gl.h"
#include "model_gl.h"
#include "texture_gl.h"
#include "font_gl.h"
#include "leak_dumper.h"

namespace Shared {
	namespace Graphics {
		namespace Gl {

			// =====================================================
			//	class GraphicsFactoryGl
			// =====================================================

			class GraphicsFactoryGl : public GraphicsFactory {
			public:
				//context
				virtual Context *newContext() {
					return new ContextGl();
				}

				//textures
				virtual TextureManager *newTextureManager() {
					return new TextureManager();
				}
				virtual Texture1D *newTexture1D() {
					return new Texture1DGl();
				}
				virtual Texture2D *newTexture2D() {
					return new Texture2DGl();
				}
				virtual Texture3D *newTexture3D() {
					return new Texture3DGl();
				}
				virtual TextureCube *newTextureCube() {
					return new TextureCubeGl();
				}

				//models
				virtual ModelManager *newModelManager() {
					return new ModelManager();
				}
				virtual ModelRenderer *newModelRenderer() {
					return new ModelRendererGl();
				}
				virtual Model *newModel(const string &path, TextureManager* textureManager, bool deletePixMapAfterLoad, std::map<string, vector<pair<string, string> > > *loadedFileList, string *sourceLoader) {
					return new ModelGl(path, textureManager, deletePixMapAfterLoad, loadedFileList, sourceLoader);
				}

				//text
				virtual FontManager *newFontManager() {
					return new FontManager();
				}
				virtual TextRenderer2D *newTextRenderer2D() {
					return new TextRenderer2DGl();
				}
				virtual TextRenderer3D *newTextRenderer3D() {
					return new TextRenderer3DGl();
				}
				virtual Font2D *newFont2D() {
					return new Font2DGl();
				}
				virtual Font3D *newFont3D() {
					return new Font3DGl();
				}

				//particles
				virtual ParticleManager *newParticleManager() {
					return new ParticleManager();
				}
				virtual ParticleRenderer *newParticleRenderer() {
					return new ParticleRendererGl();
				}
			};

		}
	}
}//end namespace

#endif
