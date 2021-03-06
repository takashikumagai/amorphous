#ifndef  __D3DXMESHOBJECTBASE_H__
#define  __D3DXMESHOBJECTBASE_H__


#include "3DMath/Sphere.hpp"
#include "Graphics/Mesh/BasicMesh.hpp"
#include "Graphics/Direct3D/Direct3D9.hpp"
#include "Graphics/Shader/FixedFunctionPipelineManager.hpp"


namespace amorphous
{


extern void LoadVerticesForD3DXMesh( const CMMA_VertexSet& rVertexSet,                // [in]
									 std::vector<D3DVERTEXELEMENT9>& vecVertElement,  // [out]
						             int &vertex_size,                                // [out]
						             void*& pVBData                                   // [out]
							         );

extern bool LoadIndices( C3DMeshModelArchive& archive, std::vector<U16>& vecIBData );

extern void GetAttributeTableFromTriangleSet( const std::vector<CMMA_TriangleSet>& vecTriangleSet,
									          std::vector<D3DXATTRIBUTERANGE>& vecAttributeRange );


/**
 Base class of the D3D implementation of the mesh class
*/
class CD3DXMeshObjectBase : public MeshImpl
{
protected:

	/// bounding sphere in local space of the model
	/// - Each implementation must properly initialize the sphere
	Sphere m_LocalShpere;


	// D3D attributes

	D3DMATERIAL9 *m_pMeshMaterials;

	/// flexible vertex format flag
	DWORD m_dwFVF;

	/// size of vertex (in bytes)
	int m_iVertexSize;

	/// index size in bytes. This is either 16 or 32. Default: 16
	unsigned int m_IndexSizeInBits;

	/// vertex elements
	/// - updated when LoadVertices() is called.
	D3DVERTEXELEMENT9 *m_paVertexElements;

	/// vertex decleration
	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDecleration;

private:

protected:

	virtual void LoadVertices( void*& pVBData, C3DMeshModelArchive& archive );

	HRESULT LoadMaterials( D3DXMATERIAL* d3dxMaterials, int num_materials );

//	virtual const D3DVERTEXELEMENT9 *GetVertexElemenets( CMMA_VertexSet& rVertexSet );

	bool FillIndexBuffer( LPD3DXMESH pMesh, C3DMeshModelArchive& archive );

	/// materials must be loaded before calling this method
	HRESULT SetAttributeTable( LPD3DXMESH pMesh, const std::vector<CMMA_TriangleSet>& vecTriangleSet );

	HRESULT LoadD3DMaterialsFromArchive( C3DMeshModelArchive& archive );

	/// Synchronously load D3DXMesh
	LPD3DXMESH LoadD3DXMeshFromArchive( C3DMeshModelArchive& archive );

	/// \param xfilename [in] .x file
	/// \param rpMesh [out] reference to a pointer that receives the mesh loaded by this function
	/// \param rpAdjacencyBuffer [out] reference to a pointer that receives adjacency buffer
	/// materials are also loaded from the input .x file and stored in CD3DXMeshObjectBase::m_pMeshMaterials
	HRESULT LoadD3DXMeshAndMaterialsFromXFile( const std::string& xfilename,
		                                       LPD3DXMESH& rpMesh,
		                                       LPD3DXBUFFER& rpAdjacencyBuffer );

	/// check the attribute tables (for debugging)
	void PeekAttribTables( LPD3DXBASEMESH pMesh );

	/// procedures to create bounding sphere from D3DXMesh
	/// - Deprecated, and nobody's using this. See this as a sample code
	/// - The bounding shpere should be create from mesh archives
	HRESULT CreateLocalBoundingSphereFromD3DXMesh( LPD3DXMESH pMesh );

	/// load mesh from .x file
	/// - Declared protected since this is a platform dependent
	///   and sould not be directly called by outside module
	///   unlike LoadFromArchive()
	virtual HRESULT LoadFromXFile( const std::string& filename ) = 0;

public:

	inline CD3DXMeshObjectBase();

	virtual ~CD3DXMeshObjectBase() { Release(); }

	/// returns true on success
	bool LoadFromFile( const std::string& filename, U32 option_flags = 0 );

	/// load basic mesh properties from a mesh archive
	/// NOTE: filename is required to get the path for textures files
	virtual bool LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename, U32 option_flags = 0 ) = 0;

	virtual void Release();

	virtual LPD3DXBASEMESH GetBaseMesh() { return NULL; }

	virtual const LPD3DXBASEMESH GetBaseMesh() const { return NULL; }

	bool LoadNonAsyncResources( C3DMeshModelArchive& rArchive, U32 option_flags );

	virtual bool CreateMesh( int num_vertices, int num_indices, U32 option_flags, std::vector<D3DVERTEXELEMENT9>& vecVertexElement ) { return false; }

	/// added to call LockAttributeBuffer() during asynchrnous loading
	/// - LockAttributeBuffer() is not a member of LPD3DXBASEMESH
	virtual LPD3DXMESH GetMesh() { return NULL; }

	inline int GetNumMaterials() const { return (int)m_vecMaterial.size(); }

	inline const D3DMATERIAL9& GetD3DMaterial( int i ) const { return m_pMeshMaterials[i]; }

	/// returns const reference to the i-th material
	inline const MeshMaterial& GetMaterial( int material_index ) const { return m_vecMaterial[material_index]; }

