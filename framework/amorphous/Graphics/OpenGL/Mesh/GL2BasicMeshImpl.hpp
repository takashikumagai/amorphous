#ifndef __GL2BasicMeshImpl_HPP__
#define __GL2BasicMeshImpl_HPP__


#include "amorphous/base.hpp"
#include <gl/gl.h>											// Header File For The OpenGL32 Library
#include <gl/glu.h>											// Header File For The GLu32 Library
#include "amorphous/Graphics/Mesh/BasicMesh.hpp"
#include "amorphous/Graphics/MeshModel/3DMeshModelArchive.hpp"


namespace amorphous
{
using namespace serialization;


class CGL2BasicMeshImpl : public MeshImpl
{
private:

	bool m_VBOSupported;

	// Mesh Data
//	int				m_nVertexCount;								// Vertex Count

	// Vertex Buffer Object Names
	GLuint m_PositionBuffer;							// Vertex VBO Name
	GLuint m_NormalBuffer;							    // vertex normal?
	GLuint m_DiffuseColorBuffer;						// diffuse color?
	GLuint m_TexCoordBuffer;							// Texture Coordinate VBO Name

	GLuint m_IndexBuffer;

	std::vector<int> m_vecClientState;

	std::vector<CMMA_TriangleSet> m_vecTriangleSet;

	int m_NumIndices;

	std::vector<unsigned short> m_vecIndex;

private:

	void BuildVBOs( C3DMeshModelArchive& archive );

	void BuildVBOs_ARB( C3DMeshModelArchive& archive );

	void BindBuffers_ARB();

public:

	CGL2BasicMeshImpl();

	CGL2BasicMeshImpl( const std::string& filename );

	virtual ~CGL2BasicMeshImpl();

	void Release();

	virtual bool LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename, U32 option_flags );

	bool LoadFromFile( const std::string& filename, U32 option_flags = 0 );

//	bool CreateMesh( int num_vertices, int num_indices, U32 option_flags, std::vector<D3DVERTEXELEMENT9>& vecVertexElement );

	bool CreateMesh( int num_vertices, int num_indices, U32 option_flags, U32 vertex_format_flags );

	/// render object by using the fixed function pipeline
	void Render();

	virtual void Render( ShaderManager& rShaderMgr );

	virtual void RenderSubset( ShaderManager& rShaderMgr, int material_index );

	virtual void RenderSubsets( ShaderManager& rShaderMgr,
		                        const std::vector<int>& vecMaterialIndex,
								std::vector<ShaderTechniqueHandle>& vecShaderTechnique );

	virtual void RenderSubsets( ShaderManager& rShaderMgr,
		                        const std::vector<int>& vecMaterialIndex /* some option to specify handles for texture */);
};


} // namespace amorphous



#endif	/*  __GL2BasicMeshImpl_HPP__  */
