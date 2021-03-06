#ifndef __LWO2_LAYER_H__
#define __LWO2_LAYER_H__


#include <vector>
#include <list>
#include <string>
#include "fwd.hpp"
#include "LWO2_Common.hpp"
#include "amorphous/3DMath/Vector3.hpp"
#include "amorphous/Graphics/FloatRGBAColor.hpp"
#include "amorphous/Support/memory_helpers.hpp"




namespace amorphous
{


// TODO: support multiple PNTS / POLS chunks in a layer


#define LWO2_INVALID_POLYGON_INDEX	-1


/**
 * holds an index to the most recent PNTS chunk and the corresponding uv coordinates (u,v)
 */
struct SIndexAndUV
{
	/// index to the most recent PNTS chunk
	int i;

	/// index to a polygon in the most recent POLS chunk? ( used in VMAD chunk )
	int PolygonIndex;

	float u;
	float v;

	SIndexAndUV() : i(0), PolygonIndex(LWO2_INVALID_POLYGON_INDEX), u(0), v(0) {}

	bool operator<( const SIndexAndUV& iuv ) { return ( this->i < iuv.i ); };
};



//=========================================================================================
// SVertexColor_LWO2
//=========================================================================================

/*
 * a pair of index and ARGB color for a vertex
 */
struct SVertexColor_LWO2
{
	/// index to a point in the most recent PNTS chunk
	int iIndex;

    /// index to a polygon in the most recent POLS chunk? ( used in VMAD chunk )
	int iPolygonIndex;

	float fAlpha, fRed, fGreen, fBlue;

	SVertexColor_LWO2() { iIndex = 0; iPolygonIndex = LWO2_INVALID_POLYGON_INDEX; fAlpha = fRed = fGreen = fBlue = 0.0f; }

	bool operator<( const SVertexColor_LWO2& vert_color_map ) { return ( this->iIndex < vert_color_map.iIndex ); };


	/// return value has the same format as D3DCOLOR
	UINT4 GetColor_UINT4_ARGB();
};


//=========================================================================================
// LWO2_VertexColorMap
//=========================================================================================

class LWO2_VertexColorMap
{
public:

	std::string strName;

	int iNumIndices;

	SVertexColor_LWO2 *paVertexColor;

public:

	LWO2_VertexColorMap() : iNumIndices(0), paVertexColor(NULL) {}

	~LWO2_VertexColorMap() { Release(); }

	void Release() { iNumIndices = 0; SafeDelete(paVertexColor); }

	std::string& GetName() { return strName; }

	inline bool FindVertexColor( UINT4 pnt_index, SFloatRGBAColor& rDestColor );

	LWO2_VertexColorMap(const LWO2_VertexColorMap& vc_map);
	LWO2_VertexColorMap operator=(LWO2_VertexColorMap vc_map);
};



//==================================================================================
// LWO2_TextureUVMap
//==================================================================================

class LWO2_TextureUVMap
{
public:

	std::string strName;

	std::vector<SIndexAndUV> vecIndexUV;

	LWO2_TextureUVMap() {}

	~LWO2_TextureUVMap(){}

};



//==================================================================================
// LWO2_PointSelectionSet
//==================================================================================

class LWO2_PointSelectionSet
{
	std::string m_strName;
	std::vector<UINT4> m_vecPointIndex;

public:

	LWO2_PointSelectionSet() {}

	const std::string& GetName() const { return m_strName; }

	std::vector<UINT4>& GetPointIndex() { return m_vecPointIndex; }

	int GetNumPoints() const { return (int)m_vecPointIndex.size(); }

	void SetName( const char *pcName ) { m_strName = pcName; }

	friend class LWO2_Layer;
};


/**
 *   stores polygon info
 */
class LWO2_Face
{
	std::vector<UINT4> m_vecPointIndex;

	int m_iSurfaceIndex;

	int m_iPartIndex;

	Vector3 m_vFaceNormal;

public:

