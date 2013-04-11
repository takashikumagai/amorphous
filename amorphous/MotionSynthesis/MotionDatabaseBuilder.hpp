#ifndef  __MotionDatabaseBuilder_H__
#define  __MotionDatabaseBuilder_H__


#include "amorphous/XML/XMLNodeReader.hpp"
#include "amorphous/Support/Log/DefaultLog.hpp"

#include "fwd.hpp"
#include "MotionPrimitive.hpp"
#include "MotionDatabase.hpp"
#include "Skeleton.hpp"
#include "HumanoidMotionTable.hpp"


namespace amorphous
{


namespace msynth
{

class CJointFixMod
{
public:

	enum TargetFlag
	{
		TX = (1 << 0),
		TY = (1 << 1),
		TZ = (1 << 2),
		RX = (1 << 3),
		RY = (1 << 4),
		RZ = (1 << 5),
	};

	std::string m_JointName;

	U32 m_TargetFlags;

	Matrix34 m_Fixed;

	CJointFixMod()
		:
	m_TargetFlags(0),
	m_Fixed( Matrix34Identity() )
	{}
};


class CMotionPrimitiveDesc
{
public:

	std::string m_Name;
	int m_StartFrame;
	int m_EndFrame;

	/// If this is left empty, the first of the scene's root bones is used.
	std::string m_RootBoneName;

	bool m_bIsLoopMotion;

	bool m_bResetHorizontalRootPos;

	float m_fRootNodeHeightShift;

	/// how to normalize horizontal orientation of the motion
	/// AverageMotionDirection or
	/// LocalPositiveZDirection or
	/// AlignLastKeyframe - put that last keyframe on the x=0 axis
	/// AlignLastKeyframeBVH - put that last keyframe on the x=0 axis (for BVH motion data)
	/// None
	std::string m_NormalizeOrientation;

	std::vector<std::string> m_vecAnnotation;

	std::string m_StartBoneName;

	std::vector<CJointFixMod> m_vecFixMod;

	/// output
	boost::shared_ptr<CMotionPrimitive> m_pMotionPrimitive;

public:

	CMotionPrimitiveDesc()
		:
	m_StartFrame(0),
	m_EndFrame(0),
	m_bIsLoopMotion(false),
	m_bResetHorizontalRootPos(false),
	m_fRootNodeHeightShift(0.0f)
	{}

	CMotionPrimitiveDesc( int sf, int ef )
		:
	m_StartFrame(sf),
	m_EndFrame(ef),
	m_bIsLoopMotion(false),
	m_bResetHorizontalRootPos(false),
	m_fRootNodeHeightShift(0.0f)
	{}
};


/// created for each bvh file
class CMotionPrimitiveDescGroup
{
public:

	/// .bvh file that contains captured motion data
	/// or .lws file
	std::string m_Filename;

	/// skeleton extracted from the motion file
	/// - each compiler implementation is responsible for creating a skeleton
	///   from motion data store it here.
	/// - BVH file: create a skeleton from the HIERARCHY section of BVH file
	/// - LWS file: create a skeleton from bones in LWS file
	CSkeleton m_Skeleton;

	/// scaling factor applied to skeleton and all the motion primitives
	/// created from file
	float m_fScalingFactor;

	std::vector<CMotionPrimitiveDesc> m_Desc;

public:

	CMotionPrimitiveDescGroup()
		:
	m_fScalingFactor(1.0f)
	{}
};


class CMotionDatabaseCompiler
{
protected:

	/// Borrowed reference set by CMotionDatabaseBuilder and available to derived class
	/// derived classes are responsible for storing created motion primitives to this variable
	std::vector< boost::shared_ptr<CMotionPrimitive> > *m_pvecpMotionPrimitive;

	std::vector<std::string> *m_vecpAnnotationName;

	CHumanoidMotionTable *m_pMotionTable;

protected:

	/// Allows derived class to access private member of 'CMotionPrimitive'
	std::vector<char>& AnnotationArray( CMotionPrimitive& motion ) { return motion.m_vecAnnotation; }

	int GetAnnotationIndex( const std::string& annotation_name );

public:

	CMotionDatabaseCompiler()
		:
	m_pvecpMotionPrimitive(NULL),
	m_vecpAnnotationName(NULL),
	m_pMotionTable(NULL)
	{}

	virtual ~CMotionDatabaseCompiler() {}

	/// Create motion data from descs
	/// - Each derived class is responsible for implementing this method
	///   to create motion primitives from 'm_vecDescGroup' and store
	///   them to 'm_vecMotionPrimitive'
	virtual void CreateMotionPrimitives( CMotionPrimitiveDescGroup& desc_group ) = 0;

