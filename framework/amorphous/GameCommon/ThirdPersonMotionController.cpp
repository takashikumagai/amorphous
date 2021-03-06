#include "ThirdPersonMotionController.hpp"
#include "amorphous/3DMath/3DGameMath.hpp"
#include "amorphous/Item/SkeletalCharacter.hpp"
#include "amorphous/Input/InputHandler.hpp"


namespace amorphous
{



ThirdPersonMotionController::ThirdPersonMotionController()
:
m_fVeritcalCameraSpaceInput(0),
m_fHorizontalCameraSpaceInput(0)
{
}


/// Returns a vector extended to the borders of the box which has the dimension of x:[-1,1] / y[-1,1]
Vector2 GetExtendedInputVector( const Vector2& vInput )
{
	if( fabs(vInput.x) < fabs(vInput.y) )
	{
		// clip at y=1 / y=-1 border
		if( 0 < vInput.y )
			return Vector2( vInput.x / vInput.y, 1 );
		else
			return Vector2( vInput.x / vInput.y,-1 );
	}
	else
	{
		// clip at x=1 / x=-1 border
		if( 0 < vInput.x )
			return Vector2( 1, vInput.y / vInput.x );
		else
			return Vector2( -1, vInput.y / vInput.x );
	}
}


void ThirdPersonMotionController::Update()
{
	if( !m_pCharacter )
		return;

	SkeletalCharacter& skeletal_character = *m_pCharacter;

//	if( !skeletal_character.IsCameraDependentMotionControlEnabled() )
//		return;

	if( fabsf( m_fHorizontalCameraSpaceInput ) < 0.1f
	 && fabsf( m_fVeritcalCameraSpaceInput ) < 0.1f )
	{
		skeletal_character.SetFwdSpeed( 0.0f );
		skeletal_character.SetTurnSpeed( 0.0f );
		skeletal_character.SetDesiredHorizontalDirection( Vector3(0,0,0) );
		return;
	}

	Vector3 vInput = Vector3( m_fHorizontalCameraSpaceInput, 0, m_fVeritcalCameraSpaceInput );
	const float input_vector_length = vInput.GetLength();
	const Vector3 vInputDirection = vInput / input_vector_length;
	Vector2 vExtended = GetExtendedInputVector( Vector2(vInput.x,vInput.z) );
	vInput = vInput / vExtended.GetLength();

//	if( 1.0f < vInput.GetLength() )
//		vInput = vInput / GetLength();

	EntityHandle<ItemEntity> entity = m_pCharacter->GetItemEntity();
	shared_ptr<ItemEntity> pEntity = entity.Get();
	if( !pEntity )
		return;

	Matrix33 character_orient = pEntity->GetWorldPose().matOrient;
	if( character_orient.GetColumn(1).GetLength() < 0.001f )
		return;

	Horizontalize( character_orient );

	Matrix33 cam_orient = m_CameraPose.matOrient;// Camera().GetPose().matOrient;// m_CameraOrientation.current.ToRotationMatrix();

	Horizontalize( cam_orient );

	// Calculate the desired direction in world space
	Vector3 vWorldDesiredDirection = cam_orient * vInputDirection;

	skeletal_character.SetDesiredHorizontalDirection( vWorldDesiredDirection );

	// gamepad analog axis-x/y: [0,2]
	// keyboad up/down/left/right: [0,1]
	float fwd_speed = vInput.GetLength() * 0.5f;
	if(skeletal_character.GetActionInputState(ACTION_MOV_BOOST) == CInputState::PRESSED)
		fwd_speed *= 2.0f;
	clamp( fwd_speed, 0.0f, 1.0f );

//	float speed = ( 0.1f < GetActionInput(*m_pKeyBind,ACTION_MOV_BOOST) ) ? 1.0f : 0.5f;

	if( 0.75f < fwd_speed )
		int boost_input_detected = 1;

	skeletal_character.SetFwdSpeed( fwd_speed );
}


float GetAnalogMotionInput( const InputData& input, float sign )
{
	if( input.iType == ITYPE_KEY_PRESSED
	 || input.iType == ITYPE_VALUE_CHANGED )
	{
		return input.fParam1 * sign;
	}
	else if( input.iType == ITYPE_KEY_RELEASED )
	{
		return 0;
	}

	return 0;
}


void ThirdPersonMotionController::HandleInput( int action_code, const InputData& input )
{
	switch( action_code )
	{
	case ACTION_MOV_FORWARD:
		LOG_PRINT( " ACTION_MOV_FORWARD: " + to_string(input.fParam1) );
		m_fVeritcalCameraSpaceInput = GetAnalogMotionInput( input, 1.0f );
		break;

	case ACTION_MOV_BACKWARD:
		LOG_PRINT( " ACTION_MOV_BACKWARD: " + to_string(input.fParam1) );
		m_fVeritcalCameraSpaceInput = GetAnalogMotionInput( input, -1.0f );
		break;

	case ACTION_MOV_TURN_R:
		m_fHorizontalCameraSpaceInput = GetAnalogMotionInput( input, 1.0f );
		break;

	case ACTION_MOV_TURN_L:
		m_fHorizontalCameraSpaceInput = GetAnalogMotionInput( input, -1.0f );
		break;

	default:
		break;
	}
}


} // namespace amorphous
