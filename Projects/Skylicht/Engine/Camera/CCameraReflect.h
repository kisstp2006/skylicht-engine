/*
!@
MIT License

Copyright (c) 2024 Skylicht Technology CO., LTD

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

#include "CCamera.h"
#include "Components/CComponentSystem.h"
#include "Components/ILateUpdate.h"

namespace Skylicht
{
	/// @brief This is an object class that sets up a camera to be symmetrical with a selected camera. It's used for rendering planar reflections.
	/// @ingroup Camera
	class SKYLICHT_API CCameraReflect :
		public CComponentSystem,
		public ILateUpdate
	{
	protected:
		CCamera* m_camera;
		CCamera* m_targetCamera;

		core::plane3df m_plane;

	public:
		CCameraReflect();

		virtual ~CCameraReflect();

		virtual void initComponent();

		virtual void updateComponent();

		virtual void lateUpdate();

		void setTargetCamera(CCamera* cam);

		inline CCamera* getTargetCamera()
		{
			return m_targetCamera;
		}

		inline void setPlane(const core::plane3df& p)
		{
			m_plane = p;
		}
	};
}