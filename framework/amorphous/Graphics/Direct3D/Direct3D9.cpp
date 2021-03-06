#include "Direct3D9.hpp"
#include "3DMath/Matrix44.hpp"
#include "Graphics/Direct3D/Conversions.hpp"
#include "Graphics/Direct3D/D3DSurfaceFormat.hpp"
#include "Graphics/Direct3D/D3DAlphaBlend.hpp"
#include "Graphics/Direct3D/D3DTextureResourceVisitor.hpp"
#include "Graphics/FogParams.hpp"
#include "Graphics/TextureStage.hpp"
#include "Support/Log/DefaultLog.hpp"
#include "Support/Macro.h"


namespace amorphous
{


using namespace std;


const char *hr_d3d_error_to_string(HRESULT hr)
{
	switch(hr)
	{
	case D3DERR_DEVICELOST:          return "D3DERR_DEVICELOST";
	case D3DERR_DRIVERINTERNALERROR: return "D3DERR_DRIVERINTERNALERROR";
	case D3DERR_INVALIDCALL:         return "D3DERR_INVALIDCALL";
	case D3DERR_NOTAVAILABLE:        return "D3DERR_NOTAVAILABLE";
	case D3DERR_OUTOFVIDEOMEMORY:    return "D3DERR_OUTOFVIDEOMEMORY";
	case E_OUTOFMEMORY:              return "E_OUTOFMEMORY";
	case D3DXERR_INVALIDDATA:        return "D3DXERR_INVALIDDATA";
//	case : return "";
//	case : return "";
	default: return "Unknown";
	}

	return "Unknown";
}


static DWORD ToD3DTexStageOp( TexStageOp::Name op )
{
	switch( op )
	{
	case TexStageOp::SELECT_ARG0: return D3DTOP_SELECTARG1;
	case TexStageOp::SELECT_ARG1: return D3DTOP_SELECTARG2;
	case TexStageOp::MODULATE:    return D3DTOP_MODULATE;
	case TexStageOp::DISABLE:     return D3DTOP_DISABLE;
	default:
		return D3DTOP_DISABLE;
	}

}


static DWORD ToD3DTexStageArg( TexStageArg::Name tex_arg )
{
	switch( tex_arg )
	{
	case TexStageArg::PREV:    return D3DTA_CURRENT;
	case TexStageArg::DIFFUSE: return D3DTA_DIFFUSE;
	case TexStageArg::TEXTURE: return D3DTA_TEXTURE;
	default:
		return D3DTA_DIFFUSE;
	}
}


static const char *GetDeviceTypeString( D3DDEVTYPE dev_type )
{
	switch(dev_type)
	{
	case D3DDEVTYPE_HAL:     return "D3DDEVTYPE_HAL";
	case D3DDEVTYPE_NULLREF: return "D3DDEVTYPE_NULLREF";
	case D3DDEVTYPE_REF:     return "D3DDEVTYPE_REF";
	case D3DDEVTYPE_SW:      return "D3DDEVTYPE_SW";
	default: return "invalid";
	}
}

static const char *GetVertexProcessingTypeString( DWORD behavior_flags )
{
	if(behavior_flags & D3DCREATE_PUREDEVICE)                     return "Pure Device";
	else if(behavior_flags & D3DCREATE_HARDWARE_VERTEXPROCESSING) return "Hardware";
	else if(behavior_flags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) return "Software";
	else if(behavior_flags & D3DCREATE_MIXED_VERTEXPROCESSING)    return "Mixed";
	else return "unkonwn";
}



//========================================================================
// CDirect3D9
//========================================================================

// definition of singleton instance
CDirect3D9 CDirect3D9::ms_CDirect3D9_;


CDirect3D9::CDirect3D9()
:
m_pD3D( NULL ),
m_pD3DDevice( NULL ),
m_DeviceType(D3DDEVTYPE_HAL),
m_BehaviorFlags(0)
{
	m_CurrentDepthBufferType = D3DZB_TRUE;

	m_vecClipPlane.resize( 8 );
}


bool CDirect3D9::InitD3D( HWND hWnd, int iWindowWidth, int iWindowHeight, int screen_mode )
{
	if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		LOG_PRINT_ERROR( "Direct3DCreate9() failed." );
		return false;
	}

