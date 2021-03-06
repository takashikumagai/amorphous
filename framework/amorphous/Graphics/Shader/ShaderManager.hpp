#ifndef __ShaderManager_HPP__
#define __ShaderManager_HPP__


#include <memory>
#include "ShaderTechniqueHandle.hpp"
#include "ShaderParameter.hpp"
#include "amorphous/Graphics/D3DHeaders.hpp"
#include "amorphous/3DMath/Matrix34.hpp"
#include "amorphous/3DMath/Matrix44.hpp"
#include "amorphous/3DMath/MatrixConversions.hpp"
#include "amorphous/3DMath/Transform.hpp"
#include "amorphous/Support/stream_buffer.hpp"


namespace amorphous
{


class ShaderManager
{
/*
	enum eShaderConstParam
	{
		NUM_MAX_TECHNIQUES = 64,
		NUM_TEXTURE_STAGES = 8,
		NUM_MAX_CUBETEXTURES = 4
	};

	enum eHandleID
	{
		HANDLE_VIEWER_POS = 0,
		HANDLE_AMBIENT_COLOR,
		NUM_HANDLES,
	};

	enum eMatHandleID
	{
		MATRIX_WORLD = 0,
		MATRIX_VIEW,
		MATRIX_PROJ,
		MATRIX_WORLD_VIEW,
		MATRIX_WORLD_VIEW_PROJ,
		NUM_MATRIX_HANDLES
	};
*/
	bool m_RegisteredToHub;

private:

	virtual bool Init() { return true; }

protected:

	int GetTechniqueIndex( ShaderTechniqueHandle& tech_handle ) { return tech_handle.GetTechniqueIndex(); }

	void SetInvalidTechnique( ShaderTechniqueHandle& tech_handle ) { tech_handle.SetTechniqueIndex( ShaderTechniqueHandle::INVALID_INDEX ); }

	void SetTechniqueIndex( ShaderTechniqueHandle& tech_handle, int index ) { return tech_handle.SetTechniqueIndex( index ); }

	bool IsUninitializedTechnique( ShaderTechniqueHandle& tech_handle ) { return tech_handle.GetTechniqueIndex() == ShaderTechniqueHandle::UNINITIALIZED; }

	bool IsInvalidTechnique( ShaderTechniqueHandle& tech_handle ) { return tech_handle.GetTechniqueIndex() == ShaderTechniqueHandle::INVALID_INDEX; }

	template<typename T>
	T GetParameterValue( ShaderParameter<T> param ) { return param.m_Parameter; }

	template<typename T>
	int GetParameterIndex( ShaderParameter<T> param ) { return param.m_ParameterIndex; }

	template<typename T>
	void SetParameterIndex( ShaderParameter<T>& param, int index ) { param.m_ParameterIndex = index; }

public:

	ShaderManager();

	virtual ~ShaderManager();

	virtual bool LoadShaderFromFile( const std::string& filename ) { return false; }

	virtual bool LoadShaderFromText( const stream_buffer& buffer ) { return false; }

	virtual bool LoadShaderFromText( const std::string& vertex_shader, const std::string& fragment_shader ) { return false; }

	virtual LPD3DXEFFECT GetEffect() { return NULL; }

	virtual void Release() {}

	virtual void Reload() {}

	virtual void SetWorldTransform( const Matrix34& world_pose ) { SetWorldTransform( ToMatrix44(world_pose) ); }

	virtual void SetWorldTransform( const Matrix44& matWorld ) {}

	virtual void SetViewTransform( const Matrix44& matView ) {}

	virtual void SetProjectionTransform( const Matrix44& matProj ) {}

	virtual void SetWorldViewTransform( const Matrix44& matWorld, const Matrix44& matView  ) {}

	virtual void SetWorldViewProjectionTransform( const Matrix44& matWorld, const Matrix44& matView, const Matrix44& matProj ) {}


	virtual void GetWorldTransform( Matrix44& matWorld ) const {}

	virtual void GetViewTransform( Matrix44& matView ) const {}

//	virtual void GetProjectionTransform( Matrix44& matProj ) const {}

	inline Matrix44 GetWorldTransform() const;

	inline Matrix44 GetViewTransform() const;

//	inline Matrix44 GetProjectionTransform() const;


	virtual void SetViewerPosition( const Vector3& vEyePosition ) {}

	virtual void SetVertexBlendMatrix( int i, const Matrix34& mat ) {}

