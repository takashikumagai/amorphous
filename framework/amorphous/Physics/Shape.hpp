#ifndef  __PhysShape_H__
#define  __PhysShape_H__


#include "../3DMath/Matrix34.hpp"
#include "../Support/SafeDelete.hpp"

#include "fwd.hpp"
#include "ShapeEnums.hpp"


namespace amorphous
{


namespace physics
{


class CShapeImpl
{
public:

	CShapeImpl() {}

	virtual ~CShapeImpl() {}

//	virtual int GetType() const = 0;

//	virtual bool Raycast( CRay& world_ray, Scalar max_ray_dist, bool first_hit ) = 0;

	/// Retrieves the actor which this shape is associated with. 
//	virtual CActor &GetActor () const { return *m_pActor; }

	/// Sets which collision group this shape is part of. 
	virtual void SetCollisionGroup ( U16 group ) = 0;

	/// Retrieves the value set with setGroup(). 
	virtual U16 GetCollisionGroup() const = 0;

	/// Returns a world space AABB enclosing this shape. 
//	virtual void  GetWorldAABB ( AABB3 &dest ) const = 0;

	/// Sets shape flags. 
//	virtual void  setFlag (NxShapeFlag flag, bool value)=0 

	/// Retrieves shape flags. 
//	virtual NX_BOOL  getFlag (NxShapeFlag flag) const =0 

	/// Assigns a material index to the shape.
	virtual void SetMaterialID( int material_id ) = 0;

	/// Retrieves the material index currently assigned to the shape. 
	virtual int GetMaterialID() const = 0;

	/// Sets the skin width. See NxShapeDesc::skinWidth. 
//	virtual void  setSkinWidth (Scalar skinWidth)=0 

	/// Retrieves the skin width. See NxShapeDesc::skinWidth. 
//	virtual Scalar  getSkinWidth () const =0 

	/// returns the type of shape. 
	// virtual NxShapeType  getType () const =0 
	 
	/// Assigns a CCD Skeleton mesh. 
//	virtual void  setCCDSkeleton (NxCCDSkeleton *ccdSkel)=0 
	 
	/// Retrieves the CCDSkeleton for this shape. 
//	virtual NxCCDSkeleton *  getCCDSkeleton () const =0 
	 
	/// Sets a name string for the object that can be retrieved with getName(). 
//	virtual void  setName (const char *name)=0 
	 
	/// retrieves the name string set with setName(). 
//	virtual const char *  getName () const =0 
	 
	/// Sets 128-bit mask used for collision filtering. See comments for NxGroupsMask. 
//	virtual void  setGroupsMask (const NxGroupsMask &mask)=0 
	 
	/// Gets 128-bit mask used for collision filtering. See comments for NxGroupsMask. 
//	virtual const NxGroupsMask  getGroupsMask () const =0 
	 
	/// Pose Manipulation

	/// The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual void  setLocalPose (const Matrix34 &mat)=0 
	 
	/// The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual void  setLocalPosition (const Vector3 &vec)=0 
	 
	/// The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual void  setLocalOrientation (const Matrix33 &mat)=0 

	/// The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by. This transformation is identity by default. 
	virtual Matrix34 GetLocalPose() const = 0;

	/// The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual Vector3  getLocalPosition () const =0 
	 
	/// The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by. This transformation is identity by default. 
//	virtual Matrix33  getLocalOrientation () const =0 
	 
	/// The setGlobal() calls are convenience methods which transform the passed parameter into the current local space of the actor and then call setLocalPose(). 
//	virtual void  setGlobalPose (const Matrix34 &mat)=0 
	 
	/// The setGlobal() calls are convenience methods which transform the passed parameter into the current local space of the actor and then call setLocalPose(). 
//	virtual void  setGlobalPosition (const Vector3 &vec)=0 
	 
	/// The setGlobal() calls are convenience methods which transform the passed parameter into the current local space of the actor and then call setLocalPose(). 
//	virtual void  setGlobalOrientation (const Matrix33 &mat)=0 
	 
	/// The getGlobal*() methods retrieve the shape's current world space pose. This is the local pose multiplied by the actor's current global pose. 
//	virtual Matrix34  getGlobalPose () const =0 
	 