	LWO2_Face(){ m_iSurfaceIndex = 0; m_iPartIndex = -1; m_vFaceNormal = Vector3(0,0,0); }

//	LWO2_Face(const LWO2_Face& face);
	~LWO2_Face() {}

	inline int GetNumPoints() const { return (int)m_vecPointIndex.size(); }

	inline const std::vector<UINT4>& GetVertexIndex() const { return m_vecPointIndex; }

	inline UINT4 GetVertexIndex( int i ) const { return m_vecPointIndex[i]; }

	inline Vector3 GetFaceNormal() const { return m_vFaceNormal; }

	inline int GetSurfaceIndex() const { return m_iSurfaceIndex; }

	inline int GetPartIndex() const { return m_iPartIndex; }

	friend class LWO2_Layer;
};


// represents named groups of polygons (Parts)
class LWO2_PolygonGroup
{
public:

//	std::string m_strName;

	/// indices to the faces in the most recent POLS chunk
	std::vector<UINT4> m_vecPolygonIndex;

	/// tag indices
	std::vector<UINT2> m_vecTag;

//	LWO2_PolygonGroup() {}
//	string& GetName() { return m_strName; }
	friend class LWO2_Layer;
};



class LWO2_WeightMap
{
	std::string m_strName;

	std::vector<int> m_vecPntIndex;
	std::vector<float> m_vecfWeight;

public:

	const std::string& GetName() const { return m_strName; }

	int GetNumMaps() const { return (int)m_vecPntIndex.size(); }

	inline void GetWeightMap( int index, UINT4& iPntIndex, float& fWeight );

	/// finds a weight value for a vertex. returns true if the weight is found
	inline bool FindWeight( UINT4 pnt_index, float& rfDestWeight );

	friend class LWO2_Layer;
};


class LWO2_Bone
{
	std::string m_strName;
	UINT4 m_aiPointIndex[2];

public:

	const std::string& GetName() const { return m_strName; }
	UINT4 GetVertexIndex( int i ) const { return m_aiPointIndex[i]; }

	friend class LWO2_Layer;
};


class LWO2_BoneWeightMap
{
public:
	UINT4 iBoneIndex;
	UINT4 iWeightMapTagIndex;
};


//==================================================================================
// LWO2_Layer
//==================================================================================

class LWO2_Layer
{
	int m_iLayerIndex;

	std::string m_strLayerName;

	/// buffer to store vertices
	std::vector<Vector3> m_vecPoint;

	std::vector<LWO2_Face> m_vecFace;

	std::vector<LWO2_TextureUVMap> m_vecTexuvmap;

	std::vector<LWO2_TextureUVMap> m_vecTexVMAD;

	std::vector<LWO2_VertexColorMap> m_vecVertexColorMap;

	std::vector<LWO2_PointSelectionSet> m_vecPointSelectionSet;

	std::vector<LWO2_PolygonGroup> m_vecPolygonGroup;

	std::vector<LWO2_WeightMap> m_vecVertexWeightMap;

	std::vector<LWO2_Bone> m_vecBone;

	/// binds bone indices and tag indices of the correspnding weight maps
	std::vector<LWO2_BoneWeightMap> m_vecBoneWeightMap;

	std::vector<Vector3> m_vecVertexNormal;

public:

	LWO2_Layer() { m_iLayerIndex = 0; }

//	LWO2_Layer(const LWO2_Layer& layer);

	~LWO2_Layer() {}

	void ReadLayerChunk(UINT4& chunksize, FILE* fp);

	void ReadVertices(UINT4& chunksize, FILE* fp);

	void ReadVertexMap(UINT4& chunksize, FILE* fp);

	void ReadVMADChunk(UINT4& chunksize, FILE* fp);

	UINT4 ReadPols(UINT4& chunksize, FILE* fp);

	void ReadPTAG(UINT4& ptagsize, LWO2_Object& rObject, FILE* fp);

	bool GetUV( float& u, float& v, int iPointIndex, const LWO2_TextureUVMap *pTexUVMap );

