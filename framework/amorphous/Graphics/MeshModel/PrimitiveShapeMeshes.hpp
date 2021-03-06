#ifndef __PrimitiveShapeMeshes_HPP__
#define __PrimitiveShapeMeshes_HPP__


#include "../../base.hpp"
#include "../fwd.hpp"
#include "../../3DMath/Vector3.hpp"


namespace amorphous
{


class AxisAndDirection
{
public:
	enum Name
	{
		POS_X,
		POS_Y,
		POS_Z,
		NEG_X,
		NEG_Y,
		NEG_Z,
	};

	static Vector3 GetAxis( AxisAndDirection::Name axis_dir )
	{
		Vector3 vAxis = Vector3(0,0,0);
		vAxis[ axis_dir % 3 ] = 1.0f * ( (axis_dir < 3) ? 1 : -1 );
		return vAxis;
	}
};


class PrimitivePlacingStyle
{
public:
	enum Name
	{
		PLACE_CENTER_AT_ORIGIN,
		PLACE_ON_PLANE,
		NUM_STYLES,
	};
};


class MeshPolygonDirection
{
public:
	enum Type
	{
		INWARD,
		OUTWARD,
		NUM_TYPES
	};
};


class ConeDesc
{
public:

	float radius;
	float cone_height;
	float body_height;
	AxisAndDirection::Name axis;
	int num_sides;
	int num_segments;

public:

	ConeDesc()
		:
	radius(1),
	cone_height(1),
	body_height(1),
	axis(AxisAndDirection::POS_Y),
	num_sides(16),
	num_segments(1)
	{}
};


class BoxDesc
{
public:

	Vector3 vLengths;

public:

	BoxDesc()
		:
	vLengths( Vector3(1,1,1) )
	{}
};


class CCylinderMeshStyleFlags
{
public:
	enum Flags
	{
		TOP_POLYGONS    = (1 << 0),
		BOTTOM_POLYGONS = (1 << 1),
		WELD_VERTICES   = (1 << 3),
	};
};


class CylinderDesc
{
public:

	float radii[2]; ///< array of radii
	float height;
	AxisAndDirection::Name axis;

	/// How finely the cylinder is divided on its side.
	/// Greater value makes the cylinder closer to the circular shape.
	int num_sides;

	/// How many times the cylinder is divided along the direction of its axis.
	/// This value does not change the appearance of the mesh.
	int num_divisions;

	PrimitivePlacingStyle::Name style;

	U32 style_flags; ///< default = TOP_POLYGONS | BOTTOM_POLYGONS

public:

	CylinderDesc()
		:
	height(1),
	axis(AxisAndDirection::POS_Y),
	num_sides(24),
	num_divisions(1),
	style(PrimitivePlacingStyle::PLACE_CENTER_AT_ORIGIN),
	style_flags( CCylinderMeshStyleFlags::TOP_POLYGONS | CCylinderMeshStyleFlags::BOTTOM_POLYGONS )
	{
		for( int i = 0; i<sizeof(radii)/sizeof(float); i++ )
			radii[i] = 0.5f;
	}

	bool IsValid() const
	{
		if( 0.001f < radii[0]
		 && 0.001f < radii[1]
		 && 0.001f < height
		 && 2 < num_sides )
		{
			return true;
		}
		else
			return false;
	}
};


class SphereDesc
{
public:
	MeshPolygonDirection::Type poly_dir;
	float radii[3]; ///< array of radii
	int num_sides;
	int num_segments;
	int axis;

public:

	SphereDesc()
		:
	poly_dir(MeshPolygonDirection::OUTWARD),
	num_sides(24),
	num_segments(12),
	axis(2)
	{
		for( int i = 0; i<3; i++ )
			radii[i] = 0.5f;
	}

	bool IsValid() const
	{
		if( 0.001f < radii[0]
		 && 0.001f < radii[1]
		 && 0.001f < radii[2]
		 && 2 < num_sides
		 && 1 < num_segments )
		{
			return true;
		}
		else
			return false;
	}
};



class CapsuleDesc
{
public:
	float radius;
	float length;
	int num_sides;
	int num_segments; ///< segments of a hemisphere

public:

	CapsuleDesc()
		:
	radius(0.5f),
	length(2.0f),
	num_sides(12),
	num_segments(6)
	{
	}

	bool IsValid() const
	{
		if( 0.001f < radius
		 && 0.001f < length - radius * 2.0f
		 && 2 < num_sides
		 && 1 < num_segments )
		{
			return true;
		}
		else
			return false;
	}
};



extern void CreateCylinderMesh( const CylinderDesc& desc, General3DMesh& mesh );
extern void CreateConeMesh( const ConeDesc& desc,         General3DMesh& mesh );
extern void CreateSphereMesh( const SphereDesc& desc,     General3DMesh& mesh );
extern void CreateCapsuleMesh( const CapsuleDesc& desc,   General3DMesh& mesh );

extern Result::Name CreateCylinderMeshArchive( const CylinderDesc& desc, C3DMeshModelArchive& mesh_archive );
extern Result::Name CreateConeMeshArchive( const ConeDesc& desc,         C3DMeshModelArchive& mesh_archive );
extern Result::Name CreateSphereMeshArchive( const SphereDesc& desc,     C3DMeshModelArchive& mesh_archive );
extern Result::Name CreateCapsuleMeshArchive( const CapsuleDesc& desc,   C3DMeshModelArchive& mesh_archive );

extern Result::Name CreateArchiveFromGeneral3DMesh( std::shared_ptr<General3DMesh>& pSrcMesh, C3DMeshModelArchive& dest_mesh_archive );


} // namespace amorphous



#endif  /* __PrimitiveShapeMeshes_HPP__ */