	// create D3D device

	D3DDISPLAYMODE d3ddm;
	if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
	{
		return false;
	}

	// save the surface format of the display mode
	m_AdapterFormat = d3ddm.Format;


	D3DPRESENT_PARAMETERS D3DPresentParam; 

	ZeroMemory( &D3DPresentParam, sizeof( D3DPresentParam ) );
	D3DPresentParam.BackBufferHeight				= iWindowHeight;
	D3DPresentParam.BackBufferWidth					= iWindowWidth;
	D3DPresentParam.BackBufferFormat				= d3ddm.Format;
//	D3DPresentParam.BackBufferFormat				= D3DFMT_A8R8G8B8;
	D3DPresentParam.BackBufferCount					= 1;
	D3DPresentParam.MultiSampleType					= D3DMULTISAMPLE_NONE;
//	D3DPresentParam.SwapEffect						= D3DSWAPEFFECT_COPY;	//original
	D3DPresentParam.SwapEffect						= D3DSWAPEFFECT_DISCARD;
	D3DPresentParam.hDeviceWindow					= hWnd;
	D3DPresentParam.Windowed						= ( screen_mode == WINDOWED ? TRUE : FALSE );
	D3DPresentParam.EnableAutoDepthStencil			= TRUE;
//	D3DPresentParam.AutoDepthStencilFormat			= D3DFMT_D16;
	D3DPresentParam.AutoDepthStencilFormat			= D3DFMT_D24S8;
	D3DPresentParam.Flags							= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	D3DPresentParam.FullScreen_RefreshRateInHz		= D3DPRESENT_RATE_DEFAULT;
//	D3DPresentParam.PresentationInterval			= D3DPRESENT_INTERVAL_DEFAULT;
	D3DPresentParam.PresentationInterval			= D3DPRESENT_INTERVAL_IMMEDIATE;

	// save as current present parameters
	m_CurrentPresentParameters = D3DPresentParam;

	// create D3D device

	bool d3d_device_created = CreateD3DDevice( D3DPresentParam, hWnd );
	if( !d3d_device_created )
		return false;

	// set up default render states
	SetDefaultRenderStates();

	// set up default camera matrix
	D3DXMATRIXA16 matView;
	D3DXVECTOR3 vEye( 0, 0, 3 );
	D3DXVECTOR3 vAt( 0, 0.7f, 0 );
	D3DXVECTOR3 vUp( 0, 1, 0 );
	D3DXMatrixLookAtLH( &matView, &vEye, &vAt, &vUp);

	m_pD3DDevice->SetTransform( D3DTS_VIEW,  &matView );

	float aspect_ratio
		= (float)m_CurrentPresentParameters.BackBufferWidth
		/ (float)m_CurrentPresentParameters.BackBufferHeight;

    // set up default projection matrix
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, aspect_ratio, 0.5f, 500.0f );
    m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	m_State = CGraphicsDevice::STATE_INITIALIZED;

	return true;
}


