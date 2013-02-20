#ifndef  __PrimitiveRendererTest_H__
#define  __PrimitiveRendererTest_H__


#include "gds/Graphics/fwd.hpp"
#include "gds/Graphics/MeshObjectHandle.hpp"
#include "gds/Graphics/ShaderHandle.hpp"
#include "gds/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "gds/Input/fwd.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class CPrimitiveRendererTest : public CGraphicsTestBase
{
	ShaderHandle m_Shader;

	ShaderTechniqueHandle m_MeshTechnique;

	boost::shared_ptr<FontBase> m_pFont;

	std::string m_TextBuffer;

private:

//	bool InitShader();

public:

	CPrimitiveRendererTest();

	~CPrimitiveRendererTest();

	const char *GetAppTitle() const { return "PrimitiveRendererTest"; }

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

	virtual void HandleInput( const InputData& input );
};


#endif /* __PrimitiveRendererTest_H__ */