	/// The getGlobal*() methods retrieve the shape's current world space pose. This is the local pose multiplied by the actor's current global pose. 
//	virtual Vector3  getGlobalPosition () const =0 
	 
	/// The getGlobal*() methods retrieve the shape's current world space pose. This is the local pose multiplied by the actor's current global pose. 
//	virtual Matrix33  getGlobalOrientation () const =0 

	/// NX_INLINE const NxHeightFieldShape *  isHeightField () const  

	/// Raycasting and Overlap Testing 

	/// casts a world-space ray against the shape. 
	virtual bool Raycast ( const CRay &world_ray, Scalar max_dist, U32 hintFlags, CRaycastHit &hit, bool first_hit ) const = 0;

	/// Checks whether the shape overlaps a world-space sphere or not. 
//	virtual bool  checkOverlapSphere (const NxSphere &worldSphere) const =0 

	/// Checks whether the shape overlaps a world-space OBB or not. 
//	virtual bool  checkOverlapOBB (const NxBox &worldBox) const =0 

	/// Checks whether the shape overlaps a world-space AABB or not. 
//	virtual bool  checkOverlapAABB (const NxBounds3 &worldBounds) const =0 

	/// Checks whether the shape overlaps a world-space capsule or not. 
//	virtual bool  checkOverlapCapsule (const NxCapsule &worldCapsule) const =0

	virtual Vector3 GetDimensions() const { return Vector3(0,0,0); }

	virtual Scalar GetRadius() const { return 0; }

	virtual Scalar GetLength() const { return 0; }

	virtual void SetDimensions( const Vector3& radii ) {}

	virtual void SetRadius( Scalar radius ) {}

	virtual void SetLength( Scalar length ) {}
};


class CShape
{
protected:

	CShapeImpl *m_pImpl;

	int m_MaterialID; // cache material id

	/// owner actor (borrowed reference)
	CActor *m_pActor;

public:

	CShape( CShapeImpl *pImpl ) : m_pImpl(pImpl), m_MaterialID(0) {}

	virtual ~CShape() { SafeDelete( m_pImpl ); }

//	CShape() : m_pBaseImpl(NULL) { m_pBaseImpl = ...; }
//	virtual ~CShape() { SafeDelete( m_pBaseImpl ); }

	virtual int GetType() const = 0;

//	virtual bool Raycast( CRay& world_ray, Scalar max_ray_dist, bool first_hit ) = 0;

	/// Retrieves the actor which this shape is associated with. 
	virtual CActor &GetActor() const { return *m_pActor; }

	/// Sets which collision group this shape is part of. 
	void SetCollisionGroup( U16 group ) { m_pImpl->SetCollisionGroup( group ); }

	/// Retrieves the value set with setGroup(). 
	U16 GetCollisionGroup() const { return m_pImpl->GetCollisionGroup(); }

	/// Returns a world space AABB enclosing this shape. 
//	virtual void  GetWorldAABB ( AABB3 &dest ) const = 0;

	/// Sets shape flags. 
//	virtual void  setFlag (NxShapeFlag flag, bool value)=0 

	/// Retrieves shape flags. 
//	virtual NX_BOOL  getFlag (NxShapeFlag flag) const =0 

	/// Assigns a material index to the shape.
	void SetMaterialID( int material_id ) { m_pImpl->SetMaterialID( material_id ); }

	/// Retrieves the material index currently assigned to the shape. 
	int GetMaterialID() const { return m_pImpl->GetMaterialID(); }

	/// Sets the skin width. See NxShapeDesc::skinWidth. 
//	virtual void  setSkinWidth (Scalar skinWidth)=0 

	/// Retrieves the skin width. See NxShapeDesc::skinWidth. 
//	virtual Scalar  getSkinWidth () const =0 

	/// returns the type of shape. 
	// virtual NxShapeType  getType () const =0 
	 
	/// Assigns a CCD Skeleton mesh. 
//	virtual void  setCCDSkeleton (NxCCDSkeleton *ccdSkel)=0 
	 
	/// Retrieves the CCDSkeleton for this shape. 
//	virtual NxCCDSkeleton *  getCCDSkeleton () const =0 
	 
