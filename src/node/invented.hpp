#ifndef INVENTED_HPP
#define INVENTED_HPP

#include "common/common.hpp"
#include "common/progress.hpp"

/// Track the "invention" of nodes. Invention propagates through 
/// clone and duplicate but does not appear in an itemisation.
/// It provides a "generation number" mechanism. No _FUNCTION
/// method is required.
class Invented
{
public: 
    Invented() :
		step(Progress::NO_STEP)
	{
	}
	
	void Invent()
	{
		ASSERT( step == Progress::NO_STEP );
		step = Progress::GetCurrent().GetStep();
		ASSERT( step != Progress::NO_STEP );				
	}
	
	bool IsOld() const
	{
		return (Progress::GetCurrent().GetStep() > step);
	}

	int step;
};

#endif
