/*
!@
MIT License

Copyright (c) 2022 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the Rights to use, copy, modify,
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
#include "CEntityHandler.h"
#include "CEntityHandleData.h"
#include "CEntityChildsData.h"
#include "CEntityManager.h"
#include "GameObject/CGameObject.h"
#include "GameObject/CZone.h"
#include "Transform/CWorldTransformData.h"
#include "Culling/CVisibleData.h"
#include "Utils/CRandomID.h"

namespace Skylicht
{
	CEntityHandler::CEntityHandler() :
		m_shadowCasting(true)
	{

	}

	CEntityHandler::~CEntityHandler()
	{
		removeAllEntities();
	}

	void CEntityHandler::initComponent()
	{

	}

	void CEntityHandler::updateComponent()
	{

	}

	void CEntityHandler::onUpdateCullingLayer(u32 mask)
	{
		for (u32 i = 0, n = m_entities.size(); i < n; i++)
		{
			CVisibleData* visible = GET_ENTITY_DATA(m_entities[i], CVisibleData);
			visible->CullingLayer = mask;
		}
	}

	CEntity* CEntityHandler::searchEntityByID(const char* id)
	{
		for (u32 i = 0, n = m_entities.size(); i < n; i++)
		{
			if (m_entities[i]->getID() == id)
				return m_entities[i];
		}
		return NULL;
	}

	CEntity* CEntityHandler::createEntity()
	{
		CEntityManager* entityManager = m_gameObject->getEntityManager();

		CEntity* entity = entityManager->createEntity();
		CWorldTransformData* transformData = entity->addData<CWorldTransformData>();

		int namePosition = (int)m_entities.size();

		// name
		char name[512];
		sprintf(name, "#%d", namePosition);

		// id
		std::string id = CRandomID::generate();
		entity->setID(id.c_str());

		// add parent relative
		CEntity* parent = m_gameObject->getEntity();
		if (parent != NULL)
		{
			transformData->Name = name;
			transformData->ParentIndex = parent->getIndex();
		}

		// add handle data
		CEntityHandleData* handleData = entity->addData<CEntityHandleData>();
		handleData->Handler = this;

		m_entities.push_back(entity);
		return entity;
	}

	CEntity* CEntityHandler::createEntity(CEntity* parent)
	{
		CEntityManager* entityManager = m_gameObject->getEntityManager();

		CEntity* entity = entityManager->createEntity();
		CWorldTransformData* transformData = entity->addData<CWorldTransformData>();

		char name[512];
		sprintf(name, "#%d", entity->getIndex());

		// id
		std::string id = CRandomID::generate();
		entity->setID(id.c_str());

		// add parent relative
		if (parent != NULL)
		{
			transformData->Name = name;
			transformData->ParentIndex = parent->getIndex();

			CEntityChildsData* childs = GET_ENTITY_DATA(parent, CEntityChildsData);
			if (childs == NULL)
				childs = parent->addData<CEntityChildsData>();
			childs->Childs.push_back(entity);
		}

		if (parent == NULL)
			m_entities.push_back(entity);

		return entity;
	}

	void CEntityHandler::removeEntity(CEntity* entity)
	{
		CEntityManager* entityManager = m_gameObject->getEntityManager();

		for (int i = (int)m_entities.size() - 1; i >= 0; i--)
		{
			if (m_entities[i] == entity)
			{
				if (entity->isAlive())
				{
					removeChilds(entity);
					entityManager->removeEntity(entity);
				}
				m_entities.erase(i);
			}
		}
	}

	void CEntityHandler::regenerateEntityId()
	{
		for (int i = (int)m_entities.size() - 1; i >= 0; i--)
		{
			CEntity* entity = m_entities[i];
			if (entity->isAlive())
			{
				std::string id = CRandomID::generate();
				entity->setID(id.c_str());
			}
		}
	}

	void CEntityHandler::removeAllEntities()
	{
		if (m_gameObject == NULL)
			return;

		CEntityManager* entityManager = m_gameObject->getEntityManager();
		if (entityManager == NULL)
			return;

		for (int i = (int)m_entities.size() - 1; i >= 0; i--)
		{
			CEntity* entity = m_entities[i];
			if (entity->isAlive())
			{
				removeChilds(entity);
				entityManager->removeEntity(entity);
			}
		}

		m_entities.clear();
	}

	void CEntityHandler::removeChilds(CEntity* entity)
	{
		CEntityManager* entityManager = m_gameObject->getEntityManager();

		CEntityChildsData* childs = NULL;
		CWorldTransformData* transformData = entity->getData<CWorldTransformData>();
		if (transformData->ParentIndex >= 0)
		{
			CEntity* parent = entityManager->getEntity(transformData->ParentIndex);
			childs = GET_ENTITY_DATA(parent, CEntityChildsData);
			if (childs)
			{
				int childCount = (int)childs->Childs.size();
				for (int i = childCount - 1; i >= 0; i--)
				{
					if (childs->Childs[i] == entity)
					{
						childs->Childs.erase(i);
						break;
					}
				}
			}
		}

		childs = GET_ENTITY_DATA(entity, CEntityChildsData);
		if (childs)
		{
			int childCount = (int)childs->Childs.size();
			for (int i = childCount - 1; i >= 0; i--)
			{
				CEntity* c = childs->Childs[i];

				removeChilds(c);
				entityManager->removeEntity(c);
			}
			childs->Childs.clear();
		}
	}

	void CEntityHandler::setEntities(CEntity** entities, u32 count)
	{
		m_entities.clear();
		for (u32 i = 0; i < count; i++)
		{
			CEntity* entity = entities[i];
			CWorldTransformData* transformData = GET_ENTITY_DATA(entity, CWorldTransformData);

			// assign name
			if (transformData->Name.empty())
			{
				char name[512];
				sprintf(name, "#%d", entity->getIndex());
				transformData->Name = name;
			}

			// generate id for entity
			std::string id = CRandomID::generate();
			entity->setID(id.c_str());

			// add to handler
			m_entities.push_back(entity);
		}
	}

	void CEntityHandler::getEntitiesTransforms(core::array<core::matrix4>& result)
	{
		u32 n = m_entities.size();
		result.set_used(n);

		for (u32 i = 0; i < n; i++)
		{
			CEntity* entity = m_entities[i];

			CWorldTransformData* transformData = GET_ENTITY_DATA(entity, CWorldTransformData);
			result[i] = transformData->Relative;
		}
	}

	void CEntityHandler::setShadowCasting(bool b)
	{
		u32 n = m_entities.size();

		for (u32 i = 0; i < n; i++)
		{
			CEntity* entity = m_entities[i];

			CVisibleData* visible = GET_ENTITY_DATA(entity, CVisibleData);
			visible->ShadowCasting = b;
		}

		m_shadowCasting = b;
	}
}
