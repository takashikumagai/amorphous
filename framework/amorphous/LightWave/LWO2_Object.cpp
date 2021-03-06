#include "LWO2_Object.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"


namespace amorphous
{

using namespace std;


//================================================================================
// LWO2_TAGChunk::Methods()                                     - LWO2_TAGChunk
//================================================================================

LWO2_TAGChunk::LWO2_TAGChunk(const LWO2_TAGChunk& tagchunk)
{
	this->tagchunk_size = tagchunk.tagchunk_size;
	this->pTAGStrings = new char [tagchunk_size];
	memcpy(this->pTAGStrings, tagchunk.pTAGStrings, tagchunk_size);
	this->iNumTAGs = tagchunk.iNumTAGs;
	this->piIndex = new int [iNumTAGs];
	memcpy(this->piIndex, tagchunk.piIndex, iNumTAGs * sizeof(int));
}


void LWO2_TAGChunk::AllocateTAGStrings(UINT4 tagchunksize, FILE* fp)
{
	char *p;
	int temp_indices[1024];

	this->tagchunk_size = tagchunksize;

	vector<string> m_vecstrTag;

	// Copy the content of the TAGS chunk
	pTAGStrings = new char [tagchunksize];
	fread(pTAGStrings, sizeof(char), tagchunksize, fp);
	p = pTAGStrings;
	int i = 0;
	UINT4 n = 0;

	// Set Indices to each tag
	while(n < tagchunksize)
	{

		m_vecstrTag.push_back( string() );
		m_vecstrTag.back() = p + n;


		temp_indices[i] = n;
		n += (int)strlen(p+n);		//pTAGStrings[n] indicates the terminating NULL character
		if(n % 2 == 0)  //We got the double NULL at the end of this tag
			n++;
		n++;
		i++;

	}
	this->iNumTAGs = i;
	piIndex = new int [i];
	memcpy(piIndex, temp_indices, sizeof(int) * i);
	return;
}


//================================================================================
// LWO2_StillClip::Methods()                                   - LWO2_StillClip
//================================================================================

// only supports STIL subchunk
void LWO2_StillClip::Read(UINT4 clipsize, FILE* fp)
{
	UINT4 uiType, uiClipID;
	UINT4 bytesread, bytesleft;
	UINT2 wRead;
	char temp[512];
/*
	uiClipID = ReadBE4BytesIntoLE(fp);  //The index that identifies this clip uniquely
	uiType = ReadBE4BytesIntoLE(fp);  //type of this CLIP chunk
	this->uiClipIndex = uiClipID;
	bytesread = 8;
	bytesleft = clipsize - bytesread;

	switch(uiType)
	{
	case ID_STIL:
		wRead = ReadBE2BytesIntoLE(fp);
		bytesread +=2;
		fread(temp ,sizeof(char), wRead, fp);
		strName = temp;
		return;

	default:
		AdvanceFP(fp, bytesleft);
		break;
	}
*/
	uiClipID = ReadBE4BytesIntoLE(fp);  // The index that identifies this clip uniquely
	this->uiClipIndex = uiClipID;
	bytesread = 4;
	bytesleft = clipsize - 4;
	while( 0 < bytesleft )
	{
		uiType = ReadBE4BytesIntoLE(fp);  // a CLIP sub-chunk
		bytesleft -= 4;

		switch(uiType)
		{
		case ID_STIL:
			wRead = ReadBE2BytesIntoLE(fp);
			bytesread +=2;
			fread(temp ,sizeof(char), wRead, fp);
			strName = temp;
			bytesleft -= (2+wRead);
			break;

		case ID_FLAG:
			AdvanceFP(fp, 6);
			bytesleft -= 6;
			break;

		default:
			AdvanceFP(fp, bytesleft);
			break;
		}
	}
}



//================================================================================
// LWO2_Object::Methods()                                         - LWO2_Object
//================================================================================

bool LWO2_Object::LoadLWO2Object( const std::string& object_filename )
{
	UINT4 chunksize, bytesread = 0, datasize = 0;
	UINT4 uiRead;
	UINT4 uiTypeLastPOLS;
	UINT4 uiPrevType;

	LWO2_Surface surf;
	LWO2_StillClip clip;
	LWO2_Layer new_layer;

	Vector3 vMin, vMax;

	FILE* fp = fopen( object_filename.c_str(), "rb" );  //binary-reading mode
	if(fp==NULL)
	{
		LOG_PRINT_ERROR( "The specified LWO file '" + object_filename +"' was not found." );
		return false;
	}

	// The first 4 bytes of the "*.lwo" file
	uiRead = ReadBE4BytesIntoLE(fp);
	if(uiRead != ID_FORM)
	{
		LOG_PRINT_ERROR( object_filename + " is not an IFF file (Missing FORM tag)." );
		return false;
	}

	// Next 4 bytes indicate the datasize of the LWO file.
	// Here, the datasize means the size of the file excepting the first 8 bytes, FORM and datasize itself
	uiRead = ReadBE4BytesIntoLE(fp);
	datasize = uiRead;

	// Next 4 bytes, from 8 to 11 indicates the format.
	// This 4 bytes have to be LWO2
	uiRead = ReadBE4BytesIntoLE(fp);
	bytesread += 4;
	if(uiRead != ID_LWO2)
	{
		LOG_PRINT_ERROR( object_filename + " does not have LWO2 format (Missing LWO2 tag)." );
		return false;
	}

	// save the original filename
	m_strFilename = object_filename;

	m_ProgressDisplay.set_total_units( (int)(datasize) );

	while(bytesread < datasize)
	{
		// read a primary chunk ID
		uiRead = ReadBE4BytesIntoLE(fp);

		// next, read the size of this chunk
		chunksize = ReadBE4BytesIntoLE(fp);

		// these 2 tags accounts for 4 * 2 = 8 bytes
		bytesread += 8;

		switch(uiRead)
		{
		case ID_TAGS:
			this->m_tag.AllocateTAGStrings(chunksize, fp);
			break;

		case ID_LAYR:
			this->m_lstLayer.push_back(new_layer);
			this->m_lstLayer.back().ReadLayerChunk(chunksize, fp);
			break;

		case ID_PNTS:
			m_lstLayer.back().ReadVertices(chunksize, fp);
			break;

		case ID_POLS:
			uiTypeLastPOLS = m_lstLayer.back().ReadPols(chunksize, fp);
			break;

		case ID_PTAG:
			m_lstLayer.back().ReadPTAG(chunksize, *this, fp);	// set surface indices to faces
			break;

		case ID_VMAP:  //There are as many VMAP chunks as the number of surfaces in the LWO file
			//Store the indices to vertices & the indices to uvs in UVMap[]
			//and store the actual uv data in pTextureUV[].
			m_lstLayer.back().ReadVertexMap(chunksize, fp);
			break;

		case ID_VMAD:
//			m_lstLayer.back().ReadVMAD(chunksize, PolUVMap[numduvmaps++], pFaces, pTextureUV, numTextureUVs, fp);
			m_lstLayer.back().ReadVMADChunk(chunksize, fp);
			break;

		case ID_SURF:  //There are as many SURF chunks as the number of the surfaces in the LWO file
//			surf.ReadOneSurface(chunksize, fp);
//			this->m_vecSurface.push_back(surf);
			m_vecSurface.push_back( LWO2_Surface() );
			m_vecSurface.back().ReadOneSurface( chunksize, fp );
			break;

		case ID_CLIP:
			clip.Read(chunksize, fp);
			this->m_vecStillClip.push_back(clip);
			break;

		case ID_BBOX:
			fread( &vMin, sizeof(Vector3), 1, fp );
			fread( &vMax, sizeof(Vector3), 1, fp );
//			AdvanceFP(fp, chunksize);
			break;

		default:
			AdvanceFP(fp, chunksize);
			break;
		}
		bytesread += chunksize;

		m_ProgressDisplay.set_current_units( bytesread );

		if( m_ProgressDisplay.get_total_units() < m_ProgressDisplay.get_current_units() )
			LOG_PRINT_ERROR( " Something is wrong with the progress display: total_units < current_units." );

		uiPrevType = uiRead;
	}

	fclose(fp);


	// compute normals on each face(polygon) in the LightWave object 
	ComputeFaceNormals();
	ComputeVertexNormals();

	return true;

}


void LWO2_Object::WriteDebug( const std::string& filename ) const
{
	FILE* fp;
	fp = fopen(filename.c_str(), "w");
	if( !fp )
		return;

	size_t i, num_tags = m_tag.iNumTAGs;
	for(i=0; i<num_tags; i++)
	{
		fprintf( fp, "%s\n", m_tag.pTAGStrings + m_tag.piIndex[i] );
	}

	fprintf( fp,"\n----- %d Layers -----\n", (int)m_lstLayer.size() );

	list<LWO2_Layer>::const_iterator p;
	for(p = m_lstLayer.begin(); p!=m_lstLayer.end(); p++)
	{
		fprintf( fp, "[%02d] %s\n", p->GetLayerIndex(), p->GetName().c_str() );
		fprintf( fp, "%d points\n", (int)p->GetVertex().size() );
		fprintf( fp, "%d faces\n",  (int)p->GetFace().size() );
	}


	fprintf( fp, "\n----- %d Surfaces -----\n", (int)m_vecSurface.size() );

	size_t iNumSurfs = m_vecSurface.size();
	for( i=0; i<iNumSurfs; i++ )
	{
		const LWO2_Surface& rSurf = m_vecSurface[i];

		fprintf( fp, "name: %s\n", rSurf.GetName().c_str() );
		fprintf( fp, "texture uv: %s\n", "???"/*rSurf.GetUVMapName().c_str()*/ );

	}

	fprintf( fp, "\nCLIP - STIL\n" );

	size_t iNumClips = m_vecStillClip.size();
	for( i=0; i<iNumClips; i++ )
	{
		const LWO2_StillClip& clip = m_vecStillClip[i];
		fprintf( fp, " [%02d] %s\n", clip.uiClipIndex, clip.strName.c_str() );
	}

	fclose(fp);
}

/*
LWO2_Object::LWO2_Object(const LWO2_Object& lwo2data){
	this->m_lstLayer.assign(lwo2data.m_lstLayer.begin(), lwo2data.m_lstLayer.end());
	this->m_vecSurface.assign(lwo2data.m_vecSurface.begin(), lwo2data.m_vecSurface.end());
	this->m_vecStillClip.assign(lwo2data.m_vecStillClip.begin(), lwo2data.m_vecStillClip.end());
}

LWO2_Object::~LWO2_Object(){
	m_lstLayer.clear();
	m_vecSurface.clear();
	m_vecStillClip.clear();
}*/


LWO2_Surface *LWO2_Object::FindSurfaceFromTAG( UINT2 wSurfIndex )
{
	int surf_index = GetSurfaceIndexFromSurfaceTAG( wSurfIndex );

	if( 0 <= surf_index
	 && surf_index < (int)m_vecSurface.size() )
	{
		return &m_vecSurface[surf_index];
	}
	else
		return NULL;
}


int LWO2_Object::GetSurfaceIndexFromSurfaceTAG( const UINT2 wSurfTagIndex )
{
	const char* pcSurfaceName = m_tag.pTAGStrings + m_tag.piIndex[wSurfTagIndex];

	size_t i, num_surfs = m_vecSurface.size();
	for(i=0; i<num_surfs; i++)
	{
		LWO2_Surface& rSurf = m_vecSurface[i];
		if( strcmp(pcSurfaceName, rSurf.GetName().c_str()) == 0 )
			return (int)i;
	}

	// This issues too many warnings when this function is called
	// from C3DMeshModelBuilder_LW::ProcessLayer() for every polygon
	// with invalid index.
//	LOG_PRINT_WARNING( fmt_string( "The surface with the index '%d' was not found in %s.", (int)wSurfTagIndex, m_strFilename.c_str() ) );

	return -1;	// surface not found
}


void LWO2_Object::ComputeFaceNormals()
{
	list<LWO2_Layer>::iterator itrLayer;

	for( itrLayer = m_lstLayer.begin(); itrLayer != m_lstLayer.end(); itrLayer++ )
	{
		itrLayer->ComputeFaceNormals();
	}
}


void LWO2_Object::ComputeVertexNormals()
{
	list<LWO2_Layer>::iterator itrLayer;

	for( itrLayer = m_lstLayer.begin(); itrLayer != m_lstLayer.end(); itrLayer++ )
	{
		itrLayer->ComputeVertexNormals();
	}
}


//find the uv-mapping used by the surface
LWO2_TextureUVMap* LWO2_Object::FindTextureUVMapByName(const char *pcTexUVMapName, LWO2_Layer& rLayer)
{
	vector<LWO2_TextureUVMap>& rvecTexUVMap = rLayer.GetTextureUVMap();

	size_t i, num = rvecTexUVMap.size();
	for(i=0; i<num ; i++)
	{
		LWO2_TextureUVMap& rTexUVMap = rvecTexUVMap.at(i);
		if(strcmp(rTexUVMap.strName.c_str(), pcTexUVMapName) == 0)
			return &rTexUVMap;
	}
	//MessageBox(NULL, "The requested uv-mapping for texture was not found in this layer", "Error", MB_OK|MB_ICONWARNING);
	return NULL;
}

/*
//find the uv-mapping used by the surface
LWO2_TextureUVMap* LWO2_Object::FindTextureUVMapFromSurface(LWO2_Surface& rSurf, LWO2_Layer& rLayer)
{
	return FindTextureUVMapByName( rSurf.GetUVMapName().c_str(), rLayer);
}
*/

LWO2_TextureUVMap* LWO2_Object::FindTextureUVMapFromSurface( const LWO2_Surface& rSurf,
		                                                       const UINT4 channel_id,
													            LWO2_Layer& rLayer)
{
	const vector<LWO2_SurfaceBlock>& rvecSurfBlock = rSurf.GetSurfaceBlock();
	int i, iNumSurfaceBlocks = (int)rvecSurfBlock.size();
	int index;
	for( i=0; i<iNumSurfaceBlocks; i++ )
	{
		const LWO2_SurfaceBlock& rSurfBlock = rvecSurfBlock[i];

		if( rSurfBlock.m_Channel == channel_id )
		{
			index = i;
			break;
		}
	}

	if( i == iNumSurfaceBlocks )
		return NULL;

	return FindTextureUVMapByName( rvecSurfBlock[index].m_strUVMapName.c_str(), rLayer);

}


/*
//find the uv-mapping used by the surface
LWO2_TextureUVMap* LWO2_Object::FindTextureUVMapFromSurface(LWO2_Surface& rSurf, LWO2_Layer& rLayer)
{
	int i;
	for(i=0; i< rLayer.m_texuvmap.size(); i++)
	{
		LWO2_TextureUVMap& rTexUVMap = rLayer.m_texuvmap.at(i);
		if(strcmp(rTexUVMap.pName, rSurf.pUVMapName) == 0)
			return &rTexUVMap;
	}
	//MessageBox(NULL, "The requested uv-mapping for texture was not found in this layer", "Error", MB_OK|MB_ICONWARNING);
	return NULL;
}*/


//find the uv-mapping used by the surface
LWO2_VertexColorMap* LWO2_Object::FindVertexColorMapFromSurface(LWO2_Surface& rSurf, LWO2_Layer& rLayer)
{
	size_t i, num = rLayer.GetVertexColorMap().size();
	for(i=0; i<num ; i++)
	{
		LWO2_VertexColorMap& rVCMap = rLayer.GetVertexColorMap()[i];

		if( rVCMap.strName == rSurf.GetVertexColorMap().strName )
			return &rVCMap;
	}
	//MessageBox(NULL, "The requested vertex color mapping was not found in this layer", "Error", MB_OK|MB_ICONWARNING);
	return NULL;
}


int LWO2_Object::GetNumTagStrings() const
{
	return m_tag.iNumTAGs;
}


const char *LWO2_Object::GetTagString( int i ) const
{
	if( m_tag.iNumTAGs <= i )
		return NULL;

	return m_tag.pTAGStrings + m_tag.piIndex[i];
}


int LWO2_Object::GetBoneIndexForWeightMap( LWO2_WeightMap& rWeightMap, LWO2_Layer& rLayer )
{
	const string& strWeightMapName = rWeightMap.GetName();

	int i, num_tags = GetNumTagStrings();
	int tag_index;
	for( i=0; i<num_tags; i++ )
	{
		if( strWeightMapName == GetTagString(i) )
		{
			tag_index = i;
			break;
		}
	}

	if( i == num_tags )
		return -1;	// the corresponding tag string was not found

	list<LWO2_Layer>& rlstLayer = m_lstLayer;
	list<LWO2_Layer>::iterator itrLayer;

	// search all the layers to find the bone - weight maps because the skeleton may exist in
	// a layer different from 'rLayer'
	for(itrLayer = rlstLayer.begin();
		itrLayer != rlstLayer.end();
		itrLayer++)
	{
		const vector<LWO2_BoneWeightMap>& rBoneWeightMap = (*itrLayer).GetBoneWeightMap();
		for( i=0; i<(int)rBoneWeightMap.size(); i++ )
		{
			if( rBoneWeightMap[i].iWeightMapTagIndex == tag_index )
				return rBoneWeightMap[i].iBoneIndex;
		}
	}
/*
	vector<LWO2_BoneWeightMap>& rBoneWeightMap = rLayer.GetBoneWeightMap();
	for( i=0; i<rBoneWeightMap.size(); i++ )
	{
		if( rBoneWeightMap[i].iWeightMapTagIndex == tag_index )
			return rBoneWeightMap[i].iBoneIndex;
	}
*/
	return -1;	// no corresponding bone index was found
}


LWO2_Layer *LWO2_Object::GetLayerWithKeyword( const string& keyword, int match_condition )
{
	list<LWO2_Layer>::iterator itrLayer;

	for(itrLayer = m_lstLayer.begin();
		itrLayer != m_lstLayer.end();
		itrLayer++)
	{
		if( LWO2_NameMatchCond::meet_cond( match_condition, keyword, itrLayer->GetName() ) )
		{
			// the layer name matches the target layer name
			return &(*itrLayer);
		}
	}

	return NULL;
}


vector<LWO2_Layer *> LWO2_Object::GetLayersWithKeyword( const string& keyword, int match_condition)
{
	vector<LWO2_Layer *> vecpTargetLayer;

	list<LWO2_Layer>::iterator itrLayer;
	for(itrLayer = m_lstLayer.begin();
		itrLayer != m_lstLayer.end();
		itrLayer++)
	{
		if( LWO2_NameMatchCond::meet_cond( match_condition, keyword, itrLayer->GetName() ) )
		{
			// the layer name matches the target layer name
			vecpTargetLayer.push_back( &(*itrLayer) );
		}
	}

	return vecpTargetLayer;
}


vector<LWO2_Layer *> LWO2_Object::GetLayersWithKeywords( const vector<string> keyword,
													       int match_condition)
{
	vector<LWO2_Layer *> vecpTargetLayer;

	size_t i, num_keywords = keyword.size();
	list<LWO2_Layer>::iterator itrLayer;

	// check each layer to see if it is a target
	for(itrLayer = m_lstLayer.begin();
		itrLayer != m_lstLayer.end();
		itrLayer++)
	{
		for( i=0; i<num_keywords; i++ )
		{
			if( LWO2_NameMatchCond::meet_cond( match_condition, keyword[i], itrLayer->GetName() ) )
			{
				// the layer name matches one of the target layer names
				// add the layer to the buffer and leave the loop
				vecpTargetLayer.push_back( &(*itrLayer) );
				break;
			}
		}
	}

	return vecpTargetLayer;
}



} // amorphous
