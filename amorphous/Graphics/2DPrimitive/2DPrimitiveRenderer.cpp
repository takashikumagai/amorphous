#include "2DPrimitiveRenderer.hpp"


namespace amorphous
{


class VFF
{
public:

	enum FlagName
	{
		VF_POSITION			= (1 << 0),
		VF_NORMAL			= (1 << 1),
		VF_DIFFUSE_COLOR	= (1 << 2),
		VF_2D_TEXCOORD		= (1 << 3),
		VF_BUMPMAP			= (1 << 4), ///< usually requires tangent and binormal vector for each vertex. VF_NORMAL must be raised when the flag
		VF_WEIGHT			= (1 << 5),
		VF_2D_TEXCOORD0		= VF_2D_TEXCOORD,
		VF_2D_TEXCOORD1		= (1 << 6),
		VF_2D_TEXCOORD2		= (1 << 7),
		VF_2D_TEXCOORD3		= (1 << 8),
	};

	enum FlagSet
	{
		VF_COLORVERTEX		= VF_POSITION|VF_NORMAL|VF_DIFFUSE_COLOR,
		VF_TEXTUREVERTEX	= VF_POSITION|VF_NORMAL|VF_DIFFUSE_COLOR|VF_2D_TEXCOORD,
		VF_WEIGHTVERTEX		= VF_POSITION|VF_NORMAL|VF_DIFFUSE_COLOR|VF_2D_TEXCOORD|VF_WEIGHT,
		VF_BUMPVERTEX		= VF_POSITION|VF_NORMAL|VF_DIFFUSE_COLOR|VF_2D_TEXCOORD|VF_BUMPMAP,
		VF_BUMPWEIGHTVERTEX	= VF_POSITION|VF_NORMAL|VF_DIFFUSE_COLOR|VF_2D_TEXCOORD|VF_BUMPMAP|VF_WEIGHT,
		VF_SHADOWVERTEX		= VF_POSITION|VF_NORMAL,
		VF_SHADOWWEIGHTVERTEX	= VF_POSITION|VF_NORMAL|VF_WEIGHT
	};
};


} // namespace amorphous