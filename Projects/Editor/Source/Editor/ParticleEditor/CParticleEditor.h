/*
!@
MIT License

Copyright (c) 2021 Skylicht Technology CO., LTD

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
#include "Editor/SerializableEditor/CSerializableEditor.h"

#include "GUI/GUI.h"

#include "ParticleSystem/Particles/CParticleSerializable.h"
#include "ParticleSystem/Particles/CModel.h"

namespace Skylicht
{
	namespace Editor
	{
		class CSpaceProperty;

		class CParticleEditor : public CSerializableEditor
		{
		protected:
			Particle::CParticleSerializable* m_ps;
			CObjectSerializable* m_data;

			bool m_isChanged;

		public:
			CParticleEditor();

			virtual ~CParticleEditor();

			virtual void closeGUI();

			virtual void initGUI(Particle::CParticleSerializable* ps, CSpaceProperty* ui);

			virtual void onUpdateValue(CObjectSerializable* object);

			virtual void initCustomValueGUI(CObjectSerializable* object, CValueProperty* data, GUI::CBoxLayout* layout, CSpaceProperty* ui);

			CObjectSerializable* getData()
			{
				return m_data;
			}

			SColor getColor(Particle::CModel* r, Particle::CModel* g, Particle::CModel* b, int state);

			void setColor(const SColor& c, Particle::CModel* r, Particle::CModel* g, Particle::CModel* b, int state);
		};
	}
}