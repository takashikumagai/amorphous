#ifndef __GLProgressiveMeshImpl_HPP__
#define __GLProgressiveMeshImpl_HPP__


#include "GLBasicMeshImpl.hpp"


namespace amorphous
{


/// NOT IMPLEMENTED YET
class CGLProgressiveMeshImpl : public CGLBasicMeshImpl
{

public:

	CGLProgressiveMeshImpl();

	CGLProgressiveMeshImpl( const std::string& filename );

	virtual ~CGLProgressiveMeshImpl();

	void Release() {}

//	virtual bool LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename, U32 option_flags );

//	bool LoadFromFile( const std::string& filename, U32 option_flags = 0 );

//	virtual bool CreateMesh( int num_vertices, int num_indices, U32 option_flags, std::vector<D3DVERTEXELEMENT9>& vecVertexElement ) {}

	/// render object by using the fixed function pipeline
//	void Render();

//	virtual void Render( CShaderManager& rShaderMgr );

//	virtual void RenderSubsets( CShaderManager& rShaderMgr,
//		                        const std::vector<int>& vecMaterialIndex,
//								std::vector<CShaderTechniqueHandle>& vecShaderTechnique );

//	virtual void RenderSubsets( CShaderManager& rShaderMgr,
//		                        const std::vector<int>& vecMaterialIndex /* some option to specify handles for texture */);

//	virtual CMeshType::Name GetMeshType() const { return CMeshType::PROGRESSIVE; }
};


} // namespace amorphous



#endif	/*  __GLProgressiveMeshImpl_HPP__  */