bool CDirect3D9::CreateD3DDevice( D3DPRESENT_PARAMETERS& present_params, HWND hWnd )
{
	SAFE_RELEASE( m_pD3DDevice );

	const int num_params_sets_to_try = 3;

	D3DDEVTYPE device_types[] =
	{
		D3DDEVTYPE_HAL, // most desirable
		D3DDEVTYPE_HAL,
		D3DDEVTYPE_REF  // least desirable
	};

	DWORD behavior_flags[] =
	{
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, // most desirable
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED  // least desirable
	};

	HRESULT hr;

	// Start with the most desirable settings
	for( int i=0; i<num_params_sets_to_try; i++ )
	{
		hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT,
		device_types[i],
		hWnd,
		behavior_flags[i],
		&present_params,
		&m_pD3DDevice );
		
		if( SUCCEEDED(hr) )
		{
			// save params
			m_DeviceType    = device_types[i];
			m_BehaviorFlags = behavior_flags[i];
			m_CurrentPresentParameters = present_params;

			LOG_PRINT( fmt_string( "Created a D3D device. device type: %s / vertex processing: %s",
			GetDeviceTypeString(m_DeviceType),
			GetVertexProcessingTypeString(m_BehaviorFlags) ) );

			// update adapter modes
			EnumAdapterModesForDefaultAdapter();

			return true;
		}
		else
		{
			LOG_PRINT_WARNING( fmt_string( "CreateDevice() failed with the next params: device type = %s, behavior flags = %s. Error: %s",
			GetDeviceTypeString(m_DeviceType),
			GetVertexProcessingTypeString(m_BehaviorFlags),
			hr_d3d_error_to_string(hr) ) );
		}
	}

	// LOG_PRINT_WARNING( " - Hardware vertex processing is not available." );

	return false;
}


bool CDirect3D9::ResetD3DDevice( HWND hWnd, int iWindowWidth, int iWindowHeight, bool bFullScreen )
{
	D3DPRESENT_PARAMETERS present_param = m_CurrentPresentParameters;

	// modify the current presentation parameters
	present_param.BackBufferHeight			= iWindowHeight;
	present_param.BackBufferWidth			= iWindowWidth;
	present_param.BackBufferCount			= 1;
	present_param.Windowed					= !bFullScreen;
	present_param.hDeviceWindow				= hWnd;
//	present_param.AutoDepthStencilFormat	= D3DFMT_D16;
	present_param.AutoDepthStencilFormat	= D3DFMT_D24S8;

//	if( !bFullScreen )
//		present_param.BackBufferFormat			= D3DFMT_UNKNOWN;

	// reset D3D device
//	HRESULT hr = m_pD3DDevice->Reset(&present_param);

	bool d3d_device_created = CreateD3DDevice( present_param, hWnd );
	if( !d3d_device_created )
		return false;

	// set up default render states
	SetDefaultRenderStates();

	return true;
}


void CDirect3D9::SetDefaultRenderStates()
{
	// some default render states
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING,         FALSE );
	m_pD3DDevice->SetRenderState( D3DRS_DITHERENABLE,     TRUE );
	m_pD3DDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
	m_pD3DDevice->SetRenderState( D3DRS_AMBIENT,          0x33333333 );
	m_pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
}


void CDirect3D9::Release()
{
	SAFE_RELEASE( m_pD3DDevice );
	SAFE_RELEASE( m_pD3D );
}


void CDirect3D9::GetAdapterModesForDefaultAdapter( std::vector<AdapterMode>& dest_buffer )
{
	if( !m_pD3D || !m_pD3DDevice )
		LOG_PRINT_ERROR( "Not initialized yet." );

	if( m_vecAdapterMode.size() == 0 )
		LOG_PRINT_WARNING( "No adapeter modes are in m_vecAdapterMode." );

	dest_buffer = m_vecAdapterMode;
}


bool CDirect3D9::IsCurrentDisplayMode( const DisplayMode& display_mode ) const
{
	const D3DPRESENT_PARAMETERS& present_params = m_CurrentPresentParameters;
	if( display_mode.Width       == present_params.BackBufferWidth
	 && display_mode.Height      == present_params.BackBufferHeight
	 && display_mode.Format      == FromD3DSurfaceFormat( present_params.BackBufferFormat ) )
//	 && display_mode.RefreshRate == present_params.FullScreen_RefreshRateInHz )
	{
		// What about refresh rates?
		return true;
	}
	else
	{
		return false;
	}
}