	/// Sets a name string for the object that can be retrieved with getName(). 
//	virtual void  setName (const char *name)=0 
	 
	/// retrieves the name string set with setName(). 
//	virtual const char *  getName () const =0 
	 
	/// Sets 128-bit mask used for collision filtering. See comments for NxGroupsMask. 
//	virtual void  setGroupsMask (const NxGroupsMask &mask)=0 
	 
	/// Gets 128-bit mask used for collision filtering. See comments for NxGroupsMask. 
//	virtual const NxGroupsMask  getGroupsMask () const =0 
	 
	/// Pose Manipulation

	/// The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual void  setLocalPose (const Matrix34 &mat)=0 
	 
	/// The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual void  setLocalPosition (const Vector3 &vec)=0 
	 
	/// The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual void  setLocalOrientation (const Matrix33 &mat)=0 

	/// The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by. This transformation is identity by default. 
	Matrix34 GetLocalPose() const { return m_pImpl->GetLocalPose(); }

	/// The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by. 
//	virtual Vector3  getLocalPosition () const =0 
	 
	/// The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by. This transformation is identity by default. 
//	virtual Matrix33  getLocalOrientation () const =0 
	 
	/// The setGlobal() calls are convenience methods which transform the passed parameter into the current local space of the actor and then call setLocalPose(). 
//	virtual void  setGlobalPose (const Matrix34 &mat)=0 
	 
	/// The setGlobal() calls are convenience methods which transform the passed parameter into the current local space of the actor and then call setLocalPose(). 
//	virtual void  setGlobalPosition (const Vector3 &vec)=0 
	 
	/// The setGlobal() calls are convenience methods which transform the passed parameter into the current local space of the actor and then call setLocalPose(). 
//	virtual void  setGlobalOrientation (const Matrix33 &mat)=0 
	 
	/// The getGlobal*() methods retrieve the shape's current world space pose. This is the local pose multiplied by the actor's current global pose. 
//	virtual Matrix34  getGlobalPose () const =0 
	 
	/// The getGlobal*() methods retrieve the shape's current world space pose. This is the local pose multiplied by the actor's current global pose. 
//	virtual Vector3  getGlobalPosition () const =0 
	 
	/// The getGlobal*() methods retrieve the shape's current world space pose. This is the local pose multiplied by the actor's current global pose. 
//	virtual Matrix33  getGlobalOrientation () const =0 

	/// NX_INLINE const NxHeightFieldShape *  isHeightField () const  

	/// Raycasting and Overlap Testing 

	/// casts a world-space ray against the shape. 
	bool Raycast ( const CRay &world_ray, Scalar max_dist, U32 hint_flags, CRaycastHit &hit, bool first_hit ) const { return m_pImpl->Raycast( world_ray, max_dist, hint_flags, hit, first_hit ); }

	/// Checks whether the shape overlaps a world-space sphere or not. 
//	virtual bool  checkOverlapSphere (const NxSphere &worldSphere) const =0 

	/// Checks whether the shape overlaps a world-space OBB or not. 
//	virtual bool  checkOverlapOBB (const NxBox &worldBox) const =0 

	/// Checks whether the shape overlaps a world-space AABB or not. 
//	virtual bool  checkOverlapAABB (const NxBounds3 &worldBounds) const =0 

	/// Checks whether the shape overlaps a world-space capsule or not. 
//	virtual bool  checkOverlapCapsule (const NxCapsule &worldCapsule) const =0

	/// TODO: Allow only physics classes to access impl. Example: CCloth
	const CShapeImpl *GetImpl() const { return m_pImpl; }

	friend class CActor;
};


class CBoxShape : public CShape
{
//	CBoxShapeImpl *m_pImpl;

public:

	CBoxShape( CShapeImpl *pImpl ) : CShape(pImpl) {}

	virtual ~CBoxShape() {}

	virtual int GetType() const { return PhysShape::Box; }

	/// Sets the box dimensions.
	/// - radii of the box
	void SetDimensions( const Vector3& radii ) { m_pImpl->SetDimensions( radii ); }

	/// Retrieves the dimensions of the box. 
	/// - radii of the box
	Vector3 GetDimensions() const { return m_pImpl->GetDimensions(); }

