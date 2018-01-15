#ifndef  __amorphous_ApplicationCore_HPP__
#define  __amorphous_ApplicationCore_HPP__


namespace amorphous
{


class ApplicationCore
{
public:

    virtual ~ApplicationCore() {}

	virtual bool IsAppExitRequested() const = 0;

	// Called every frame in the main loop
	// The application needs to both update the application states and render the scene.
	virtual void UpdateFrame() = 0;
};


} // namespace amorphous


#endif		/*  __amorphous_ApplicationCore_HPP__  */
