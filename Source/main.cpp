
#include <iostream>

#include <AMReX_BoxLib.H>
#include <AMReX_BLProfiler.H>
#include <AMReX_ParallelDescriptor.H>

#include <WarpX.H>

int main(int argc, char* argv[])
{
    BoxLib::Initialize(argc,argv);

    BL_PROFILE_VAR("main()", pmain);

    const Real strt_total = ParallelDescriptor::second();

    {
	WarpX warpx;
	
	warpx.InitData();

	warpx.Evolve();
	
	Real end_total = ParallelDescriptor::second() - strt_total;
	
	ParallelDescriptor::ReduceRealMax(end_total ,ParallelDescriptor::IOProcessorNumber());
	if (warpx.Verbose() && ParallelDescriptor::IOProcessor()) {
	    std::cout << "Total Time                     : " << end_total << '\n';
	}
    }

    BL_PROFILE_VAR_STOP(pmain);

    BoxLib::Finalize();
}