	/// Gets the box represented as a world space OBB. 
//	virtual void  GetWorldOBB (NxBox &obb) const = 0 

	/// Saves the state of the shape object to a descriptor. 
//	virtual void  saveToDesc (NxBoxShapeDesc &desc) const =0 
};


class CSphereShape : public CShape
{
//	CSphereShapeImpl *m_pImpl;

public:

	CSphereShape( CShapeImpl *pImpl ) : CShape(pImpl) {}

	virtual ~CSphereShape() {}

	virtual int GetType() const { return PhysShape::Sphere; }

	/// Retrieves the radius of the sphere. 
	Scalar GetRadius() const { return m_pImpl->GetRadius(); }

	/// Sets the sphere radius. 
	void  SetRadius( Scalar radius ) { m_pImpl->SetRadius( radius ); }

	/// Gets the sphere data in world space. 
//	virtual void  getWorldSphere (NxSphere &worldSphere) const =0 

//	virtual void  saveToDesc (NxSphereShapeDesc &desc) const =0 

};


class CCapsuleShape : public CShape
{
//	CCapsuleShapeImpl *m_pImpl;

public:

	CCapsuleShape( CShapeImpl *pImpl ) : CShape(pImpl) {}

	virtual ~CCapsuleShape() {}

	virtual int GetType() const { return PhysShape::Capsule; }

	/// Call this to initialize or alter the capsule. 
//	virtual void  setDimensions (Scalar radius, Scalar height)=0 

	/// Alters the radius of the capsule. 
//	virtual void  setRadius (Scalar radius)=0 

	/// Retrieves the radius of the capsule. 
	Scalar GetRadius() const { return m_pImpl->GetRadius(); }

	/// Alters the height of the capsule. 
//	virtual void  setHeight (Scalar height)=0 

	/// Retrieves the height of the capsule. 
	Scalar GetLength() const { return m_pImpl->GetLength(); }

	/// Retrieves the capsule parameters in world space. See NxCapsule. 
//	virtual void  getWorldCapsule (NxCapsule &worldCapsule) const =0 
 
//	virtual void  saveToDesc (NxCapsuleShapeDesc &desc) const =0 

};


/*
class CCylinderShape
{
//	CCylinderShapeImpl *m_pImpl;

public:

	CCylinderShape() {}
	virtual ~CCylinderShape() {}

	virtual int GetType() const { return Cylinder; }
};
*/


class CConvexShape : public CShape
{
public:

	CConvexShape( CShapeImpl *pImpl ) : CShape(pImpl) {}

	int GetType() const { return PhysShape::Convex; }
};


class CTriangleMeshShape : public CShape
{
//	CTriangleMeshShapeImpl *m_pImpl;

public:

	CTriangleMeshShape( CShapeImpl *pImpl ) : CShape(pImpl) {}

	virtual ~CTriangleMeshShape() {}

	virtual int GetType() const { return PhysShape::TriangleMesh; }

//	virtual void  saveToDesc (NxTriangleMeshShapeDesc &desc) const =0 

	/// Retrieves the triangle mesh data associated with this instance. 
//	virtual NxTriangleMesh &  GetTriangleMesh ()=0 

//	virtual const NxTriangleMesh &  GetTriangleMesh () const =0 

	/// Retrieves triangle data from a triangle ID. 
//	virtual U32  getTriangle (NxTriangle &triangle, NxTriangle *edgeTri, U32 *flags, NxTriangleID triangleIndex, bool worldSpaceTranslation=true, bool worldSpaceRotation=true) const =0 

	/// Finds triangles touching the input bounds. 
//	NX_INLINE bool  overlapAABBTriangles (const NxBounds3 &bounds, U32 flags, U32 &nb, const U32 *&indices) const  

	/// Finds triangles touching the input bounds.  
//	virtual bool  overlapAABBTriangles (const NxBounds3 &bounds, U32 flags, NxUserEntityReport< U32 > *callback) const =0 
};


class CPlaneShape : public CShape
{
public:

	CPlaneShape( CShapeImpl *pImpl ) : CShape(pImpl) {}

	int GetType() const { return PhysShape::Plane; }
};


} // namespace physics

} // namespace amorphous



#endif		/*  __PhysShape_H__  */
