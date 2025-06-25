/*
!@
MIT License

Copyright (c) 2019 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#pragma once

#include "Shader/CShaderParams.h"
#include "Shader/CShader.h"

#include "GameObject/CGameObject.h"

namespace Skylicht
{
	/// @brief The object class describes material information
	/// such as which shader it's associated with, which texture it uses, and what its color parameters are.
	/// @ingroup Materials
	/// 
	/// **Skylicht-Editor**
	/// 
	/// You can create materials in two ways
	/// 
	/// **Method 1:** Select the model file (.fbx, .dae) and click the "Export Material" button
	/// @image html Material/export-material.jpg
	/// 
	/// **Method 2:** Create it manually by choosing the menu Asset/Create/Material.
	/// @image html Material/material-asset.jpg
	/// 
	/// **Adjusting materials:**
	/// 
	/// The Material Property window allows you to add multiple materials within a .mat file. It also includes functions like delete/rename.
	/// 
	/// @image html Material/material-property-window.jpg "Material property"
	/// <br>
	/// @image html Material/material-tool.jpg "Delete/Rename Material"
	/// 
	/// **Next**, drag the shader .xml file into the Shader Field to select the material's shader. 
	/// 
	/// From there, you can then drag texture files into the various uniform textures, or adjust the parameters defined within the shader .xml.
	/// 
	/// @image html Material/material.jpg
	/// 
	/// Materials are essential for the CRenderMesh component to display the model's appearance on the Skylicht-Engine
	/// 
	/// @image html RenderMesh/render-mesh-property.jpg width=1200px
	/// 
	/// 
	/// You use CTextureManager to support loading .mat material files.
	/// @code
	/// std::vector<std::string> textureFolders;
	/// ArrayMaterial& materials = CMaterialManager::getInstance()->loadMaterial(
	/// 	"SampleModels/BlendShape/Cat.mat",
	/// 	true,
	/// 	textureFolders
	/// );
	/// 
	/// for (CMaterial* mat: materials)
	/// {
	/// 	printf("%s\n", mat->getName());
	/// }
	/// @endcode
	class SKYLICHT_API CMaterial : public IReferenceCounted
	{
	public:
		struct SUniformTexture
		{
			std::string Name;
			std::string Path;
			ITexture* Texture;
			int TextureSlot;
			int WrapU;
			int WrapV;

			int Anisotropic;
			bool Trilinear;
			bool Bilinear;

			SUniformTexture()
			{
				Texture = NULL;
				TextureSlot = -1;
				WrapU = 0; // Repeat
				WrapV = 0; // Repeat
				Anisotropic = 2;
				Trilinear = false;
				Bilinear = false;
			}

			SUniformTexture* clone()
			{
				SUniformTexture* c = new SUniformTexture();
				c->Name = Name;
				c->Path = Path;
				c->Texture = Texture;
				c->TextureSlot = TextureSlot;
				c->WrapU = WrapU;
				c->WrapV = WrapV;
				c->Anisotropic = Anisotropic;
				c->Trilinear = Trilinear;
				c->Bilinear = Bilinear;
				return c;
			}
		};

		struct SUniformValue
		{
			std::string Name;
			float FloatValue[4];
			int FloatSize;
			bool ShaderDefaultValue;

			EUniformType Type;
			int ValueIndex;

			SUniformValue()
			{
				ShaderDefaultValue = false;
				FloatSize = 0;
				Type = NUM_SHADER_TYPE;
				ValueIndex = -1;
				memset(FloatValue, 0, 4 * sizeof(float));
			}

			SUniformValue* clone()
			{
				SUniformValue* c = new SUniformValue();
				memcpy(c->FloatValue, FloatValue, sizeof(float) * 4);
				c->Name = Name;
				c->FloatSize = FloatSize;
				c->ShaderDefaultValue = ShaderDefaultValue;
				c->Type = Type;
				c->ValueIndex = ValueIndex;
				return c;
			}
		};

		struct SExtraParams
		{
			std::string ShaderPath;
			std::vector<SUniformValue*> UniformParams;
			std::vector<SUniformTexture*> UniformTextures;
		};

	private:
		// this will copy value from m_uniformParams (see function updateShaderParams)
		CShaderParams m_shaderParams;

		std::string m_package;
		std::string m_materialName;
		std::string m_shaderPath;
		std::string m_materialPath;

		std::vector<SUniformValue*> m_uniformParams;
		std::vector<SUniformTexture*> m_uniformTextures;

		std::vector<SExtraParams*> m_extras;

		ITexture* m_resourceTexture[MATERIAL_MAX_TEXTURES];
		ITexture* m_textures[MATERIAL_MAX_TEXTURES];
		ITexture* m_overrideTextures[CShader::ResourceCount];

		video::E_COMPARISON_FUNC m_zBuffer;
		bool m_zWriteEnable;
		bool m_backfaceCulling;
		bool m_frontfaceCulling;
		bool m_doubleSided;
		bool m_deferred;
		bool m_castShadow;
		bool m_manualInitMaterial;

		CShader* m_shader;

	public:
		CMaterial(const char* name, const char* shaderPath);

		virtual ~CMaterial();

		inline CShader* getShader()
		{
			return m_shader;
		}

		void rename(const char* name)
		{
			m_materialName = name;
		}

		inline const char* getName()
		{
			return m_materialName.c_str();
		}

		inline const char* getPackage()
		{
			return m_package.c_str();
		}

		inline void setPackage(const char* package)
		{
			m_package = package;
		}

		inline const char* getShaderPath()
		{
			return m_shaderPath.c_str();
		}

		inline const char* getMaterialPath()
		{
			return m_materialPath.c_str();
		}

		inline void setMaterialPath(const char* path)
		{
			m_materialPath = path;
		}

		inline bool isDeferred()
		{
			return m_deferred;
		}

		inline void setManualInitTexture(bool b)
		{
			m_manualInitMaterial = b;
		}

		CMaterial* clone();
		void copyTo(CMaterial* mat);
		void copyParamsTo(CMaterial* mat);

		void deleteAllParams();
		void deleteExtraParams();

		void setUniform(const char* name, float f);
		void setUniform2(const char* name, float* f);
		void setUniform3(const char* name, float* f);
		void setUniform4(const char* name, float* f);
		void setUniform4(const char* name, const SColor& color);

		const char* getUniformTextureName(int slot);
		void setUniformTexture(const char* name, const char* path, bool loadTexture = true);
		void setUniformTexture(const char* name, const char* path, std::vector<std::string>& folder, bool loadTexture = true);
		void setUniformTexture(const char* name, ITexture* texture);
		void setUniformTextureWrapU(const char* name, int wrapU);
		void setUniformTextureWrapV(const char* name, int wrapV);

		SExtraParams* newExtra(const char* shaderPath);
		void setExtraUniformTexture(SExtraParams* e, const char* name, const char* path);
		void setExtraUniform(SExtraParams* e, const char* name, float* f, int floatSize);
		void setExtraUniformTextureWrapU(SExtraParams* e, const char* name, int wrapU);
		void setExtraUniformTextureWrapV(SExtraParams* e, const char* name, int wrapV);

		inline std::vector<SUniformValue*>& getUniformParams()
		{
			return m_uniformParams;
		}

		inline std::vector<SUniformTexture*>& getUniformTexture()
		{
			return m_uniformTextures;
		}

		inline std::vector<SExtraParams*>& getExtraParams()
		{
			return m_extras;
		}

		inline void setBackfaceCulling(bool b)
		{
			m_backfaceCulling = b;
		}

		inline bool isBackfaceCulling()
		{
			return m_backfaceCulling;
		}

		inline void setFrontfaceCulling(bool b)
		{
			m_frontfaceCulling = b;
		}

		inline bool isFrontfaceCulling()
		{
			return m_frontfaceCulling;
		}

		inline void setZWrite(bool b)
		{
			m_zWriteEnable = b;
		}

		inline bool isZWrite()
		{
			return m_zWriteEnable;
		}

		inline void setZTest(video::E_COMPARISON_FUNC f)
		{
			m_zBuffer = f;
		}

		inline video::E_COMPARISON_FUNC getZTest()
		{
			return m_zBuffer;
		}

		SUniformValue* getUniform(const char* name);

		SUniformTexture* getUniformTexture(const char* name);

		SUniformValue* getExtraUniform(SExtraParams* e, const char* name);

		SUniformTexture* getExtraUniformTexture(SExtraParams* e, const char* name);

		inline CShaderParams& getShaderParams()
		{
			return m_shaderParams;
		}

		bool haveUniform(const char* name);

		void initMaterial();

		void setTexture(ITexture** textures, int num);

		void setTexture(int slot, ITexture* texture);

		ITexture* getTexture(int slot);

		void setProperty(const std::string& name, const std::string& value);

		std::string getProperty(const std::string& name);

		void loadDefaultTexture();

		void loadUniformTexture();

		void unloadDefaultTexture();

		void unloadUniformTexture();

		void unloadUniformTexture(const char* name);

		void setOverrideResource(ITexture* texture, CShader::EResourceType type)
		{
			m_overrideTextures[type] = texture;
		}

	public:

		void changeShader(CShader* shader);

		void changeShader(const char* path);

		bool autoDetectLoadTexture();

		void applyMaterial();

		void applyMaterial(SMaterial& mat);

		void updateTexture(SMaterial& mat);

		void replaceTexture(ITexture* oldTexture, ITexture* newTexture);

		void updateShaderParams();

	protected:

		void bindUniformParam();

		void updateSetTextureSlot();

		void initDefaultValue();

		void setDefaultValue(SUniformValue* v, SUniform* u);

		SUniformValue* newUniform(const char* name, int floatSize);

		SUniformTexture* newUniformTexture(const char* name);

		SUniformTexture* newExtraUniformTexture(SExtraParams* e, const char* name);

		SUniformValue* newExtraUniform(SExtraParams* e, const char* name, int floatSize);

		void addShaderUI(CShader::SUniformUI* ui);

		SExtraParams* getExtraParams(const char* shaderPath);

		void saveExtraParams();

		void reloadExtraParams(const char* shaderPath);

		SUniformTexture* findExtraTexture(const char* name);

		SUniformValue* findExtraParam(const char* name, int floatSize);

	protected:


	};

	typedef std::vector<CMaterial*> ArrayMaterial;
}