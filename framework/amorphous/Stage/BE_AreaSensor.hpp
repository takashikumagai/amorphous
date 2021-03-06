#ifndef __BE_AREASENSOR_H__
#define __BE_AREASENSOR_H__

#include "BaseEntity.hpp"


namespace amorphous
{


class CBE_AreaSensor : public BaseEntity
{
private:

public:

	CBE_AreaSensor();
	//~CBE_AreaSensor();

	// void Init() {}
	void InitCopyEntity( CCopyEntity* pCopyEnt );

	// void Act(CCopyEntity* pCopyEnt) {}	//behavior in in one frame
	// void Draw(CCopyEntity* pCopyEnt) {} 
	void Touch(CCopyEntity* pCopyEnt_Self, CCopyEntity* pCopyEnt_Other);
	// void ClipTrace( STrace& rLocalTrace, CCopyEntity* pMyself );
	// void MessageProcedure(GameMessage& rGameMessage, CCopyEntity* pCopyEnt_Self) {}

	virtual unsigned int GetArchiveObjectID() const { return BE_AREASENSOR; }

	virtual void Serialize( IArchive& ar, const unsigned int version );

};
} // namespace amorphous



#endif  /*  __BE_AREASENSOR_H__  */