void CDirect3D9::EnumAdapterModesForDefaultAdapter()
{
	const D3DFORMAT allowable_formats[] =
	{
		D3DFMT_A1R5G5B5,
//		D3DFMT_A2R10G10B10,
		D3DFMT_A8R8G8B8,
		D3DFMT_R5G6B5,
		D3DFMT_X1R5G5B5,
		D3DFMT_X8R8G8B8
	};

	for(int i=0; i<numof(allowable_formats); i++)
	{
		m_vecAdapterMode.push_back( AdapterMode(FromD3DSurfaceFormat(allowable_formats[i])) );
		AdapterMode& adapter_mode = m_vecAdapterMode.back();

		uint num_adapter_modes = m_pD3D->GetAdapterModeCount( D3DADAPTER_DEFAULT, allowable_formats[i] );

		for(uint j=0;j<num_adapter_modes;j++)
		{
			D3DDISPLAYMODE mode;
			m_pD3D->EnumAdapterModes( D3DADAPTER_DEFAULT, allowable_formats[i], j, &mode );

			// add to the list
			adapter_mode.vecDisplayMode.push_back( DisplayMode(
			mode.Width,
			mode.Height,
			mode.RefreshRate,
			FromD3DSurfaceFormat(mode.Format) )
			);

			// check if this is the current display mode
			if( IsCurrentDisplayMode( adapter_mode.vecDisplayMode.back() ) )
				adapter_mode.vecDisplayMode.back().Current = true;
				
		}
	}
}


Result::Name CDirect3D9::SetTexture( int stage, const TextureHandle& texture )
{
	if( !m_pD3DDevice )
		return Result::UNKNOWN_ERROR;

	return SetTextureD3D_FFP( (uint)stage, texture );
}


Result::Name CDirect3D9::SetTextureStageParams( uint stage, const TextureStage& params )
{
//	LOG_PRINT_ERROR( " Not implemented yet." );
//	return Result::UNKNOWN_ERROR;

	HRESULT hr = S_OK;

	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   ToD3DTexStageOp(params.ColorOp) );
	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, ToD3DTexStageArg(params.ColorArg0) );
	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, ToD3DTexStageArg(params.ColorArg1) );

	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   ToD3DTexStageOp(params.AlphaOp) );
	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG1, ToD3DTexStageArg(params.AlphaArg0) );
	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, ToD3DTexStageArg(params.AlphaArg1) );

	return Result::SUCCESS;
}


Result::Name CDirect3D9::SetTextureTrasnformParams( uint stage, const CTextureTransformParams& params )
{
	HRESULT hr = S_OK;
	DWORD count_flags = 0;
	switch( params.NumElements )
	{
	case 2: count_flags = D3DTTFF_COUNT2; break;
	case 3: count_flags = D3DTTFF_COUNT3; break;
	default:
		break;
	}

	if( count_flags != 0 )
		hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );

	hr = m_pD3DDevice->SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);

	return Result::SUCCESS;
}


Result::Name CDirect3D9::SetTextureCoordTrasnform( uint stage, const Matrix44& transform )
{
	D3DTRANSFORMSTATETYPE ts = D3DTS_TEXTURE0;

	switch( stage )
	{
	case 0: ts = D3DTS_TEXTURE0; break;
	case 1: ts = D3DTS_TEXTURE1; break;
	case 2: ts = D3DTS_TEXTURE2; break;
	case 3: ts = D3DTS_TEXTURE3; break;
	case 4: ts = D3DTS_TEXTURE4; break;
	case 5: ts = D3DTS_TEXTURE5; break;
	case 6: ts = D3DTS_TEXTURE6; break;
	case 7: ts = D3DTS_TEXTURE7; break;
	default:
		ts = D3DTS_TEXTURE0;
		break;
	}

	D3DXMATRIX mat;
	transform.GetRowMajorMatrix44( (Scalar *)&mat );
	m_pD3DDevice->SetTransform( ts, &mat );

	return Result::SUCCESS;
}


