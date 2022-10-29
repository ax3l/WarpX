/* Copyright 2019-2022 The WarpX Community
 *
 * This file is part of WarpX.
 *
 * Authors: David Grote, Maxence Thevenet, Weiqun Zhang, Roelof Groenewald
 *
 * License: BSD-3-Clause-LBNL
 */
//#include "WarpXWrappers.H"
#include "Utils/WarpXProfilerWrapper.H"

#include <map>
#include <string>

/**
 * Declare global map to hold python callback functions.
 *
 * The keys of the map describe at what point in the simulation the python
 * functions will be called. Currently supported keys (callback points) are
 * afterinit, beforecollisions, aftercollisions, beforeEsolve, poissonsolver,
 * afterEsolve, beforedeposition, afterdeposition, particlescraper,
 * particleloader, beforestep, afterstep, afterdiagnostics, afterrestart,
 * oncheckpointsignal, particleinjection and appliedfields.
*/
extern std::map< std::string, std::function<()> > warpx_callback_py_map;

/**
 * \brief Function to check if the given name is a key in warpx_callback_py_map
 */
bool IsPythonCallBackInstalled ( std::string name );

/**
 * \brief Function to look for and execute Python callbacks
 */
void ExecutePythonCallback ( std::string name );