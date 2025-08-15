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

#include "pch.h"

#include "Lighting/CDirectionalLight.h"
#include "Lighting/CPointLight.h"
#include "Lighting/CSpotLight.h"

#include "RenderMesh/CRenderMesh.h"
#include "RenderMesh/CRenderMeshInstancing.h"
#include "OcclusionQuery/COcclusionQuery.h"
#include "IndirectLighting/CIndirectLighting.h"
#include "LightProbes/CLightProbes.h"
#include "Lightmap/CLightmap.h"
#include "Animation/CAnimationController.h"

#ifdef BUILD_SKYLICHT_LIGHMAPPER
#include "BakeLightComponent/CBakeLightComponent.h"
using namespace Skylicht::Lightmapper;
#endif

#include "SkyDome/CSkyDome.h"
#include "SkyBox/CSkyBox.h"

#include "Primitive/CCube.h"
#include "Primitive/CSphere.h"
#include "Primitive/CPlane.h"
#include "Primitive/CCapsule.h"
#include "Primitive/CCylinder.h"

#include "RenderLine/CRenderLine.h"

#include "ParticleSystem/CParticleComponent.h"
#include "ParticleSystem/CParticleTrailComponent.h"
using namespace Particle;

#ifdef BUILD_SKYLICHT_PHYSIC
#include "Collider/CBoxCollider.h"
#include "Collider/CStaticPlaneCollider.h"
#include "Collider/CBvhMeshCollider.h"
#include "Collider/CConvexMeshCollider.h"
#include "Collider/CMeshCollider.h"
#include "Collider/CCylinderCollider.h"
#include "Collider/CCapsuleCollider.h"
#include "Collider/CSphereCollider.h"
#include "RigidBody/CRigidbody.h"
using namespace Physics;
#endif

#ifdef BUILD_SKYLICHT_GRAPH
#include "Graph/CGraphComponent.h"
using namespace Graph;
#endif

namespace Skylicht
{
	namespace Editor
	{
		// BEGIN DECLARE COMPONENT THAT WILL COMPILE
		USE_COMPONENT(CDirectionalLight);
		USE_COMPONENT(CPointLight);
		USE_COMPONENT(CSpotLight);

		USE_COMPONENT(CRenderMesh);
		USE_COMPONENT(CRenderMeshInstancing);
		USE_COMPONENT(COcclusionQuery);
		USE_COMPONENT(CAnimationController);

		USE_COMPONENT(CSkyDome);
		USE_COMPONENT(CSkyBox);

		USE_COMPONENT(CIndirectLighting);
		USE_COMPONENT(CLightProbes);
		USE_COMPONENT(CLightmap);

		USE_COMPONENT(CCube);
		USE_COMPONENT(CSphere);
		USE_COMPONENT(CPlane);
		USE_COMPONENT(CCapsule);
		USE_COMPONENT(CCylinder);

		USE_COMPONENT(CRenderLine);

		USE_COMPONENT(CParticleComponent);
		USE_COMPONENT(CParticleTrailComponent);

#ifdef BUILD_SKYLICHT_PHYSIC
		USE_COMPONENT(CRigidbody);
		USE_COMPONENT(CBoxCollider);
		USE_COMPONENT(CStaticPlaneCollider);
		USE_COMPONENT(CMeshCollider);
		USE_COMPONENT(CBvhMeshCollider);
		USE_COMPONENT(CConvexMeshCollider);
		USE_COMPONENT(CCylinderCollider);
		USE_COMPONENT(CCapsuleCollider);
		USE_COMPONENT(CSphereCollider);
#endif

#ifdef BUILD_SKYLICHT_GRAPH
		USE_COMPONENT(CGraphComponent);
#endif

#ifdef BUILD_SKYLICHT_LIGHMAPPER
		USE_COMPONENT(CBakeLightComponent);
#endif

		// END DECLARE COMPONENT
	}
}