static inline D3DCULL ToD3DCullMode( CullingMode::Name cull_mode )
{
	switch( cull_mode )
	{
	case CullingMode::CLOCKWISE:        return D3DCULL_CW;
	case CullingMode::COUNTERCLOCKWISE: return D3DCULL_CCW;
//	case CullingMode::NONE:             return D3DCULL_NONE;
	default: return D3DCULL_NONE;
	}
}


static inline bool ToD3DRenderStateType( RenderStateType::Name type, D3DRENDERSTATETYPE& dest )
{
	switch(type)
	{
	case RenderStateType::ALPHA_BLEND:               dest = D3DRS_ALPHABLENDENABLE;  return true;
	case RenderStateType::ALPHA_TEST:                dest = D3DRS_ALPHATESTENABLE;   return true;
	case RenderStateType::LIGHTING:                  dest = D3DRS_LIGHTING;		     return true;
	case RenderStateType::FOG:                       dest = D3DRS_FOGENABLE;	     return true;
	case RenderStateType::WRITING_INTO_DEPTH_BUFFER: dest = D3DRS_ZWRITEENABLE;	     return true;
	case RenderStateType::SCISSOR_TEST:              dest = D3DRS_SCISSORTESTENABLE; return true;
	default:
		return false;
	}
}


bool CDirect3D9::GetRenderState( RenderStateType::Name type )
{
	HRESULT hr = S_OK;
	D3DRENDERSTATETYPE d3d_rst;
	DWORD val = 0;
	bool res = ToD3DRenderStateType( type, d3d_rst );
	if( res )
	{
		hr = m_pD3DDevice->GetRenderState( d3d_rst, &val );
		if( SUCCEEDED(hr) )
			return (val == TRUE) ? true : false;
		else
			return false;
	}

	switch(type)
	{
	case RenderStateType::DEPTH_TEST:
		m_pD3DDevice->GetRenderState( D3DRS_ZENABLE, &val );
		return (val == D3DZB_FALSE) ? false : true;

	case RenderStateType::FACE_CULLING:
		m_pD3DDevice->GetRenderState( D3DRS_CULLMODE, &val );
		if( val == D3DCULL_NONE )
			int culling_is_off = 1;
		return (val == D3DCULL_NONE) ? false : true;

	default:
		return false;
	}

	return false;
}


Result::Name CDirect3D9::SetRenderState( RenderStateType::Name type, bool enable )
{
	if( !m_pD3DDevice )
		return Result::UNKNOWN_ERROR;

	D3DRENDERSTATETYPE d3d_rst;
	HRESULT hr = S_OK;
	bool res = ToD3DRenderStateType( type, d3d_rst );
	if( res )
	{
		hr = m_pD3DDevice->SetRenderState( d3d_rst, enable ? TRUE : FALSE );

		if( SUCCEEDED(hr) )
			return Result::SUCCESS;
		else
			return Result::UNKNOWN_ERROR;
	}

	switch(type)
	{
	case RenderStateType::DEPTH_TEST:
		if( enable )
			m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, m_CurrentDepthBufferType );
		else
			m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
		break;

	case RenderStateType::FACE_CULLING:
		if( enable )
			m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, ToD3DCullMode(m_CullMode) );
		else
			m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		break;

	default:
		return Result::INVALID_ARGS;
	}

	return Result::UNKNOWN_ERROR;
}


void CDirect3D9::SetSourceBlendMode( AlphaBlend::Mode src_blend_mode )
{
	if( m_pD3DDevice )
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, g_dwD3DBlendMode[src_blend_mode] );
}


void CDirect3D9::SetDestBlendMode( AlphaBlend::Mode dest_blend_mode )
{
	if( m_pD3DDevice )
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, g_dwD3DBlendMode[dest_blend_mode] );
}


