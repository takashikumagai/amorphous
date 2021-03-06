#ifndef  __MiscShaderDemo_HPP__
#define  __MiscShaderDemo_HPP__


#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Input/fwd.hpp"
#include "amorphous/Graphics/Mesh/CustomMesh.hpp"
#include "amorphous/Support/indexed_vector.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class MiscShaderDemo : public CGraphicsTestBase
{
	indexed_vector<MeshHandle> m_Meshes;

	indexed_vector<ShaderHandle> m_Shaders;

	ShaderTechniqueHandle m_MeshTechnique;

	bool m_RenderAllMeshes;

//	CustomMesh m_Mesh;

//	MeshHandle m_RegularMesh;

//	unsigned int m_CurrentShader;

private:

	bool InitShaders();

	void RenderMeshes();

	ShaderHandle GetCurrentShader();

public:

	MiscShaderDemo();

	~MiscShaderDemo();

	const char *GetAppTitle() const { return "MiscShaderDemo"; }

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

	virtual void HandleInput( const InputData& input );
};


#endif /* __MiscShaderDemo_HPP__ */
