#ifndef  __GraphicsApplicationBase_HPP__
#define  __GraphicsApplicationBase_HPP__


#include <boost/shared_ptr.hpp>
#include "ApplicationBase.hpp"
#include "Graphics/fwd.hpp"
#include "Graphics/Camera.hpp"
#include "Support/CameraController.hpp"


template<class T>
class CInputDataDelegate : public CInputHandler
{
	T *m_pTarget;
public:
	CInputDataDelegate(T *pTarget) : m_pTarget(pTarget) {}

	void ProcessInput( SInputData& input )
	{
		m_pTarget->HandleInput( input );
	}
};



/**
 Base class for graphics application
 - This class is for state-less application that uses a window and graphics.
 */
class CGraphicsApplicationBase : public CApplicationBase
{
	CCamera m_Camera;

//	CPlatformDependentCameraController m_CameraController;
	boost::shared_ptr<CCameraControllerBase> m_pCameraController;

	boost::shared_ptr<CFontBase> m_pFont;

	boost::shared_ptr<CInputHandler> m_pInputHandler;

private:

//	void Execute();
//	void InitDebugItems();
//	void ReleaseDebugItems();

	virtual const std::string GetApplicationTitle() { return "Graphics Application"; }

	virtual int Init() { return 0; }

	void UpdateCameraMatrices();

	void UpdateFrame();

	// Override this to implement the rendering routine
	virtual void Render() = 0;

	virtual void Update( float dt ) {}

	virtual void HandleInput( const SInputData& input ) {}

	void RenderBase();

protected:

	bool m_UseCameraController;

	boost::shared_ptr<CCameraControllerBase> GetCameraController() { return m_pCameraController; }

	CCamera& Camera() { return m_Camera; }

public:

	CGraphicsApplicationBase();

	virtual ~CGraphicsApplicationBase();

//	bool InitBase();

//	void Release();

//	void AcquireInputDevices();

//	static void SetDefaultSleepTime( int sleep_time_in_ms ) { ms_DefaultSleepTimeMS = sleep_time_in_ms; }

	void Run();

	friend class CInputDataDelegate<CGraphicsApplicationBase>;
};


#endif		/*  __GraphicsApplicationBase_HPP__  */