static inline D3DCMPFUNC ToD3DAlphaFunc( CompareFunc::Name alpha_func )
{
	switch( alpha_func )
	{
	case CompareFunc::ALWAYS :                  return D3DCMP_ALWAYS;
	case CompareFunc::NEVER:                    return D3DCMP_NEVER;
	case CompareFunc::LESS_THAN:                return D3DCMP_LESS;
	case CompareFunc::LESS_THAN_OR_EQUAL_TO:    return D3DCMP_LESSEQUAL;
	case CompareFunc::GREATER_THAN:             return D3DCMP_GREATER;
	case CompareFunc::GREATER_THAN_OR_EQUAL_TO: return D3DCMP_GREATEREQUAL;
	default: return D3DCMP_ALWAYS;
	}
}


void CDirect3D9::SetAlphaFunction( CompareFunc::Name alpha_func )
{
	if( m_pD3DDevice )
		m_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, ToD3DAlphaFunc(alpha_func) );
}


void CDirect3D9::SetReferenceAlphaValue( float ref_alpha )
{
//	DWORD ref = get_clamped( ref_alpha, 0.0f, 0.1f );
	if( m_pD3DDevice )
		m_pD3DDevice->SetRenderState( D3DRS_ALPHAREF, (DWORD)get_clamped( ref_alpha, 0.0f, 0.1f ) );
}


static inline D3DFOGMODE ToD3DFogMode( FogMode::Name fog_mode )
{
	switch( fog_mode )
	{
	case FogMode::LINEAR: return D3DFOG_LINEAR;
	case FogMode::EXP:    return D3DFOG_EXP;
	case FogMode::EXP2:   return D3DFOG_EXP2;
	default: return D3DFOG_LINEAR;
	}
}


Result::Name CDirect3D9::SetFogParams( const FogParams& fog_params )
{
	HRESULT hr = S_OK;

	hr = m_pD3DDevice->SetRenderState( D3DRS_FOGTABLEMODE, ToD3DFogMode(fog_params.Mode) );

	D3DCOLOR argb32 = fog_params.Color.GetARGB32();
	hr = m_pD3DDevice->SetRenderState( D3DRS_FOGCOLOR,   argb32 );

	float start = (float)fog_params.Start; 
	hr = m_pD3DDevice->SetRenderState( D3DRS_FOGSTART,   *(DWORD *)&start );

	float end = (float)fog_params.End;
	hr = m_pD3DDevice->SetRenderState( D3DRS_FOGEND,     *(DWORD *)&end );

	float density = (float)fog_params.Density;
	hr = m_pD3DDevice->SetRenderState( D3DRS_FOGDENSITY, *(DWORD *)&density );

	// Always use pixel fog and do not change params
//	hr = m_pD3DDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_NONE );
//	hr = m_pD3DDevice->SetRenderState( D3DRS_RANGEFOGENABLE, FALSE );

	return Result::SUCCESS;
}


