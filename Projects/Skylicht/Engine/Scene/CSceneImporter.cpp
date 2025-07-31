/*
!@
MIT License

Copyright (c) 2021 Skylicht Technology CO., LTD

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
#include "CSceneImporter.h"
#include "CSceneExporter.h"
#include "Utils/CStringImp.h"
#include "Components/CDependentComponent.h"

namespace Skylicht
{
	io::IXMLReader* g_sceneReader = NULL;
	std::string g_sceneReaderPath;
	CScene* g_scene = NULL;
	bool g_generateId = false;

	int g_loadSceneStep = 10;
	int g_loadingScene = 0;

	std::list<CGameObject*> g_listGameObject;
	std::list<CGameObject*>::iterator g_currentGameObject;

	CObjectSerializable* g_template = NULL;

	void CSceneImporter::buildComponent(CGameObject* object, io::IXMLReader* reader)
	{
		std::wstring nodeName = L"node";
		std::wstring attributeName;

		int tree = 1;
		bool done = false;

		while (!done && reader->read())
		{
			switch (reader->getNodeType())
			{
			case io::EXN_ELEMENT:
				if (nodeName == reader->getNodeName())
				{
					tree++;

					// only read node of Component child
					if (reader->getAttributeValue(L"type") != NULL && tree == 2)
					{
						attributeName = reader->getAttributeValue(L"type");
						std::string componentName = CStringImp::convertUnicodeToUTF8(attributeName.c_str());

						CComponentSystem* comSystem = object->getComponentByTypeName(componentName.c_str());
						if (comSystem == NULL)
						{
							// try add component
							if (object->addComponentByTypeName(componentName.c_str()) == NULL)
							{
								char log[512];
								sprintf(log, "[CSceneImporter] Found unsupport component '%s'", componentName.c_str());
								os::Printer::log(log);

								// unsupport component
								CNullComponent* nullComponent = object->addComponent<CNullComponent>();
								nullComponent->setName(componentName);
							}
						}
					}
				}
				break;
			case io::EXN_ELEMENT_END:
				if (nodeName == reader->getNodeName())
				{
					tree--;
					done = tree == 0;
				}
				break;
			default:
				break;
			}
		}
	}

	void CSceneImporter::buildScene(CScene* scene, CContainerObject* target, io::IXMLReader* reader)
	{
		std::wstring nodeName = L"node";
		std::wstring attributeName;

		std::stack<std::wstring> serializableTree;

		std::stack<CContainerObject*> container;
		if (target)
			container.push(target);

		g_listGameObject.clear();

		CGameObject* currentObject = NULL;

		while (reader->read())
		{
			switch (reader->getNodeType())
			{
			case io::EXN_ELEMENT:
				if (nodeName == reader->getNodeName())
				{
					if (reader->getAttributeValue(L"type") != NULL)
					{
						attributeName = reader->getAttributeValue(L"type");

						bool skipPush = false;

						if (attributeName == L"CZone")
						{
							CZone* zone = scene->createZone();
							container.push((CContainerObject*)zone);
							g_listGameObject.push_back(zone);
							currentObject = zone;
						}
						else if (attributeName == L"CContainerObject")
						{
							CContainerObject* currentContainer = container.top();
							CContainerObject* object = currentContainer->createContainerObject();
							container.push(object);
							g_listGameObject.push_back(object);
							currentObject = object;
						}
						else if (attributeName == L"CGameObject")
						{
							CContainerObject* currentContainer = container.top();
							CGameObject* object = currentContainer->createEmptyObject();
							g_listGameObject.push_back(object);
							currentObject = object;
						}
						else if (attributeName == L"Components")
						{
							if (currentObject != NULL)
							{
								buildComponent(currentObject, reader);
								continue;
							}
						}

						serializableTree.push(attributeName);
					}
					else
					{
						serializableTree.push(L"");
					}
				}
				break;
			case io::EXN_ELEMENT_END:
				if (nodeName == reader->getNodeName())
				{
					std::wstring type = serializableTree.top();
					if (type == L"CContainerObject" || type == L"CZone")
					{
						container.pop();
					}
					serializableTree.pop();
				}
				break;
			default:
				break;
			}
		}

		g_currentGameObject = g_listGameObject.begin();
	}

	bool CSceneImporter::beginImportScene(CScene* scene, const char* file)
	{
		// step 1
		// build scene object
		g_sceneReader = getIrrlichtDevice()->getFileSystem()->createXMLReader(file);
		if (g_sceneReader == NULL)
			return false;

		buildScene(scene, NULL, g_sceneReader);

		// close
		g_sceneReader->drop();

		// re-open the scene file
		g_sceneReader = getIrrlichtDevice()->getFileSystem()->createXMLReader(file);
		g_sceneReaderPath = file;
		g_scene = scene;
		g_loadingScene = 0;
		g_generateId = false;

		return true;
	}

	bool CSceneImporter::loadStep(CScene* scene, io::IXMLReader* reader)
	{
		std::wstring nodeName = L"node";
		std::wstring attributeName;

		int step = 0;

		while (step < g_loadSceneStep && reader->read())
		{
			switch (reader->getNodeType())
			{
			case io::EXN_ELEMENT:
				if (nodeName == reader->getNodeName())
				{
					if (reader->getAttributeValue(L"type") != NULL)
					{
						attributeName = reader->getAttributeValue(L"type");

						if (attributeName == L"CScene")
						{
							CObjectSerializable* data = scene->createSerializable();
							data->parseSerializable(reader);
							scene->loadSerializable(data);
							delete data;
						}
						else if (attributeName == L"CZone" ||
							attributeName == L"CContainerObject" ||
							attributeName == L"CGameObject")
						{
							CGameObject* gameobject = dynamic_cast<CGameObject*>(*g_currentGameObject);

							// get id generated
							std::string id = gameobject->getID();

							++g_currentGameObject;
							++g_loadingScene;
							++step;

							std::string name = CStringImp::convertUnicodeToUTF8(attributeName.c_str());
							CObjectSerializable* data = new CObjectSerializable(name.c_str());

							// load to end of node Components
							CSerializableLoader::load(reader, data, "Components");
							gameobject->loadSerializable(data);
							gameobject->startComponent();
							delete data;

							// use new id, that generated
							if (g_generateId)
								gameobject->setID(id.c_str());
						}
					}
				}
				break;
			default:
				break;
			}
		}

		return g_currentGameObject == g_listGameObject.end();
	}

	float CSceneImporter::getLoadingPercent()
	{
		int size = (int)g_listGameObject.size();
		return g_loadingScene / (float)size;
	}

	int CSceneImporter::getTotalObjects()
	{
		return (int)g_listGameObject.size();
	}

	bool CSceneImporter::updateLoadScene()
	{
		// step 2
		// load object attribute
		if (CSceneImporter::loadStep(g_scene, g_sceneReader))
		{
			// drop
			if (g_sceneReader)
			{
				g_sceneReader->drop();
				g_sceneReader = NULL;
			}

			// final index search object
			g_scene->updateIndexSearchObject();
			g_scene = NULL;
			return true;
		}

		return false;
	}

	CGameObject* CSceneImporter::importTemplate(CContainerObject* target, const char* path)
	{
		if (!beginImportTemplate(target, path))
			return NULL;

		if (g_listGameObject.size() == 0)
			return NULL;

		// re-open the scene file
		g_sceneReader = getIrrlichtDevice()->getFileSystem()->createXMLReader(path);
		g_sceneReaderPath = path;
		g_scene = target->getScene();
		g_loadingScene = 0;
		g_generateId = true;

		// import object
		while (updateLoadScene() == false);

		return g_listGameObject.front();
	}

	bool CSceneImporter::beginImportTemplate(CContainerObject* target, const char* path)
	{
		io::IXMLReader* xmlReader = getIrrlichtDevice()->getFileSystem()->createXMLReader(path);
		if (xmlReader == NULL)
			return false;

		buildScene(target->getScene(), target, xmlReader);

		xmlReader->drop();

		g_sceneReaderPath = path;
		g_scene = target->getScene();
		g_loadingScene = 0;
		g_generateId = true;

		return true;
	}

	CObjectSerializable* CSceneImporter::importTemplateToObject(CZone* zone, const char* path)
	{
		CGameObject* obj = importTemplate(zone, path);
		if (obj == NULL)
			return NULL;

		CContainerObject* container = dynamic_cast<CContainerObject*>(obj);
		if (container)
			container->updateAddRemoveObject();

		CObjectSerializable* data = CSceneExporter::exportGameObject(obj);
		obj->remove();
		return data;
	}

	void CSceneImporter::reloadTemplate(CGameObject* obj, CObjectSerializable* templateData)
	{
		if (obj->getTypeName() != templateData->Name)
			return;

		// save the transform
		core::matrix4 saveTransform = obj->getTransform()->getRelativeTransform();

		std::vector<std::string> componentOrder;
		CDependentComponent* depComp = CDependentComponent::getInstance();

		CObjectSerializable* components = templateData->getProperty<CObjectSerializable>("Components");
		if (components)
		{
			// remove the components, that is not exist in template
			std::vector<CComponentSystem*> remove;
			for (int i = 0, n = obj->getComponentCount(); i < n; i++)
			{
				CComponentSystem* comp = obj->getComponentByPos(i);
				CObjectSerializable* compProperty = components->getProperty<CObjectSerializable>(comp->getTypeName().c_str());
				if (compProperty == NULL)
				{
					if (!depComp->isDependentComponent(comp->getTypeName().c_str()))
						remove.push_back(comp);
				}
			}

			while (remove.size() > 0)
			{
				obj->removeComponent(remove[remove.size() - 1]);
				remove.erase(--remove.end());
			}

			// add new components, that is not exist in gameobject
			for (int i = 0, n = components->getNumProperty(); i < n; i++)
			{
				CValueProperty* p = components->getPropertyID(i);

				const char* componentName = p->Name.c_str();
				componentOrder.push_back(componentName);

				if (p->getType() == Skylicht::Object)
				{
					if (obj->getComponentByTypeName(componentName) == NULL)
						obj->addComponentByTypeName(componentName);
				}

			}
		}
		else
		{
			obj->releaseAllComponent();
		}

		// sync data
		obj->sortComponent(componentOrder);

		// get id generated
		std::string id = obj->getID();

		// load data
		obj->loadSerializable(templateData);
		obj->startComponent();

		// apply current transform for root template
		if (obj == obj->getParentTemplate())
			obj->getTransform()->setRelativeTransform(saveTransform);

		// revert id
		obj->setID(id.c_str());
		obj->setTemplateChanged(false);

		// sync in childs object
		CContainerObject* container = dynamic_cast<CContainerObject*>(obj);
		if (container)
		{
			std::vector<CGameObject*> syncObjects;
			std::vector<std::string> objectOrder;

			// reload template data in childs
			CObjectSerializable* childs = templateData->getProperty<CObjectSerializable>("Childs");
			if (childs)
			{
				for (int i = 0, n = childs->getNumProperty(); i < n; i++)
				{
					CObjectSerializable* child = dynamic_cast<CObjectSerializable*>(childs->getPropertyID(i));

					std::string templateObjId = child->get("templateObjectId", std::string(""));
					if (!templateObjId.empty())
					{
						objectOrder.push_back(templateObjId);

						CGameObject* obj = container->searchObjectInChildByTemplateObjId(templateObjId.c_str());
						if (obj)
						{
							// don't reload child template asset
							if (!obj->isTemplateAsset())
							{
								reloadTemplate(obj, child);
							}

							syncObjects.push_back(obj);
						}
						else
						{
							// create new game object
							container->createObject(child, true);
						}
					}
				}
			}

			// remove the gameobject, that is not exist in template
			ArrayGameObject* childObjs = container->getChilds();
			for (CGameObject* obj : *childObjs)
			{
				bool remove = true;

				for (CGameObject* i : syncObjects)
				{
					if (obj == i)
					{
						remove = false;
						break;
					}
				}

				if (remove)
				{
					obj->remove();
				}
			}

			container->updateAddRemoveObject();

			if (objectOrder.size() > 0)
				container->sortChildsByTemplateOrder(objectOrder);
		}
	}

	int CSceneImporter::beginReloadTemplate(CScene* scene, CObjectSerializable* templateData)
	{
		std::string templateId = templateData->get("templateId", std::string(""));
		return beginReplaceTemplate(scene, templateId.c_str(), templateData);
	}

	int CSceneImporter::beginReplaceTemplate(CScene* scene, const char* oldTemplateId, CObjectSerializable* templateData)
	{
		std::string templateId = oldTemplateId;

		std::stack<CContainerObject*> stack;

		for (int i = 0, n = scene->getZoneCount(); i < n; i++)
			stack.push(scene->getZone(i));

		g_listGameObject.clear();

		while (!stack.empty())
		{
			CContainerObject* container = stack.top();
			stack.pop();

			ArrayGameObject* childs = container->getChilds();
			for (CGameObject* child : *childs)
			{
				if (child->isTemplateAsset() && templateId == child->getTemplateID())
				{
					g_listGameObject.push_back(child);
				}

				CContainerObject* childContainer = dynamic_cast<CContainerObject*>(child);
				if (childContainer)
					stack.push(childContainer);
			}
		}

		g_template = templateData;
		g_loadingScene = 0;
		g_currentGameObject = g_listGameObject.begin();

		return (int)g_listGameObject.size();
	}

	bool CSceneImporter::reloadTemplate()
	{
		if (g_listGameObject.size() == 0 || g_template == NULL)
			return true;

		reloadTemplate(*g_currentGameObject, g_template);

		++g_currentGameObject;
		++g_loadingScene;

		if (g_currentGameObject == g_listGameObject.end())
		{
			g_template = NULL;
			return true;
		}

		return false;
	}

	std::list<CGameObject*>& CSceneImporter::getObjects()
	{
		return g_listGameObject;
	}
}