	virtual void SetVertexBlendMatrix( int i, const Matrix44& mat ) {}

	virtual void SetVertexBlendTransforms( const std::vector<Transform>& src_transforms ) {}


	virtual Result::Name SetTexture( const int iStage, const TextureHandle& texture ) { return Result::UNKNOWN_ERROR; }

	virtual Result::Name SetCubeTexture( const int index, const TextureHandle& cube_texture ) { return Result::UNKNOWN_ERROR; }

	virtual void Begin() {}

	virtual void End() {}

	virtual Result::Name SetTechnique( const unsigned int id ) { return Result::UNKNOWN_ERROR; }

	virtual Result::Name SetTechnique( ShaderTechniqueHandle& tech_handle ) { return Result::UNKNOWN_ERROR; }

//	bool RegisterTechnique( const unsigned int id, const char *pcTechnique );

	// Sets a single integer value
	virtual void SetParam( ShaderParameter<int>& int_param ) {}

	// Sets a single float value
	virtual void SetParam( ShaderParameter<float>& float_param ) {}

	// Sets a single float2 (Vector2) value
	virtual void SetParam( ShaderParameter<Vector2>& vec2_param ) {}

	// Sets a single float3 (Vector3) value
	virtual void SetParam( ShaderParameter<Vector3>& vec3_param ) {}

	// Sets a color value as 4 floats in RGBA order
	virtual void SetParam( ShaderParameter<SFloatRGBAColor>& color_param ) {}

	// Sets one or more float values
	virtual void SetParam( ShaderParameter< std::vector<float> >& float_param ) {}

	// Sets a texture
	virtual void SetParam( ShaderParameter<TextureParam>& tex_param ) {}

	// Sets a column-major 4x4 matrix
	virtual void SetParam( ShaderParameter<Matrix44>& mat44_param ) {}

	// Sets a single float value
	virtual void SetParam( const char *parameter_name, int int_param ) {}

	// Sets a single float value
	virtual void SetParam( const char *parameter_name, float float_param ) {}

	// Sets a single float2 (Vector2) value
	virtual void SetParam( const char *parameter_name, const Vector2& vec2_param ) {}

	// Sets a single float3 (Vector3) value
	virtual void SetParam( const char *parameter_name, const Vector3& vec3_param ) {}

	// Sets a color value as 4 floats in RGBA order
	virtual void SetParam( const char *parameter_name, const SFloatRGBAColor& color_param ) {}

	// Sets one or more float values
	virtual void SetParam( const char *parameter_name, const float *float_param, uint num_float_values ) {}

	// Sets a Vector2 array
	virtual void SetParam( const char *parameter_name, const Vector2 *vec2_param, uint num_vec2_values ) {}

	// Sets a Vector3 array
	virtual void SetParam( const char *parameter_name, const Vector3 *vec3_param, uint num_vec3_values ) {}

	// Sets a Vector4 array
	virtual void SetParam( const char *parameter_name, const Vector4 *vec4_param, uint num_vec4_values ) {}

	// Sets one or more float values
	inline void SetParam( const char *parameter_name, const std::vector<float>& float_param );

	// Sets a column-major 4x4 matrix
	virtual void SetParam( const char *parameter_name, const Matrix44& mat44_param ) {}

	virtual void SetBool( const char *parameter_name, bool bool_param ) {}

//	void SetParam( ShaderParameter< std::vector<int> >& integer_param );

//	void SetTextureParam()

	virtual std::shared_ptr<ShaderLightManager> GetShaderLightManager() { return std::shared_ptr<ShaderLightManager>(); }

	friend class ShaderManagerHub;
};


//============================ inline implementations ============================

inline void ShaderManager::SetParam( const char *parameter_name, const std::vector<float>& float_param )
{
	if( float_param.empty() )
		return;

	SetParam( parameter_name, &float_param[0], (uint)float_param.size() );
}


inline Matrix44 ShaderManager::GetWorldTransform() const
{
	Matrix44 world;
	GetWorldTransform( world );
	return world;
}


inline Matrix44 ShaderManager::GetViewTransform() const
{
	Matrix44 view;
	GetViewTransform( view );
	return view;
}

/*
inline Matrix44 ShaderManager::GetProjectionTransform() const
{
	Matrix44 proj;
	GetProjectionTransform( proj );
	return proj;
}
*/

} // namespace amorphous



#endif  /*  __HLSLShaderManager_HPP__  */
