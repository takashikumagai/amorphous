#ifndef  __OBBTreeDemo_HPP__
#define  __OBBTreeDemo_HPP__


#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Input/fwd.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class OBBTreeDemo : public CGraphicsTestBase
{
	boost::shared_ptr<OBBTree> m_pOBBTree;

	int m_DrawLevel;

	int m_NumDrawLevels;

//	MeshHandle m_SkyboxMesh;

	MeshHandle m_Mesh;

//	std::vector<MeshHandle> m_vecpMeshes;

	ShaderTechniqueHandle m_MeshTechnique;

//	ShaderTechniqueHandle m_SkyboxTechnique;

	ShaderTechniqueHandle m_DefaultTechnique;

	ShaderHandle m_Shader;

private:

	void InitOBBTree( C3DMeshModelArchive& mesh_archive );

public:

	OBBTreeDemo();

	~OBBTreeDemo();

	const char *GetAppTitle() const { return "OBBTreeDemo"; }

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

	void HandleInput( const InputData& input );
};


#endif /* __OBBTreeDemo_HPP__ */
