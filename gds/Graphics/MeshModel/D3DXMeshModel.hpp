#ifndef  __3DMESHMODEL_H__
#define  __3DMESHMODEL_H__


#include <string>
#include <d3dx9.h>

#include "3DMath/Transform.hpp"
#include "Graphics/TextureHandle.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderManager.hpp"
#include "Graphics/Shader/FixedFunctionPipelineManager.hpp"
#include "3DMeshModelArchive.hpp"
#include "MeshBone.hpp"


//namespace GameLib1
//{

namespace MeshModel
{


class CD3DXMeshModel;


//=========================================================================================
// CMM_Material
//=========================================================================================

class CMM_Material
{
	float m_fSpecular;
	
	/// used for textures loaded from files
	/// actual textures are stored in texture manager.
	/// texture manager and texture handles are used
	/// to share the texture between different models.
	CTextureHandle m_SurfaceTexture;
	CTextureHandle m_NormalMap;

	/// used for textures created by programs
	LPDIRECT3DTEXTURE9 m_pSurfaceTexture;
	LPDIRECT3DTEXTURE9 m_pNormalMap;

	friend class CD3DXMeshModel;

	CMM_Material()
	{
		m_fSpecular = 0.0f;
		m_pSurfaceTexture = NULL;
		m_pNormalMap = NULL;
	}
};


//=========================================================================================
// CD3DXMeshModel
//=========================================================================================

class CD3DXMeshModel
{

public:

	CD3DXMeshModel();

	~CD3DXMeshModel();

	void Release();

	bool LoadFromFile( const char *pFilename );

	bool LoadFromArchive( C3DMeshModelArchive& rArchive, const std::string& strModelFilename = "" );

	/// apply hierarchical transformations
	/// given an array of world transform matrices for each bone
	inline void UpdateTransforms( Matrix34 *paWorldTransform );

	inline void SetTransforms_Local( Matrix34 *paLocalTransform );

//	inline void GetWorldTransforms( D3DXMATRIX*& paDestWorldTransform ) { paDestWorldTransform = m_paBoneMatrix; }

	inline Transform *GetWorldTransforms() { return m_paTransforms; }

	inline int GetNumBones() const { return m_iNumBones; }

	void Render( const unsigned int RenderFlag = 0, CShaderManager& shader_mgr = FixedFunctionPipelineManager() );

	bool LockVertexBuffer( void*& pVertex );

	void UnlockVertexBuffer();

	enum eParam
	{
		PARAM_SURFACE_TEXTURE = 0,
		PARAM_NORMALMAP_TEXTURE,
		PARAM_SPECULAR,
		NUM_PARAMS
	};

	enum eRenderFlag
	{
		RENDER_USE_FIXED_FUNCTION_SHADER	= (1 << 0),
		RENDER_IGNORE_TEXTURE_SETTINGS		= (1 << 1),
		RENDER_DISABLE_ALPHABLEND			= (1 << 2),
	};


private:

	void LoadVertices( void*& pVBData, int& riSize, C3DMeshModelArchive& archive );

	void RenderNonHLSL( unsigned int RenderFlag );

	std::string m_strFilename;

	int m_iNumMaterials;

	CMM_Material *m_paMaterial;
	CMMA_TriangleSet *m_paTriangleSet;

	/// holds vertex blend matrices (world transforms)
	Transform *m_paTransforms;
	int m_iNumBones;

	/// root node of the hierarchical structure
    CMM_Bone *m_pRootBone;

	/// flexible vertex format flag
	DWORD m_dwFVF;

	/// the number of vertices
	int m_iNumVertices;

	/// size of fvf vertex (in bytes)
	int m_iVertexSize;

	/// vertex decleration
	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDecleration;

	D3DXHANDLE m_hParameter[NUM_PARAMS];

	/// Direct3D vertex & index buffers
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9	m_pIB;

};



inline void CD3DXMeshModel::UpdateTransforms( Matrix34 *paWorldTransform )
{
	if( !m_pRootBone || !m_paTransforms )
		return;	// the mesh does not have a skeleton

	int i, num_bones = m_iNumBones;
	for( i=0; i<num_bones; i++ )
	{
		m_paTransforms[i].FromMatrix34( paWorldTransform[i] );
	}
/*
	int index = 0;
	m_pRootBone->Transform_r( paWorldTransform, index );*/
}


inline void CD3DXMeshModel::SetTransforms_Local( Matrix34 *paLocalTransform )
{
	if( !m_pRootBone )
		return;

	int index = 0;
	m_pRootBone->Transform_r( NULL, paLocalTransform, index );

	CShaderManager *pShaderMgr = CShader::Get()->GetCurrentShaderManager();
	if( !pShaderMgr )
		return;

	LPD3DXEFFECT pEffect = pShaderMgr->GetEffect();

	if( !pEffect )
		return;

	HRESULT hr;
	char acParam[32];
	int i, num_bones = m_iNumBones;
	for( i=0; i<num_bones; i++ )
	{
		sprintf( acParam, "g_aBlendMatrix[%d]", i );
		D3DXMATRIX mat;
		m_paTransforms[i].ToMatrix34().GetRowMajorMatrix44( (float *)mat );
		hr = pEffect->SetMatrix( acParam, &mat );

		if( FAILED(hr) ) return;
	}
}


}	/*  3DMesh  */

//}	/* GameLib1   */


#endif		/*  __3DMESHMODEL_H__  */