	/// returns non-const reference to the i-th material
	inline MeshMaterial& Material( int material_index ) { return m_vecMaterial[material_index]; }

	bool CreateVertexDeclaration();

	inline LPDIRECT3DVERTEXDECLARATION9 GetVertexDeclaration() { return m_pVertexDecleration; }

	int GetVertexSize() const { return m_iVertexSize; }

	/// user is responsible for updating the visibility by calling UpdateVisibility( const Camera& cam )
	void ViewFrustumTest( bool do_test ) { m_bViewFrustumTest = do_test; }

	void UpdateVisibility( const Camera& cam );

	inline bool IsMeshVisible( int triset_index ) const { return m_IsVisible[triset_index]==1 ? true : false; }


	inline bool IsMeshVisible() const { return m_IsVisible[m_vecMaterial.size()]==1 ? true : false; }

	/// the number of textures for the i-th material
	int GetNumTextures( int material_index ) const { return (int)m_vecMaterial[material_index].Texture.size(); }

	const AABB3& GetAABB( int material_index ) const { return m_vecAABB[material_index]; }

	//
	// render functions
	//

	/// renders a single subset of the mesh with the current shader technique
	inline void RenderSubset( ShaderManager& rShaderMgr, int material_index );

	/// renders subsets of the mesh
	/// - use different shader techniques for each material
//	virtual void RenderSubsets( ShaderManager& rShaderMgr,
//		                        const std::vector<int>& vecMaterialIndex,
//								std::vector<ShaderTechniqueHandle>& vecShaderTechnique );

	void RenderSubsets( ShaderManager& rShaderMgr,
		                const int *paMaterialIndex,
						ShaderTechniqueHandle *paShaderTechnique,
						int num_indices );

	void RenderSubsetsCg( LPD3DXBASEMESH pMesh,
		                  ShaderManager& rShaderMgr,
		                  const int *paMaterialIndex,
						  ShaderTechniqueHandle *paShaderTechnique,
						  int num_indices );

	/// renders subsets of the mesh with the current shader technique
	/// - the same shader technique is used to render all the subsets
//	virtual void RenderSubsets( ShaderManager& rShaderMgr,
//		                        const std::vector<int>& vecMaterialIndex /* some option to specify handles for texture */);

	void RenderSubsets( ShaderManager& rShaderMgr,
		                const int *paMaterialIndex,
						int num_indices
						/* some option to specify handles for texture */ );

	/// renders the mesh with the current shader technique
	/// - Assumes that you have already set a valid technique that can be obtained from 'rShaderMgr'
	/// - the same shader technique is used to render all the materials
	inline virtual void Render( ShaderManager& rShaderMgr );

	/// Use this when you wanna use different shader techniques for each material
	/// \param vecShaderTechnique shader techniques for each material
	/// - What to do if the single shader technique is applied for all materials
	///   - shader_mgr.SetShaderTechnique() and call CD3DXMeshObjectBase::Render( shader_mgr )
	inline void Render( ShaderManager& rShaderMgr, std::vector<ShaderTechniqueHandle>& vecShaderTechnique );

	/// render object by using the fixed function pipeline
	void Render();

	virtual unsigned int GetNumVertices() const;

	virtual unsigned int GetNumTriangles() const;

	virtual bool LockVertexBuffer( void*& pLockedVertexBuffer );

	virtual bool LockIndexBuffer( void*& pLockedIndexBuffer );

	virtual bool LockAttributeBuffer( DWORD*& pLockedAttributeBuffer );

	virtual bool UnlockVertexBuffer();

	virtual bool UnlockIndexBuffer();

	virtual bool UnlockAttributeBuffer();

	void SetVertexDeclaration();

	virtual MeshType::Name GetMeshType() const = 0;

	friend class CD3DXMeshVerticesLoader;
	friend class CD3DXMeshIndicesLoader;
};



// ================================= inline implementations =================================


inline CD3DXMeshObjectBase::CD3DXMeshObjectBase()
:
m_pMeshMaterials(NULL),
m_iVertexSize(0),
m_IndexSizeInBits(16),
m_paVertexElements(NULL),
m_pVertexDecleration(NULL)
{
}


/// \param vecMaterialIndex indices of materials(subsets) to render 
inline void CD3DXMeshObjectBase::Render( ShaderManager& rShaderMgr )
{
	if( &rShaderMgr == &FixedFunctionPipelineManager() )
		Render();
	else
		MeshImpl::RenderSubsets( rShaderMgr, m_vecFullMaterialIndices );
}


inline void CD3DXMeshObjectBase::Render( ShaderManager& rShaderMgr,
										 std::vector<ShaderTechniqueHandle>& vecShaderTechnique )
{
	MeshImpl::RenderSubsets( rShaderMgr, m_vecFullMaterialIndices, vecShaderTechnique );
}


inline void CD3DXMeshObjectBase::RenderSubset( ShaderManager& rShaderMgr, int material_index )
{
//	vector<int> single_index;
//	single_index.push_back( material_index );
//	RenderSubsets( rShaderMgr, single_index );

	RenderSubsets( rShaderMgr, &material_index, 1 );
}

} // namespace amorphous



#endif		/*  __D3DXMESHOBJECTBASE_H__  */