	friend CMotionDatabaseBuilder;
};


class CMotionPrimitiveCompilerCreator
{
public:

	CMotionPrimitiveCompilerCreator() {}

	virtual ~CMotionPrimitiveCompilerCreator() {}

	virtual const char *Extension() const = 0;

	virtual boost::shared_ptr<CMotionDatabaseCompiler> Create() const = 0;
};


class CMotionMapTarget
{
public:

	std::string m_DestSkeletonMDB;
	std::string m_DestSkeletonMotion;

	std::map<std::string,std::string> m_BoneMaps;

	bool IsValid() const
	{
		if( !m_BoneMaps.empty()
		 && 0 < m_DestSkeletonMDB.length()
		 && 0 < m_DestSkeletonMotion.length() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};


class CJointModification
{
public:

	std::string m_Name;

	Matrix33 m_matPreRotation;
	Matrix33 m_matPostRotation;

	bool m_ApplyPreRotation;
	bool m_ApplyPostRotation;

public:

	CJointModification()
		:
	m_matPreRotation(Matrix33Identity()),
	m_matPostRotation(Matrix33Identity()),
	m_ApplyPreRotation(false),
	m_ApplyPostRotation(false)
	{}
};



/**
 - input: xml files that contain motion primitive descs
 - output: motion database file


*/
class CMotionDatabaseBuilder
{
protected:

	std::vector<CMotionPrimitiveDescGroup> m_vecDescGroup;

	/// stores motion primitives created from descs above
//	std::vector<CMotionPrimitive> m_vecMotionPrimitive;
	std::vector< boost::shared_ptr<CMotionPrimitive> > m_vecpMotionPrimitive;

	std::vector<std::string> m_vecAnnotationName;

	CHumanoidMotionTable m_MotionTable;

	CMotionMapTarget m_MotionMapTarget;

	std::vector<CJointModification> m_vecJointModification;

	std::string m_SourceXMLFilename;

	std::string m_OutputFilepath;

protected:

//	int GetAnnotationIndex( const std::string& annotation_name );

	void CreateMotionPrimitiveDesc( CXMLNodeReader& node_reader );

	/// called once for every bvh file
	void CreateMotionPrimitiveDescGroup( CXMLNodeReader& bvh_file_node_reader );

//	void CreateMotionPrimitive( const CMotionPrimitiveDesc& desc, const CMotionPrimitiveDescGroup& desc_group, BVHPlayer& bvh_player );

	void CreateMotionTableEntry( xercesc::DOMNode *pMotionEntryNode, CHumanoidMotionEntry& entry );

	void CreateMotionTable( xercesc::DOMNode *pMotionTableNode );

	void ProcessXMLFile( CXMLNodeReader& root_node );

	bool CreateMotionPrimitivesFromScriptFile( const std::string& script_filename );

	bool CreateAnnotationTable( CXMLNodeReader& annot_table_node );

	bool SetMotionMapTargets( CXMLNodeReader& mapping );

	void ProcessGlobalModificationOptions( CXMLNodeReader& node );

	void ApplyJointModification( const CJointModification& mod );

	void ApplyJointFixModification( const std::vector<CJointFixMod>& mods, CMotionPrimitive& target_motion );

	void ProcessRootNodeHorizontalElementOptions( CXMLNodeReader& root_joint_node, CMotionPrimitiveDesc& desc );

	void ProcessCreatedMotionPrimitive( CMotionPrimitiveDesc& desc ); 

	Result::Name MapMotionPrimitiveToAnotherSkeleton( boost::shared_ptr<CMotionPrimitive>& pSrcMotion,
													  boost::shared_ptr<CSkeleton>& pDestSkeleton );

	Result::Name MapMotionPrimitivesToAnotherSkeleton();

//	void ProcessFiles();

	void CreateMotionPrimitives();

public:

	CMotionDatabaseBuilder() {}

	~CMotionDatabaseBuilder() {}

	bool Build( const std::string& source_script_filename );

	bool SaveMotionDatabaseToFile( const std::string& db_filename );

	virtual bool IsValidMotionFile( const std::string& src_filepath ) { return true; }

	const std::string& GetOutputFilepath() const { return m_OutputFilepath; }
};


/// Used by derived classes
extern void AlignLastKeyframe( std::vector<CKeyframe>& vecKeyframe );
extern Matrix33 CalculateHorizontalOrientation( const Matrix34& pose );


extern void RegisterMotionPrimitiveCompilerCreator( boost::shared_ptr<CMotionPrimitiveCompilerCreator> pCreator );


} // namespace msynth

} // namespace amorphous



#endif /* __MotionDatabaseBuilder_H__ */