Result::Name CDirect3D9::SetCullingMode( CullingMode::Name cull_mode )
{
	m_CullMode = cull_mode;

	HRESULT hr = S_OK;

	DWORD current_d3d_cull_mode = D3DCULL_NONE;
	hr = m_pD3DDevice->GetRenderState( D3DRS_CULLMODE, &current_d3d_cull_mode );
	D3DCULL dest_d3d_cull_mode = ToD3DCullMode(cull_mode);
	if( current_d3d_cull_mode != dest_d3d_cull_mode )
	{
		if( dest_d3d_cull_mode == D3DCULL_NONE )
			int setting_culling_mode_to_D3DCULL_NONE = 1;
		hr = m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, dest_d3d_cull_mode );
	}

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::GetViewport( Viewport& viewport )
{
	D3DVIEWPORT9 vp;
	HRESULT hr = m_pD3DDevice->GetViewport( &vp );
	viewport.UpperLeftX = (uint)vp.X;
	viewport.UpperLeftY = (uint)vp.Y;
	viewport.Width      = (uint)vp.Width;
	viewport.Height     = (uint)vp.Height;
	viewport.MinDepth   = vp.MinZ;
	viewport.MaxDepth   = vp.MaxZ;

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::SetViewport( const Viewport& viewport )
{
	D3DVIEWPORT9 vp;
	vp.X      = (DWORD)viewport.UpperLeftX;
	vp.Y      = (DWORD)viewport.UpperLeftY;
	vp.Width  = (DWORD)viewport.Width;
	vp.Height = (DWORD)viewport.Height;
	vp.MinZ   = viewport.MinDepth;
	vp.MaxZ   = viewport.MaxDepth;
	HRESULT hr = m_pD3DDevice->SetViewport( &vp );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::SetClearColor( const SFloatRGBAColor& color )
{
	m_ClearColor = color.GetARGB32();
	return Result::SUCCESS;
}


Result::Name CDirect3D9::SetClearDepth( float depth )
{
	m_fClearDepth = depth;
	return Result::SUCCESS;
}


Result::Name CDirect3D9::Clear( U32 buffer_mask )
{
	DWORD mask = 0;
	if( buffer_mask & BufferMask::COLOR )   mask |= D3DCLEAR_TARGET;
	if( buffer_mask & BufferMask::DEPTH )   mask |= D3DCLEAR_ZBUFFER;
	if( buffer_mask & BufferMask::STENCIL ) mask |= D3DCLEAR_STENCIL;

	HRESULT hr = m_pD3DDevice->Clear(
		0,
		NULL,
		mask,
		m_ClearColor,
		m_fClearDepth,
		0 );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::BeginScene()
{
	HRESULT hr = m_pD3DDevice->BeginScene();
	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::EndScene()
{
	HRESULT hr = m_pD3DDevice->EndScene();
	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::Present()
{
	HRESULT hr = m_pD3DDevice->Present( NULL, NULL, NULL, NULL );
	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::SetClipPlane( uint index, const Plane& clip_plane )
{
	if( (uint)m_vecClipPlane.size() <= index )
		return Result::INVALID_ARGS;

	m_vecClipPlane[index] = clip_plane;

//	float coefficients[4] =
//	{
//		clip_plane.normal.x,
//		clip_plane.normal.y,
//		clip_plane.normal.z,
//		clip_plane.dist * (-1.0f)
//	};

//	HRESULT hr = m_pD3DDevice->SetClipPlane( index, coefficients );

//	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;

	return Result::SUCCESS;
}


Result::Name CDirect3D9::EnableClipPlane( uint index )
{
	HRESULT hr = S_OK;
	DWORD current_flags = 0;
	hr = m_pD3DDevice->GetRenderState( D3DRS_CLIPPLANEENABLE, &current_flags );
	hr = m_pD3DDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, current_flags | (1 << index) );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::DisableClipPlane( uint index )
{
	HRESULT hr = S_OK;
	DWORD current_flags = 0;
	hr = m_pD3DDevice->GetRenderState( D3DRS_CLIPPLANEENABLE, &current_flags );

	DWORD mask = ( ~(1 << index) );
	DWORD flags = current_flags & ( ~(1 << index) );
	hr = m_pD3DDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, flags );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::UpdateViewProjectionTransformsForClipPlane( uint index, const Matrix44& view_transform, const Matrix44& proj_transform )
{
	if( (uint)m_vecClipPlane.size() <= index )
		return Result::INVALID_ARGS;

	const Plane clip_plane = m_vecClipPlane[index];

	D3DXPLANE clipPlane;
	D3DXVECTOR3 point = ToD3DXVECTOR3( clip_plane.normal * clip_plane.dist );// point( 0 , 0, 0 );
	D3DXVECTOR3 normal = ToD3DXVECTOR3( clip_plane.normal * (-1.0f) );//normal( 0, -1, 0 ); // negative of the normal vector.
	// create and normalize the plane
	D3DXPlaneFromPointNormal(&clipPlane,&point,&normal);
	D3DXPlaneNormalize(&clipPlane,&clipPlane);

	// To transform a plane from world space to view space there is a methode D3DXPlaneTransform
	// but the peculiar thing about this method is that it takes the inverse transpose of the viewprojection matrix

	Matrix44 proj_view = proj_transform * view_transform;
	D3DXMATRIXA16 matrix, view_proj;
	proj_view.GetRowMajorMatrix44( (float *)&view_proj );

	D3DXMatrixInverse(&matrix, NULL, &view_proj); // second parameter is an out parameter for the determinant
	D3DXMatrixTranspose(&matrix, &matrix);

	D3DXPLANE viewSpacePlane;
	D3DXPlaneTransform(&viewSpacePlane, &clipPlane, &matrix);

	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();
	HRESULT hr = S_OK;
	hr = pd3dDev->SetClipPlane( 0, (float *)&viewSpacePlane );
	hr = pd3dDev->SetRenderState( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::SetScissorRect( const SRect& rect )
{
	HRESULT hr = S_OK;
	RECT dest;
	dest.left   = rect.left;
	dest.top    = rect.top;
	dest.right  = rect.right;
	dest.bottom = rect.bottom;

	hr = m_pD3DDevice->SetScissorRect( &dest );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


Result::Name CDirect3D9::SetSamplingParameter( uint sampler_index, SamplingParameter::Name param, uint value )
{
	HRESULT hr = S_OK;


	D3DSAMPLERSTATETYPE dest_type = D3DSAMP_MIPFILTER;

	switch(param)
	{
	case SamplingParameter::TEXTURE_WRAP_AXIS_0: dest_type = D3DSAMP_ADDRESSU;  break;
	case SamplingParameter::TEXTURE_WRAP_AXIS_1: dest_type = D3DSAMP_ADDRESSV;  break;
	case SamplingParameter::TEXTURE_WRAP_AXIS_2: dest_type = D3DSAMP_ADDRESSW;  break;
	case SamplingParameter::MIN_FILTER:          dest_type = D3DSAMP_MINFILTER; break;
	case SamplingParameter::MAG_FILTER:          dest_type = D3DSAMP_MAGFILTER; break;
	default:
		LOG_PRINTF_ERROR(( " An unsupported sampler parameter (%d)", (int)param ));
		break;
	}

	DWORD dest_value = 0;
	if( dest_type == D3DSAMP_MINFILTER
	 || dest_type == D3DSAMP_MAGFILTER )
	{
		switch(value)
		{
		case TextureFilter::NEAREST:   dest_value = D3DTEXF_POINT;   break;
		case TextureFilter::LINEAR:    dest_value = D3DTEXF_LINEAR; break;
		default:
			LOG_PRINTF_ERROR(( " An unsupported texture filter mode (%d)", (int)value ));
			break;
		}
	}
	else if(
	    dest_type == D3DSAMP_ADDRESSU
	 || dest_type == D3DSAMP_ADDRESSV
	 || dest_type == D3DSAMP_ADDRESSW )
	{
		switch(value)
		{
		case TextureAddressMode::REPEAT:          dest_value = D3DTADDRESS_WRAP;   break;
		case TextureAddressMode::MIRRORED_REPEAT: dest_value = D3DTADDRESS_MIRROR; break;
		case TextureAddressMode::CLAMP_TO_BORDER: dest_value = D3DTADDRESS_BORDER; break;
		case TextureAddressMode::CLAMP_TO_EDGE:   dest_value = D3DTADDRESS_CLAMP;  break;
		default:
			LOG_PRINTF_ERROR(( " An unsupported texture address mode (%d)", (int)value ));
			break;
		}
	}
	
	hr = m_pD3DDevice->SetSamplerState( sampler_index, dest_type, dest_value );

	return SUCCEEDED(hr) ? Result::SUCCESS : Result::UNKNOWN_ERROR;
}


} // namespace amorphous
