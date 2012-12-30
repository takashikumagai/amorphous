#include "2DRectSet.hpp"
#include <boost/shared_ptr.hpp>


namespace amorphous
{


using namespace boost;


C2DRectSet::C2DRectSet()
{
	m_pImpl = Ref2DPrimitiveFactory().Create2DRectSetImpl();
}


} // namespace amorphous
