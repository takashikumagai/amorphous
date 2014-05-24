#include "CameraController.hpp"
#include "Input/InputDevice.hpp"


namespace amorphous
{


CameraController::CameraController( int input_handler_index )
:
m_InputHandlerIndex(input_handler_index)
{
	m_pInputDataDelagate.reset( new CInputDataDelegate<CameraController>(this) );

	if( GetInputHub().GetInputHandler(input_handler_index) )
		GetInputHub().GetInputHandler(input_handler_index)->AddChild( m_pInputDataDelagate.get() );
	else
		GetInputHub().PushInputHandler( input_handler_index, m_pInputDataDelagate.get() );
}


CameraController::CameraController( InputHandler *pParentInputHandler )
{
	m_pInputDataDelagate.reset( new CInputDataDelegate<CameraController>(this) );

	if( !pParentInputHandler )
		return;

	pParentInputHandler->AddChild( m_pInputDataDelagate.get() );
}


CameraController::~CameraController()
{
	GetInputHub().RemoveInputHandler( m_InputHandlerIndex, m_pInputDataDelagate.get() );
}


bool CameraController::IsKeyPressed( int general_input_code )
{
	bool res = CameraControllerBase::IsKeyPressed( general_input_code );
	if( res )
		return true;

	if( !IsValidGeneralInputCode( general_input_code ) )
		return false;

//	return ( GetInputHub().GetInputState(general_input_code) == CInputState::PRESSED );
	return ( GetInputDeviceHub().GetInputDeviceGroup(0)->GetInputState(general_input_code) == CInputState::PRESSED );
}


// Need to avoid calling this when mouse operation is notified by Win32 message
// - See CameraController_Win32.cpp
//void CameraControllerInputHandler::HandleInput( const InputData& input )
//{
//}


} // namespace amorphous