	Vector3 GetInterpolatedNormal( LWO2_Face& rFace, int iPntIndex );

	void ComputeFaceNormals();

	void ComputeVertexNormals();


	const std::string& GetName() const { return m_strLayerName; }

	int GetLayerIndex() const { return m_iLayerIndex; }

	std::vector<Vector3>& GetVertex() { return m_vecPoint; }

	const std::vector<Vector3>& GetVertex() const { return m_vecPoint; }

	const std::vector<Vector3>& GetVertexNormal() const { return m_vecVertexNormal; }

	std::vector<Vector3>& GetVertexNormal() { return m_vecVertexNormal; }

	const std::vector<LWO2_Face>& GetFace() const { return m_vecFace; }

	std::vector<LWO2_Face>& GetFace() { return m_vecFace; }

	std::vector<LWO2_PolygonGroup>& GetPolygonGroup() { return m_vecPolygonGroup; }

	const std::vector<LWO2_PolygonGroup>& GetPolygonGroup() const { return m_vecPolygonGroup; }

	const std::string& GetPolygonGroupName();

	std::vector<LWO2_TextureUVMap>& GetTextureUVMap() { return m_vecTexuvmap; }

	const std::vector<LWO2_TextureUVMap>& GetTextureUVMap() const { return m_vecTexuvmap; }

	std::vector<LWO2_VertexColorMap>& GetVertexColorMap() { return m_vecVertexColorMap; }

	const std::vector<LWO2_VertexColorMap>& GetVertexColorMap() const { return m_vecVertexColorMap; }

	std::vector<LWO2_WeightMap>& GetVertexWeightMap() { return m_vecVertexWeightMap; }

	const std::vector<LWO2_WeightMap>& GetVertexWeightMap() const { return m_vecVertexWeightMap; }

	std::vector<LWO2_PointSelectionSet>& GetPointSelectionSet() { return m_vecPointSelectionSet; }

	bool GetVertexColor( SFloatRGBAColor& color, const int iPntIndex, const LWO2_Surface& rSurf );

	const std::vector<LWO2_Bone>& GetBone() const { return m_vecBone; }

	std::vector<LWO2_Bone>& GetBone() { return m_vecBone; }

//	std::vector<Vector3>& GetVertexNormal() { return m_vecVertexNormal; }

	const std::vector<LWO2_BoneWeightMap>& GetBoneWeightMap() const { return m_vecBoneWeightMap; }

	bool operator==(LWO2_Layer& layer){ return ( m_iLayerIndex == layer.m_iLayerIndex ); }
	bool operator<(LWO2_Layer& layer){return ( m_iLayerIndex < layer.m_iLayerIndex );}

};


// --------------------- inline implementations ---------------------

inline void LWO2_WeightMap::GetWeightMap( int index, UINT4& iPntIndex, float& fWeight )
{
	iPntIndex = m_vecPntIndex[index];
	fWeight = m_vecfWeight[index];
}


inline bool LWO2_VertexColorMap::FindVertexColor( UINT4 pnt_index, SFloatRGBAColor& rDestColor )
{
	int i, num_maps = iNumIndices;	//GetNumMaps();
	for( i=0; i<num_maps; i++ )
	{
		if( paVertexColor[i].iIndex == pnt_index )
		{
			// found the corresponding color for the vertex
			rDestColor.SetRGBA( paVertexColor[i].fRed,
				                paVertexColor[i].fGreen,
								paVertexColor[i].fBlue,
								paVertexColor[i].fAlpha );
			return true;
		}
	}

	return false;
}


inline bool LWO2_WeightMap::FindWeight( UINT4 pnt_index, float& rfDestWeight )
{
	int i, num_maps = GetNumMaps();
	for( i=0; i<num_maps; i++ )
	{
		if( m_vecPntIndex[i] == pnt_index )
		{
			// found the weight value for 'pnt_index'
			rfDestWeight = m_vecfWeight[i];
			return true;
		}
	}

	return false;
}


} // amorphous



#endif  /*  __LWO2_LAYER_H__  */
