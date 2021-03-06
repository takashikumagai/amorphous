#ifndef  __GameItem_Clothing_HPP__
#define  __GameItem_Clothing_HPP__

#include "GameItem.hpp"


namespace amorphous
{

class CClothSystem;


//======================================================================================
// Clothing
//======================================================================================


/**
 game item that represents firearm
 There are two types of clothes
 1. The one that uses cloth simulation
   e.g. long coat, cloak, skirt, etc.
 2. The one that do not use cloth simulation
   - This is divided into the two sub-categories
     - The one that has skeletal structure.
	   e.g. tight-fitting jeans/tees, etc.
	 - The one that does not have skeletal
	   - Small accessaries that can be represented as rigid bodies
	     - hat, shoes, etc.
*/
class Clothing : public GameItem
{
protected:

	bool m_ApplyClothSimulation;

	std::string m_NameOfBoneToAttachTo;

	float m_fProjectionMatrixOffset;

	std::weak_ptr<CClothSystem> m_pClothSystem;

	std::shared_ptr<CustomMesh> m_pClothMesh;

public:

	Clothing();

	virtual ~Clothing() {}

	virtual void Update( float dt );

	virtual bool HandleInput( int input_code, int input_type, float fParam );

	virtual unsigned int GetArchiveObjectID() const { return ID_CLOTHING; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual void LoadFromXMLNode( XMLNode& reader );
};


} // namespace amorphous



#endif  __GameItem_Clothing_HPP__
