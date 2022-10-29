###############################################################################
# These are the files equivalent to the primary C++ implementation - but here
# we define how they will appear in our Python module (aka Python bindings).
#
target_sources(WarpX
  PRIVATE
    WarpX_py.cpp  # TODO: modernize via https://pybind11.readthedocs.io/en/stable/advanced/cast/functional.html
)
target_sources(pyWarpX
  PRIVATE
    WarpX.cpp
)
