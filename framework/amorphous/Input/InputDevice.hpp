#ifndef __InputDevice_H__
#define __InputDevice_H__


#include <memory>
#include <mutex>
#include "fwd.hpp"
#include "InputHub.hpp"
#include "InputDeviceGroup.hpp"
#include "ForceFeedback/fwd.hpp"

/// auto repeat control requires the timer
#include "amorphous/Support/Timer.hpp"
#include "amorphous/Support/FixedVector.hpp"


namespace amorphous
{


class InputDevice
{
	enum Param
	{
		FIRST_AUTO_REPEAT_INTERVAL_MS = 300,
//		NUM_MAX_SIMULTANEOUS_PRESSES  = 4,
	};

	void SetGroup( InputDeviceGroup *pGroup );

protected:

	std::string m_InstanceName;

	std::string m_ProductName;

	InputDeviceGroup *m_pGroup;

protected:

	virtual bool IsKeyPressed( int gi_code ) { return false; }

	virtual bool IsReleventInput( int gi_code ) { return false; }

	virtual void RefreshKeyStates() {}

	void SetImplToForceFeedbackEffect( std::shared_ptr<CForceFeedbackEffectImpl> pImpl, CForceFeedbackEffect& ffe );

public:

	enum InputDeviceType
	{
		TYPE_INVALID,
		TYPE_GAMEPAD,
		TYPE_KEYBOARD,
		TYPE_MOUSE,
		NUM_INPUT_DEVICE_TYPES
	};

public:

	InputDevice();

	virtual ~InputDevice();

	virtual InputDevice::InputDeviceType GetInputDeviceType() const = 0;

	TCFixedVector<int,InputDeviceParam::NUM_MAX_SIMULTANEOUS_PRESSES>& PressedKeyList() { return m_pGroup->m_PressedKeyList; }

	CInputState& InputState( int gi_code ) { return m_pGroup->m_aInputState[gi_code]; }

	void UpdateInputState( const InputData& input_data );

	virtual Result::Name Init() { return Result::SUCCESS; }

	virtual Result::Name SendBufferedInputToInputHandlers() = 0;

	void CheckPressedKeys();

	virtual CForceFeedbackEffect CreateForceFeedbackEffect( const CForceFeedbackEffectDesc& desc );

	virtual Result::Name InitForceFeedbackEffect( CDIForceFeedbackEffectImpl& impl ) { return Result::UNKNOWN_ERROR; }

	virtual void GetStatus( std::vector<std::string>& buffer ) {}

	friend class InputDeviceHub;
};



/// Used as a singleton class
/**
 *  
 *
 */
class InputDeviceHub
{
	/**
	 * Each input device object registers itself to this list from its ctor when it is created.
	 *
	 */
	std::vector<InputDevice *> m_vecpInputDevice;

	/**
	 * In addition to the list above, each input device object registers itself to one of the
	 * input device groups in this list.
	 *
	 */
	 std::vector< std::shared_ptr<InputDeviceGroup> > m_vecpGroup;

	std::mutex m_Mutex;

public:

	InputDeviceHub();

	void RegisterInputDevice( InputDevice *pDevice );

	void UnregisterInputDevice( InputDevice *pDevice );

	void RegisterInputDeviceToGroup( InputDevice *pDevice );

	void UnregisterInputDeviceFromGroup( InputDevice *pDevice );

	void SendInputToInputHandlers();

	void SendAutoRepeat();

	void SendAutoRepeat( InputDeviceGroup& group );

	std::shared_ptr<InputDeviceGroup> GetInputDeviceGroup( int i ) { return m_vecpGroup[i]; }

	int GetNumInputDeviceGroups() const { return (int)m_vecpGroup.size(); }

	void GetInputDeviceStatus( std::vector<std::string>& dest_text_buffer );
};


inline InputDeviceHub& GetInputDeviceHub()
{
	static InputDeviceHub s_instance;
	return s_instance;
}


template<class InputDeviceClass>
inline InputDeviceClass *GetPrimaryInputDevice()
{
	std::shared_ptr<InputDeviceGroup> pGroup = GetInputDeviceHub().GetInputDeviceGroup(0);
	if( !pGroup )
		return nullptr;

	const std::vector<InputDevice *>& pInputDevices = pGroup->GetInputDevices();
	for( size_t i=0; i<pInputDevices.size(); i++ )
	{
		InputDeviceClass *pDest = dynamic_cast<InputDeviceClass *>( pInputDevices[i] );
		if( !pDest )
			continue;

		return pDest;
	}

	return nullptr;
}


} // namespace amorphous



#endif  /*  __InputDevice_H__  */
