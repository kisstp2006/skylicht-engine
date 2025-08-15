/*
!@
MIT License

Copyright (c) 2022 Skylicht Technology CO., LTD

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

#include "pch.h"
#include "CPrimitive.h"
#include "CPrimitiveRenderer.h"
#include "CPrimitiveRendererInstancing.h"
#include "CPrimitiveSerializable.h"
#include "GameObject/CGameObject.h"
#include "Entity/CEntityManager.h"
#include "Transform/CWorldTransformData.h"
#include "Transform/CWorldInverseTransformData.h"
#include "Culling/CCullingData.h"
#include "Culling/CCullingBBoxData.h"
#include "IndirectLighting/CIndirectLightingData.h"
#include "Material/CMaterialManager.h"

namespace Skylicht
{
	CPrimitive::CPrimitive() :
		m_type(CPrimiviteData::Unknown),
		m_instancing(false),
		m_useCustomMaterial(false),
		m_useNormalMap(false),
		m_color(255, 150, 150, 150),
		m_material(NULL),
		m_customMaterial(NULL)
	{

	}

	CPrimitive::~CPrimitive()
	{
		if (m_material)
			m_material->drop();
	}

	void CPrimitive::initComponent()
	{
		CEntityManager* entityMgr = m_gameObject->getEntityManager();
		entityMgr->addRenderSystem<CPrimitiveRenderer>();
		entityMgr->addRenderSystem<CPrimitiveRendererInstancing>();

		// init material
		m_material = new CMaterial("Primitive", "BuiltIn/Shader/SpecularGlossiness/Deferred/Color.xml");
		m_material->setUniform4("uColor", m_color);
		m_material->updateShaderParams();

		// add default primitive
		if (m_type != CPrimiviteData::Unknown)
		{
			addPrimitive(
				core::vector3df(),
				core::vector3df(),
				core::vector3df(1.0f, 1.0f, 1.0f)
			);
		}
	}

	CObjectSerializable* CPrimitive::createSerializable()
	{
		CObjectSerializable* object = CComponentSystem::createSerializable();

		object->autoRelease(new CBoolProperty(object, "instancing", m_instancing));

		object->autoRelease(new CBoolProperty(object, "shadow casting", m_shadowCasting));

		object->autoRelease(new CBoolProperty(object, "normal map", m_useNormalMap));

		CColorProperty* color = new CColorProperty(object, "color", m_color);
		color->setUIHeader("Default Material");
		object->autoRelease(color);

		CFilePathProperty* material = new CFilePathProperty(object, "material", m_materialPath.c_str(), CMaterialManager::getMaterialExts());
		material->setUIHeader("Custom Material");
		object->autoRelease(material);

		object->autoRelease(new CBoolProperty(object, "custom material", m_useCustomMaterial));

		CArraySerializable* primitives = new CArraySerializable("Primitives");
		object->addProperty(primitives);
		object->autoRelease(primitives);

		int numPrimities = (int)m_entities.size();
		for (int i = 0; i < numPrimities; i++)
		{
			CPrimitiveSerializable* p = new CPrimitiveSerializable(primitives);
			primitives->autoRelease(p);

			// id
			p->Id.set(m_entities[i]->getID());

			// transform
			CWorldTransformData* world = GET_ENTITY_DATA(m_entities[i], CWorldTransformData);
			p->Transform.set(world->Relative);
		}

		return object;
	}

	void CPrimitive::loadSerializable(CObjectSerializable* object)
	{
		CComponentSystem::loadSerializable(object);

		bool forceUpdateMaterial = false;
		bool useMaterial = !m_materialPath.empty();
		bool useCustom = m_useCustomMaterial;

		m_instancing = object->get<bool>("instancing", false);
		m_shadowCasting = object->get<bool>("shadow casting", true);
		m_useCustomMaterial = object->get<bool>("custom material", false);
		m_useNormalMap = object->get<bool>("normal map", false);
		m_color = object->get<SColor>("color", SColor(255, 180, 180, 180));
		m_materialPath = object->get<std::string>("material", std::string());

		if (!m_materialPath.empty() && !useMaterial)
		{
			// force enable material while update new material in editor
			m_useCustomMaterial = true;
			forceUpdateMaterial = true;
		}

		if (forceUpdateMaterial || m_useCustomMaterial != useCustom)
		{
			if (m_useCustomMaterial == true && !m_materialPath.empty())
			{
				m_customMaterial = NULL;

				ArrayMaterial& loadMaterials = CMaterialManager::getInstance()->loadMaterial(m_materialPath.c_str(), true, std::vector<std::string>());
				if (loadMaterials.size() > 0)
					m_customMaterial = loadMaterials[0];
			}
		}

		m_material->setUniform4("uColor", m_color);
		m_material->updateShaderParams();

		if (m_customMaterial)
		{
			m_customMaterial->setUniform4("uColor", m_color);
			m_customMaterial->updateShaderParams();
		}

		CArraySerializable* primitives = (CArraySerializable*)object->getProperty("Primitives");
		if (primitives == NULL)
			return;

		int numPrimities = primitives->getElementCount();

		removeAllEntities();

		for (int i = 0; i < numPrimities; i++)
		{
			CPrimitiveSerializable* primitiveData = dynamic_cast<CPrimitiveSerializable*>(primitives->getElement(i));
			if (primitiveData)
			{
				CEntity* entity = addPrimitive(
					core::vector3df(),
					core::vector3df(),
					core::vector3df(1.0f, 1.0f, 1.0f)
				);

				// set id
				if (!primitiveData->Id.get().empty())
					entity->setID(primitiveData->Id.getString());

				// set transform
				CWorldTransformData* world = GET_ENTITY_DATA(entity, CWorldTransformData);
				world->Relative = primitiveData->Transform.get();
			}
			else
			{
				// fix for old version
				CMatrixProperty* worldMatrix = dynamic_cast<CMatrixProperty*>(primitives->getElement(i));
				if (worldMatrix)
				{
					CEntity* entity = addPrimitive(
						core::vector3df(),
						core::vector3df(),
						core::vector3df(1.0f, 1.0f, 1.0f)
					);

					// set transform
					CWorldTransformData* world = GET_ENTITY_DATA(entity, CWorldTransformData);
					world->Relative = worldMatrix->get();
				}
			}
		}

		setShadowCasting(m_shadowCasting);
	}

	CEntity* CPrimitive::spawn()
	{
		return addPrimitive(core::vector3df(), core::vector3df(), core::vector3df(1.0f, 1.0f, 1.0f));
	}

	CEntity* CPrimitive::addPrimitive(const core::vector3df& pos, const core::vector3df& rotDeg, const core::vector3df& scale)
	{
		CEntity* entity = createEntity();

		CPrimiviteData* primitiveData = entity->addData<CPrimiviteData>();
		primitiveData->Type = m_type;
		primitiveData->Material = m_useCustomMaterial && m_customMaterial ? m_customMaterial : m_material;
		primitiveData->Instancing = m_instancing;
		primitiveData->NormalMap = m_useNormalMap;
		primitiveData->RootEntity = m_gameObject->getEntity()->getIndex();

		// Lighting
		entity->addData<CIndirectLightingData>();

		// Culling
		entity->addData<CWorldInverseTransformData>();
		entity->addData<CCullingData>();

		CCullingBBoxData* cullingBBox = entity->addData<CCullingBBoxData>();

		if (m_type == CPrimiviteData::Plane)
		{
			cullingBBox->BBox.MinEdge.set(-0.5f, -0.01f, -0.5f);
			cullingBBox->BBox.MaxEdge.set(0.5f, 0.01f, 0.5f);
		}
		else
		{
			cullingBBox->BBox.MinEdge.set(-0.5f, -0.5f, -0.5f);
			cullingBBox->BBox.MaxEdge.set(0.5f, 0.5f, 0.5f);
		}

		// Position
		CWorldTransformData* transform = GET_ENTITY_DATA(entity, CWorldTransformData);
		transform->Relative.setTranslation(pos);
		transform->Relative.setRotationDegrees(rotDeg);
		transform->Relative.setScale(scale);

		// Indirect lighting
		CIndirectLightingData* indirect = GET_ENTITY_DATA(entity, CIndirectLightingData);
		indirect->initSH();
		return entity;
	}

	CMaterial* CPrimitive::getMaterial()
	{
		if (m_useCustomMaterial && m_customMaterial)
			return m_customMaterial;

		return m_material;
	}

	CMesh* CPrimitive::getMesh()
	{
		CEntityManager* entityMgr = m_gameObject->getEntityManager();

		CPrimitiveRenderer* renderer = entityMgr->getSystem<CPrimitiveRenderer>();
		if (renderer)
			return renderer->getMesh(m_type);

		return NULL;
	}

	void CPrimitive::setCustomMaterial(CMaterial* material)
	{
		if (material)
		{
			m_useCustomMaterial = true;
			m_customMaterial = material;
		}
		else
		{
			m_useCustomMaterial = false;
		}

		for (u32 i = 0, n = m_entities.size(); i < n; i++)
		{
			CPrimiviteData* data = GET_ENTITY_DATA(m_entities[i], CPrimiviteData);
			if (data)
			{
				data->Material = m_useCustomMaterial && m_customMaterial ? m_customMaterial : m_material;
				data->InstancingMesh = NULL;
				data->InstancingGroup = NULL;
			}
		}
	}

	void CPrimitive::setInstancing(bool b)
	{
		m_instancing = b;
		for (u32 i = 0, n = m_entities.size(); i < n; i++)
		{
			CPrimiviteData* data = GET_ENTITY_DATA(m_entities[i], CPrimiviteData);
			if (data)
			{
				data->Instancing = b;
				data->InstancingMesh = NULL;
				data->InstancingGroup = NULL;
			}
		}
	}

	void CPrimitive::setColor(const SColor& color)
	{
		m_color = color;

		m_material->setUniform4("uColor", m_color);
		m_material->updateShaderParams();
	}
}