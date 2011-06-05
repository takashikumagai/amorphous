#ifndef __InputDeviceGroup_HPP__
#define __InputDeviceGroup_HPP__


#include "fwd.hpp"
#include "InputHandler.hpp"

/// auto repeat control requires the timer
#include "gds/Support/Timer.hpp"
#include "gds/Support/FixedVector.hpp"


class CInputDeviceGroup
{
	std::vector<CInputDevice *> m_vecpDevice;

	CInputState m_aInputState[NUM_GENERAL_INPUT_CODES];

	TCFixedVector<int,CInputDeviceParam::NUM_MAX_SIMULTANEOUS_PRESSES> m_PressedKeyList;

public:

	CInputDeviceGroup() {}

	~CInputDeviceGroup() {}

	CInputState::Name GetInputState( int gi_code ) const { return m_aInputState[gi_code].m_State; }

	std::vector<CInputDevice *>& InputDevice() { return m_vecpDevice; }

	friend class CInputDevice;
	friend class CInputDeviceHub;
};


#endif  /*  __InputDeviceGroup_HPP__  */
