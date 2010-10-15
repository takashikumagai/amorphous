#include "NxPhysShapeDescFactory.hpp"
#include "NxPhysConv.hpp"
#include "NxMathConv.hpp"
#include "../ShapeDesc.hpp"
#include "../BoxShapeDesc.hpp"
#include "../SphereShapeDesc.hpp"
#include "../CapsuleShapeDesc.hpp"
#include "../TriangleMeshDesc.hpp"
#include "../TriangleMeshShapeDesc.hpp"
#include "NxPhysTriangleMesh.hpp"


namespace physics
{


NxShapeDesc *CNxPhysShapeDescFactory::CreateNxShapeDesc( CShapeDesc &src_desc )
{
	NxShapeDesc *pNxShapeDesc = NULL;
	switch( src_desc.GetArchiveObjectID() )
	{
	case PhysShape::Box:
		{
			CBoxShapeDesc *pSrcBoxDesc = dynamic_cast<CBoxShapeDesc *>(&src_desc);
			NxBoxShapeDesc *pDesc = new NxBoxShapeDesc();
			pDesc->dimensions = ToNxVec3( pSrcBoxDesc->vSideLength );
			//pDesc->mass = src_desc.???
			pNxShapeDesc = pDesc;
		}
		break;

	case PhysShape::Sphere:
		{
			CSphereShapeDesc *pSrcSphereDesc = dynamic_cast<CSphereShapeDesc *>(&src_desc);
			NxSphereShapeDesc *pDesc = new NxSphereShapeDesc();
			pDesc->radius = pSrcSphereDesc->Radius;
			pNxShapeDesc = pDesc;
		}
		break;

	case PhysShape::Capsule:
		{
			CCapsuleShapeDesc *pSrcCapsuleDesc = dynamic_cast<CCapsuleShapeDesc *>(&src_desc);
			NxCapsuleShapeDesc *pDesc = new NxCapsuleShapeDesc();
			pDesc->radius = pSrcCapsuleDesc->fRadius;
			pDesc->height = pSrcCapsuleDesc->fLength;
			pNxShapeDesc = pDesc;
		}
		break;

	case PhysShape::TriangleMesh:
		{
			CTriangleMeshShapeDesc *pSrcTriMeshDesc = dynamic_cast<CTriangleMeshShapeDesc *>(&src_desc);
			NxTriangleMeshShapeDesc *pDesc = new NxTriangleMeshShapeDesc();

			// retrieve the triangle mesh of Ageia PhysX
			CNxPhysTriangleMesh *pNxPhysMesh = dynamic_cast<CNxPhysTriangleMesh *>(pSrcTriMeshDesc->pTriangleMesh);
			pDesc->meshData = pNxPhysMesh->GetNxTriangleMesh();
			pNxShapeDesc = pDesc;
		}
		break;

	default:
		break;
	}

	if( !pNxShapeDesc )
		return NULL;

	// Copy the values of member variables from CShapeDesc to NxShapeDesc
	NxShapeDesc& desc = *pNxShapeDesc;
	desc.materialIndex    = src_desc.MaterialIndex;
	desc.shapeFlags       = ToNxShapeFlags( src_desc.ShapeFlags );
	desc.group            = 0;//src_shape_desc.CollisionGroup;
	desc.groupsMask.bits0 = 0;
	desc.groupsMask.bits1 = 0;
	desc.groupsMask.bits2 = 0;
	desc.groupsMask.bits3 = 0;

	return pNxShapeDesc;
}


}
