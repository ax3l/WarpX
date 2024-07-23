.. _building-lonestar6:

Lonestar6 (TACC)
================

The `Lonestar6 A100 GPU cluster <https://tacc.utexas.edu/systems/lonestar6/>`__ is located at TACC.


Introduction
------------

If you are new to this system, **please see the following resources**:

* `Lonestar6 user guide <https://docs.tacc.utexas.edu/hpc/lonestar6/>`__
* Batch system: `Slurm <https://docs.tacc.utexas.edu/hpc/lonestar6/#running>`__
* `Jupyter service <https://tac.tacc.utexas.edu>`__ (`documentation <https://docs.tacc.utexas.edu/hpc/lonestar6/#vis>`__, login required)
* `Production directories <https://docs.tacc.utexas.edu/hpc/lonestar6/#files>`__:

  * ``$HOME``: personal directory on the parallel filesystem (Note that the ``$HOME`` directory is *not* suitable for production quality data generation.)
  * ``$WORK``: fastm for all but the largest jobs
  * ``$SCRATCH``: fast for large jobs, purged after 10 days


.. _building-lonestar6-preparation:

Preparation
-----------

Use the following commands to download the WarpX source code:

.. code-block:: bash

   git clone https://github.com/ECP-WarpX/WarpX.git $HOME/src/warpx

We use system software modules, add environment hints and further dependencies via the file ``$HOME/lonestar6_a100_warpx.profile``.
Create it now:

.. code-block:: bash

   cp $HOME/src/warpx/Tools/machines/lonestar6-tacc/lonestar6_a100_warpx.profile.example $HOME/lonestar6_a100_warpx.profile

.. dropdown:: Script Details
   :color: light
   :icon: info
   :animate: fade-in-slide-down

   .. literalinclude:: ../../../../Tools/machines/lonestar6-tacc/lonestar6_a100_warpx.profile.example
      :language: bash

Edit the 2nd line of this script, which sets the ``export proj=""`` variable.
For example, if you are member of the project ``nsldt``, then run ``vi $HOME/lonestar6_a100_warpx.profile``.
Enter the edit mode by typing ``i`` and edit line 2 to read:

.. code-block:: bash

   export proj="nsldt"

Exit the ``vi`` editor with ``Esc`` and then type ``:wq`` (write & quit).

.. important::

   Now, and as the first step on future logins to lonestar6, activate these environment settings:

   .. code-block:: bash

      source $HOME/lonestar6_a100_warpx.profile

Finally, since lonestar6 does not yet provide software modules for some of our dependencies, install them once:

.. code-block:: bash

   bash $HOME/src/warpx/Tools/machines/lonestar6-tacc/install_a100_dependencies.sh
   source /usr/workspace/${USER}/lonestar6/gpu/venvs/warpx-lonestar6-a100/bin/activate

.. dropdown:: Script Details
   :color: light
   :icon: info
   :animate: fade-in-slide-down

   .. literalinclude:: ../../../../Tools/machines/lonestar6-tacc/install_a100_dependencies.sh
      :language: bash


.. _building-lonestar6-compilation:

Compilation
-----------

Use the following :ref:`cmake commands <building-cmake>` to compile the application executable:

.. code-block:: bash

   cd $HOME/src/warpx
   rm -rf build_lonestar6

   cmake -S . -B build_lonestar6 -DWarpX_COMPUTE=CUDA -DWarpX_FFT=ON -DWarpX_QED_TABLE_GEN=ON -DWarpX_DIMS="1;2;RZ;3"
   cmake --build build_lonestar6 -j 8

The WarpX application executables are now in ``$HOME/src/warpx/build_lonestar6/bin/``.
Additionally, the following commands will install WarpX as a Python module:

.. code-block:: bash

   rm -rf build_lonestar6_py

   cmake -S . -B build_lonestar6_py -DWarpX_COMPUTE=CUDA -DWarpX_FFT=ON -DWarpX_QED_TABLE_GEN=ON -DWarpX_APP=OFF -DWarpX_PYTHON=ON -DWarpX_DIMS="1;2;RZ;3"
   cmake --build build_lonestar6_py -j 8 --target pip_install

Now, you can :ref:`submit lonestar6 compute jobs <running-cpp-lonestar6>` for WarpX :ref:`Python (PICMI) scripts <usage-picmi>` (:ref:`example scripts <usage-examples>`).
Or, you can use the WarpX executables to submit lonestar6 jobs (:ref:`example inputs <usage-examples>`).
For executables, you can reference their location in your :ref:`job script <running-cpp-lonestar6>` or copy them to a location in ``$PROJWORK/$proj/``.


.. _building-lonestar6-update:

Update WarpX & Dependencies
---------------------------

If you already installed WarpX in the past and want to update it, start by getting the latest source code:

.. code-block:: bash

   cd $HOME/src/warpx

   # read the output of this command - does it look ok?
   git status

   # get the latest WarpX source code
   git fetch
   git pull

   # read the output of these commands - do they look ok?
   git status
   git log     # press q to exit

And, if needed,

- :ref:`update the lonestar6_a100_warpx.profile file <building-lonestar6-preparation>`,
- log out and into the system, activate the now updated environment profile as usual,
- :ref:`execute the dependency install scripts <building-lonestar6-preparation>`.

As a last step, clean the build directory ``rm -rf $HOME/src/warpx/build_lonestar6`` and rebuild WarpX.


.. _running-cpp-lonestar6:

Running
-------

.. _running-cpp-lonestar6-a100-GPUs:

A100 GPUs (40GB)
^^^^^^^^^^^^^^^^

Lonestar6 has two A100 GPUs per node with 40GB memory each.

The batch script below can be used to run a WarpX simulation on 2 nodes on the supercomputer lonestar6 at TACC.
Replace descriptions between chevrons ``<>`` by relevant values, for instance ``<input file>`` could be ``plasma_mirror_inputs``.
Note that the only option so far is to run with one MPI rank per GPU.

.. literalinclude:: ../../../../Tools/machines/lonestar6-tacc/lonestar6_a100.sbatch
   :language: bash
   :caption: You can copy this file from ``Tools/machines/lonestar6-tacc/lonestar6_a100.sbatch``.

To run a simulation, copy the lines above to a file ``lonestar6_a100.sbatch`` and run

.. code-block:: bash

   bsub lonestar6_a100.bsub

to submit the job.
