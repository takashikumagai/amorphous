#ifndef __MeshBonesUpdateCallback_HPP__
#define __MeshBonesUpdateCallback_HPP__


#include "GraphicsResourcesUpdateCallback.hpp"
#include "../Graphics/Shader/BlendTransformsLoader.hpp"
#include "../Graphics/MeshObjectHandle.hpp"
#include "../Graphics/Mesh/SkeletalMesh.hpp"


namespace amorphous
{


/// Calculates the blend transforms for a skeletal mesh
class CMeshBonesUpdateCallback : public CGraphicsResourcesUpdateCallback
{
	std::vector<Transform> m_LocalTransforms;

	MeshHandle m_TargetSkeletalMesh;

	std::shared_ptr<BlendTransformsLoader> m_pBlendTransformsLoader;

public:

	CMeshBonesUpdateCallback() {}

	CMeshBonesUpdateCallback( std::shared_ptr<BlendTransformsLoader> pBlendTransformsLoader )
		:
	m_pBlendTransformsLoader(pBlendTransformsLoader)
	{}

	Result::Name SetSkeletalMesh( MeshHandle target_skeletal_mesh )
	{
		std::shared_ptr<BasicMesh> pMesh = target_skeletal_mesh.GetMesh();
		if( !pMesh )
			return Result::INVALID_ARGS;

		std::shared_ptr<SkeletalMesh> pSMesh = std::dynamic_pointer_cast<SkeletalMesh,BasicMesh>(pMesh);
		if( !pSMesh )
			return Result::INVALID_ARGS;

		m_TargetSkeletalMesh = target_skeletal_mesh;

		return Result::SUCCESS;
	}

	void SetBlendTransformsLoader( std::shared_ptr<BlendTransformsLoader> pBlendTransformsLoader ) { m_pBlendTransformsLoader = pBlendTransformsLoader; }

	/// Update the blend transforms stored in the blend transforms loader.
	void UpdateGraphics()
	{
		
		if( !m_pBlendTransformsLoader )
			return;

		std::vector<Transform>& dest_blend_transforms = m_pBlendTransformsLoader->BlendTransforms();
		dest_blend_transforms.resize( m_LocalTransforms.size() );

		std::shared_ptr<BasicMesh> pMesh = m_TargetSkeletalMesh.GetMesh();
		if( !pMesh )
			return;

		std::shared_ptr<SkeletalMesh> pSMesh = std::dynamic_pointer_cast<SkeletalMesh,BasicMesh>(pMesh);
		if( !pSMesh )
			return;

		pSMesh->CalculateBlendTransforms( m_LocalTransforms, dest_blend_transforms );
	}

	std::vector<Transform>&  MeshBoneLocalTransforms() { return m_LocalTransforms; }
};


//void SetGraphicsUpdateCallbackForSkeletalMesh( CGamteItem& item )
//{
//}

} // amorphous



#endif /* __MeshBonesUpdateCallback_HPP__ */
