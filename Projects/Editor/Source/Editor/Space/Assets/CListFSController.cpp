/*
!@
MIT License

Copyright (c) 2020 Skylicht Technology CO., LTD

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
#include "CListFSController.h"
#include "CTreeFSController.h"
#include "CSearchAssetController.h"
#include "Utils/CPath.h"
#include "Utils/CStringImp.h"
#include "GUI/Theme/ThemeConfig.h"
#include "Editor/SpaceController/CAssetPropertyController.h"
#include "Editor/SpaceController/CAssetCreateController.h"
#include "Editor/SpaceController/CSceneController.h"
#include "Editor/CEditor.h"

#include "Editor/Space/Hierarchy/CHierachyNode.h"

namespace Skylicht
{
	namespace Editor
	{
		CListFSController::CListFSController(GUI::CCanvas* canvas, CSpace* space, GUI::CListBase* list) :
			m_space(space),
			m_renameItem(NULL),
			m_treeController(NULL),
			m_canvas(canvas),
			m_msgBox(NULL),
			m_newFolderItem(NULL),
			m_searching(false),
			m_searchController(NULL),
			m_enableThumbnail(false)
		{
			m_assetManager = CAssetManager::getInstance();

			setListUI(list);

			std::vector<SFileInfo> files;
			m_assetManager->getRoot(files);
			add("", files, true);
		}

		CListFSController::~CListFSController()
		{

		}

		void CListFSController::setListUI(GUI::CListBase* list)
		{
			m_listFS = list;
			m_listFS->OnKeyPress = std::bind(
				&CListFSController::OnKeyPress,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3
			);

			m_listFS->OnSelected = BIND_LISTENER(&CListFSController::OnSelected, this);
			m_listFS->setMultiSelected(true);

			GUI::CBase* inner = m_listFS->getInnerPanel();
			inner->OnAcceptDragDrop = [](GUI::SDragDropPackage* data)
				{
					// accept hierarchy node
					if (data->Name == "HierarchyNode")
					{
						CHierachyNode* dragNode = (CHierachyNode*)data->UserData;
						if (dragNode->getTagDataType() == CHierachyNode::Container ||
							dragNode->getTagDataType() == CHierachyNode::GameObject)
						{
							return true;
						}
					}
					return false;
				};
			inner->OnDrop = [&](GUI::SDragDropPackage* data, float mouseX, float mouseY)
				{
					if (data->Name == "HierarchyNode")
					{
						CHierachyNode* dragNode = (CHierachyNode*)data->UserData;
						if (dragNode->getTagDataType() == CHierachyNode::Container ||
							dragNode->getTagDataType() == CHierachyNode::GameObject)
						{
							CGameObject* obj = (CGameObject*)dragNode->getTagData();
							CSceneController::getInstance()->onCreateTemplate(obj, m_currentFolder.c_str());
						}
					}
				};
		}

		void CListFSController::removePath(const char* path)
		{
			std::list<GUI::CButton*> items = m_listFS->getAllItems();
			for (GUI::CButton* item : items)
			{
				const std::string& tagPath = item->getTagString();
				if (tagPath == path)
				{
					item->remove();
					return;
				}
			}
		}

		GUI::CBase* CListFSController::scrollAndSelectPath(const char* path)
		{
			GUI::CButton* result = NULL;

			std::list<GUI::CButton*> items = m_listFS->getAllItems();
			for (GUI::CButton* item : items)
			{
				const std::string& tagPath = item->getTagString();
				if (tagPath == path)
				{
					item->setToggle(true);

					m_listFS->invalidate();
					m_listFS->recurseLayout();
					m_listFS->scrollToItem(item);

					result = item;
				}
				else
				{
					item->setToggle(false);
				}
			}

			return result;
		}

		void CListFSController::OnPress(GUI::CBase* item)
		{
			GUI::CListItemBase* rowItem = dynamic_cast<GUI::CListItemBase*>(item);
			if (rowItem == NULL)
				return;

			const std::string& fullPath = rowItem->getTagString();
			bool isFolder = rowItem->getTagBool();

			CAssetPropertyController::getInstance()->onSelectAsset(fullPath.c_str(), isFolder);
		}

		void CListFSController::OnSelected(GUI::CBase* item)
		{
			GUI::CListItemBase* rowItem = dynamic_cast<GUI::CListItemBase*>(item);
			if (rowItem == NULL)
				return;

			m_selectPath = rowItem->getTagString();

			if (m_searching)
			{
				bool isFolder = rowItem->getTagBool();
				std::string folder;

				if (isFolder == true)
				{
					if (m_treeController != NULL)
						m_treeController->expand(m_selectPath);

					folder = m_selectPath;
				}
				else
				{
					std::string folderPath = CPath::getFolderPath(m_selectPath);
					if (m_treeController != NULL)
						m_treeController->expand(folderPath);

					folder = folderPath;
				}

				m_selectSearchPath = m_selectPath;
				setCurrentFolder(folder.c_str());
			}
		}

		void CListFSController::OnKeyPress(GUI::CBase* control, int key, bool press)
		{
			GUI::CListBox* list = dynamic_cast<GUI::CListBox*>(control);
			if (list == NULL)
				return;

			if (key == GUI::KEY_F2)
			{
				rename(list->getSelected());
			}
		}

		void CListFSController::rename(GUI::CButton* node)
		{
			if (node != NULL)
			{
				m_renameItem = node;
				m_renameRevert = node->getLabel();

				GUI::CListItemBase* rowItem = dynamic_cast<GUI::CListItemBase*>(node);
				if (rowItem)
				{
					rowItem->getTextEditHelper()->beginEdit(
						BIND_LISTENER(&CListFSController::OnRename, this),
						BIND_LISTENER(&CListFSController::OnCancelRename, this)
					);
				}
			}
		}

		void CListFSController::OnRename(GUI::CBase* control)
		{
			GUI::CTextBox* textbox = dynamic_cast<GUI::CTextBox*>(control);

			std::wstring newNameW = textbox->getString();
			std::string newName = CStringImp::convertUnicodeToUTF8(newNameW.c_str());

			const std::string& path = m_renameItem->getTagString();
			std::string newPath = CPath::getFolderPath(path);
			newPath += "/";
			newPath += newName;

			if (m_assetManager->isExist(newPath.c_str()))
			{
				m_renameItem->setLabel(m_renameRevert);
				m_msgBox = new GUI::CMessageBox(m_canvas, GUI::CMessageBox::OK);
				m_msgBox->setMessage("File or folder with the new name already exists!", newName.c_str());
				m_msgBox->getMessageIcon()->setIcon(GUI::ESystemIcon::Alert);
				return;
			}

			// fix property window, that will save an old path asset
			CEditor::getInstance()->closeProperty();

			if (m_assetManager->renameAsset(path.c_str(), newName.c_str()))
			{
				m_renameItem = NULL;
				refresh();
				GUI::CBase* item = scrollAndSelectPath(newPath.c_str());
				if (item)
					OnPress(item);
			}
			else
			{
				refresh();
			}

			CEditor::getInstance()->refreshAssetSpace(m_space);

			m_listFS->focus();
		}

		void CListFSController::OnCancelRename(GUI::CBase* control)
		{
			m_renameItem = NULL;
			m_listFS->focus();
		}

		void CListFSController::add(const std::string& currentFolder, std::vector<SFileInfo>& files, bool scrollToBegin)
		{
			m_listFS->removeAllItem();
			clearThumbnail();

			if (currentFolder.size() > 0 &&
				currentFolder != m_assetManager->getAssetFolder())
			{
				GUI::CListItemBase* item = m_listFS->addItem(L"..", GUI::ESystemIcon::Folder);

				std::string parent = CPath::getFolderPath(currentFolder);
				item->tagString(parent);
				item->tagBool(true);
				item->setIconColor(GUI::ThemeConfig::FolderColor);
				item->OnDoubleLeftMouseClick = BIND_LISTENER(&CListFSController::OnFileOpen, this);
				item->OnPress = BIND_LISTENER(&CListFSController::OnPress, this);
			}

			for (SFileInfo& f : files)
			{
				GUI::CListItemBase* item;

				if (f.IsFolder)
				{
					item = m_listFS->addItem(f.NameW.c_str(), GUI::ESystemIcon::Folder);
					item->setIconColor(GUI::ThemeConfig::FolderColor);
				}
				else
				{
					ITexture* thumbnail = NULL;
					if (m_enableThumbnail)
					{
						std::string ext = CPath::getFileNameExt(f.Name);
						if (CTextureManager::isTextureExt(ext.c_str()))
							thumbnail = getFileThumbnail(f.FullPath);
					}

					if (thumbnail)
					{
						const core::dimension2du& size = thumbnail->getSize();
						GUI::SRect srcRect(0.0f, 0.0f, (float)size.Width, (float)size.Height);
						item = m_listFS->addItem(f.NameW.c_str(), thumbnail, srcRect);

						GUI::CThumbnailItem* thumbnailItem = dynamic_cast<GUI::CThumbnailItem*>(item);
						if (thumbnailItem)
						{
							GUI::CRawImage* rawImage = thumbnailItem->getImage();
							rawImage->enableRenderFillRect(true);
							rawImage->setFillRectColor(GUI::SGUIColor(255, 100, 100, 100));
						}
					}
					else
					{
						GUI::SGUIColor color;
						item = m_listFS->addItem(f.NameW.c_str(), getFileIcon(f.Name, color));
						item->setIconColor(color);
					}
				}

				item->tagString(f.FullPath);
				item->tagBool(f.IsFolder);
				item->OnDoubleLeftMouseClick = BIND_LISTENER(&CListFSController::OnFileOpen, this);
				item->OnPress = BIND_LISTENER(&CListFSController::OnPress, this);

				initDragDrop(item);
			}

			if (scrollToBegin)
				m_listFS->setScrollVertical(0.0f);

			std::string folder = currentFolder;
			if (folder.empty())
				folder = m_assetManager->getAssetFolder();

			setCurrentFolder(folder.c_str());
		}

		GUI::ESystemIcon CListFSController::getFileIcon(const std::string& name, GUI::SGUIColor& color)
		{
			std::string ext = CPath::getFileNameExt(name);

			color = GUI::SGUIColor(255, 255, 255, 255);

			if (ext == "template")
			{
				color = GUI::SGUIColor(255, 110, 170, 255);
				return GUI::ESystemIcon::ObjectBox;
			}
			else if (CMeshManager::isMeshExt(ext.c_str()))
			{
				return GUI::ESystemIcon::Res3D;
			}
			else if (ext == "mat")
			{
				return GUI::ESystemIcon::Material;
			}
			else if (CTextureManager::isTextureExt(ext.c_str()))
			{
				return GUI::ESystemIcon::ResImage;
			}

			return GUI::ESystemIcon::File;
		}

		ITexture* CListFSController::getFileThumbnail(const std::string& path)
		{
			return CAssetManager::getInstance()->getThumbnail()->getThumbnail(path.c_str());
		}

		void CListFSController::clearThumbnail()
		{
			CAssetManager::getInstance()->getThumbnail()->clearTextures();
		}

		void CListFSController::initDragDrop(GUI::CButton* item)
		{
			GUI::SDragDropPackage* dragDrop = item->setDragDropPackage("ListFSItem", item);
			dragDrop->DrawControl = item;

			item->OnAcceptDragDrop = [item](GUI::SDragDropPackage* data)
				{
					// accept hierarchy node
					if (data->Name == "HierarchyNode")
					{
						CHierachyNode* dragNode = (CHierachyNode*)data->UserData;
						if (dragNode->getTagDataType() == CHierachyNode::Container ||
							dragNode->getTagDataType() == CHierachyNode::GameObject)
						{
							return true;
						}
					}
					return false;
				};
			item->OnDrop = [&, item](GUI::SDragDropPackage* data, float mouseX, float mouseY)
				{
					if (data->Name == "HierarchyNode")
					{
						CHierachyNode* dragNode = (CHierachyNode*)data->UserData;
						if (dragNode->getTagDataType() == CHierachyNode::Container ||
							dragNode->getTagDataType() == CHierachyNode::GameObject)
						{
							CGameObject* obj = (CGameObject*)dragNode->getTagData();
							if (item->getTagBool() == true)
							{
								const std::string& folder = item->getTagString();
								CSceneController::getInstance()->onCreateTemplate(obj, folder.c_str());
							}
							else
							{
								CSceneController::getInstance()->onCreateTemplate(obj, m_currentFolder.c_str());
							}
						}
					}
				};
		}

		void CListFSController::OnFileOpen(GUI::CBase* node)
		{
			GUI::CListItemBase* rowNode = dynamic_cast<GUI::CListItemBase*>(node);
			if (rowNode != NULL)
			{
				CAssetCreateController::getInstance()->setActivateSpace(m_space);

				std::string fullPath = rowNode->getTagString();

				bool isFolder = rowNode->getTagBool();
				if (isFolder == true)
				{
					// browse folder
					std::vector<SFileInfo> files;
					m_assetManager->getFolder(fullPath.c_str(), files);

					add(fullPath, files, true);

					if (m_treeController != NULL)
						m_treeController->expand(fullPath);

					// close searching
					if (m_searching)
					{
						m_searchController->hideSearchUI();
						m_searching = false;
					}
				}
				else
				{
					// shell open the file
					std::string ext = CPath::getFileNameExt(fullPath);
					ext = CStringImp::toLower(ext);

					std::string shortPath = m_assetManager->getShortPath(fullPath.c_str());

					IFileLoader* fileLoader = m_assetManager->getFileLoader(ext.c_str());
					if (fileLoader != NULL)
						fileLoader->loadFile(shortPath.c_str());
#if WIN32
					else
					{
						char path[512] = { 0 };
						CStringImp::replaceText(path, fullPath.c_str(), "/", "\\");
						ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
					}
#endif
				}
			}
		}

		void CListFSController::browse(const char* folder)
		{
			setCurrentFolder(folder);
			refresh();
		}

		void CListFSController::setCurrentFolder(const char* folder)
		{
			m_currentFolder = folder;
			CAssetCreateController::getInstance()->setCurrentFolder(m_space, folder);
		}

		void CListFSController::refresh()
		{
			std::vector<SFileInfo> files;

			if (!m_currentFolder.empty() && !m_assetManager->isExist(m_currentFolder.c_str()))
			{
				// if this folder is deleted or isn't exist, we refesh to root
				m_currentFolder.clear();
			}

			if (m_currentFolder.empty())
				m_listFS->removeAllItem();
			else
				m_assetManager->getFolder(m_currentFolder.c_str(), files);

			add(m_currentFolder, files, false);

			m_listFS->invalidate();
			m_listFS->recurseLayout();

			if (!m_selectSearchPath.empty())
			{
				scrollAndSelectPath(m_selectSearchPath.c_str());
				m_selectSearchPath = "";
			}
		}

		void CListFSController::newFolder(const char* parent)
		{
			std::string baseName = "NewFolder";

			int id = 1;
			std::string name = baseName;
			std::string path = parent;
			path += "/";
			path += name;

			while (m_assetManager->isExist(path.c_str()))
			{
				++id;
				path = parent;
				path += "/";

				name = baseName;
				name += id;

				path += name;
			};

			m_newFolderItem = m_listFS->addItem(CStringImp::convertUTF8ToUnicode(name.c_str()).c_str(), GUI::ESystemIcon::Folder);
			m_newFolderItem->setIconColor(GUI::ThemeConfig::FolderColor);
			m_newFolderItem->tagString(parent);

			GUI::CButton* next = m_listFS->getItemByLabel(L"..");
			if (next != NULL)
				m_newFolderItem->bringNextToControl(next, true);
			else
				m_newFolderItem->sendToBack();

			m_newFolderItem->setToggle(true);

			m_listFS->recurseLayout();

			m_listFS->scrollToItem(m_newFolderItem);
			m_listFS->focus();

			m_newFolderItem->getTextEditHelper()->beginEdit(
				BIND_LISTENER(&CListFSController::OnRenameFolder, this),
				BIND_LISTENER(&CListFSController::OnCancelRenameFolder, this)
			);
		}

		void CListFSController::OnRenameFolder(GUI::CBase* control)
		{
			CEditor* editor = CEditor::getInstance();

			GUI::CTextBox* textbox = dynamic_cast<GUI::CTextBox*>(control);

			std::wstring newNameW = textbox->getString();
			std::string newName = CStringImp::convertUnicodeToUTF8(newNameW.c_str());

			const std::string& parent = m_newFolderItem->getTagString();
			std::string newPath = parent;
			newPath += "/";
			newPath += newName;

			if (m_assetManager->isExist(newPath.c_str()))
			{
				m_msgBox = new GUI::CMessageBox(m_canvas, GUI::CMessageBox::OK);
				m_msgBox->setMessage("Folder with the new name already exists!", newName.c_str());
				m_msgBox->getMessageIcon()->setIcon(GUI::ESystemIcon::Alert);
				m_msgBox->OnOK = [controller = this, parentPath = parent](GUI::CBase* base)
					{
						// Retry command new folder
						controller->newFolder(parentPath.c_str());
					};

				m_newFolderItem->remove();
				m_newFolderItem = NULL;

				editor->refreshAssetSpace(m_space);
				return;
			}

			// create new folder here
			if (m_assetManager->newFolderAsset(newPath.c_str()))
			{
				m_newFolderItem = NULL;

				refresh();
				scrollAndSelectPath(newPath.c_str());

				// focus on tree
				m_treeController->expand(newPath);
			}
			else
			{
				refresh();
			}

			m_listFS->focus();

			editor->refreshAssetSpace(m_space);
		}

		void CListFSController::OnCancelRenameFolder(GUI::CBase* control)
		{
			m_newFolderItem->remove();
			m_newFolderItem = NULL;
			m_listFS->focus();
		}
	}
}