/*
!@
MIT License

Copyright (c) 2023 Skylicht Technology CO., LTD

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

#pragma once

#include "Utils/CActivator.h"

#include "Reactive/CObserver.h"
#include "Reactive/CSubject.h"

#include "Serializable/CObjectSerializable.h"
#include "Serializable/CInterpolateSerializable.h"
#include "Editor/Space/Property/IPropertyEditor.h"

#include "GUI/GUI.h"

namespace Skylicht
{
	namespace Editor
	{
		class CSpaceProperty;

		class CSerializableEditor :
			public IPropertyEditor,
			public IActivatorObject
		{
		protected:
			std::vector<ISubject*> m_subjects;

		public:
			CSerializableEditor();

			virtual ~CSerializableEditor();

			virtual void closeGUI();

			virtual void update()
			{

			}

			virtual void initGUI(const char* path, CSpaceProperty* ui)
			{

			}

			virtual void serializableToControl(CObjectSerializable* object, CSpaceProperty* ui, GUI::CBoxLayout* layout);

			virtual void initCustomValueGUI(CObjectSerializable* object, CValueProperty* data, GUI::CBoxLayout* layout, CSpaceProperty* ui)
			{

			}

			virtual void onUpdateValue(CObjectSerializable* object)
			{

			}
		};
	}
}