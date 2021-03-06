/* Copyright 2019 Axel Huebl, Weiqun Zhang
 *
 * This file is part of WarpX.
 *
 * License: BSD-3-Clause-LBNL
 */
#ifndef WARPX_COMM_K_H_
#define WARPX_COMM_K_H_

#include <AMReX_FArrayBox.H>
#include <AMReX.H>

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp (int j, int k, int l,
                   amrex::Array4<amrex::Real      > const& arr_aux,
                   amrex::Array4<amrex::Real const> const& arr_fine,
                   amrex::Array4<amrex::Real const> const& arr_coarse,
                   const amrex::IntVect& arr_stag,
                   const amrex::IntVect& rr)
{
    using namespace amrex;

    // NOTE Indices (j,k,l) in the following refer to (x,z,-) in 2D and (x,y,z) in 3D

    // Refinement ratio
    const int rj = rr[0];
    const int rk = rr[1];
    const int rl = (AMREX_SPACEDIM == 2) ? 1 : rr[2];

    // Staggering (0: cell-centered; 1: nodal)
    const int sj = arr_stag[0];
    const int sk = arr_stag[1];
    const int sl = (AMREX_SPACEDIM == 2) ? 0 : arr_stag[2];

    // Number of points used for interpolation from coarse grid to fine grid
    const int nj = (sj == 0) ? 1 : 2;
    const int nk = (sk == 0) ? 1 : 2;
    const int nl = (sl == 0) ? 1 : 2;

    const int jc = amrex::coarsen(j, rj);
    const int kc = amrex::coarsen(k, rk);
    const int lc = amrex::coarsen(l, rl);

    amrex::Real wj;
    amrex::Real wk;
    amrex::Real wl;

    // Interpolate from coarse grid to fine grid using either 1 point with weight 1, if both grids
    // are cell-centered, or 2 points with weights depending on the distance, if both grids are nodal
    amrex::Real res = 0.0_rt;
    for         (int jj = 0; jj < nj; jj++) {
        for     (int kk = 0; kk < nk; kk++) {
            for (int ll = 0; ll < nl; ll++) {
                wj = (sj == 0) ? 1.0_rt : (rj - amrex::Math::abs(j - (jc + jj) * rj))
                                          / static_cast<amrex::Real>(rj);
                wk = (sk == 0) ? 1.0_rt : (rk - amrex::Math::abs(k - (kc + kk) * rk))
                                          / static_cast<amrex::Real>(rk);
                wl = (sl == 0) ? 1.0_rt : (rl - amrex::Math::abs(l - (lc + ll) * rl))
                                          / static_cast<amrex::Real>(rl);
                res += wj * wk * wl * arr_coarse(jc+jj,kc+kk,lc+ll);
            }
        }
    }
    arr_aux(j,k,l) = arr_fine(j,k,l) + res;
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp_nd_bfield_x (int j, int k, int l,
                               amrex::Array4<amrex::Real> const& Bxa,
                               amrex::Array4<amrex::Real const> const& Bxf,
                               amrex::Array4<amrex::Real const> const& Bxc,
                               amrex::Array4<amrex::Real const> const& Bxg)
{
    using namespace amrex;

    int jg = amrex::coarsen(j,2);
    Real wx = (j == jg*2) ? 0.0_rt : 0.5_rt;
    Real owx = 1.0_rt-wx;

    int kg = amrex::coarsen(k,2);
    Real wy = (k == kg*2) ? 0.0_rt : 0.5_rt;
    Real owy = 1.0_rt-wy;

#if (AMREX_SPACEDIM == 2)

    // interp from coarse nodal to fine nodal
    Real bg = owx * owy * Bxg(jg  ,kg  ,0)
        +     owx *  wy * Bxg(jg  ,kg+1,0)
        +      wx * owy * Bxg(jg+1,kg  ,0)
        +      wx *  wy * Bxg(jg+1,kg+1,0);

    // interp from coarse staggered to fine nodal
    wy = 0.5_rt-wy;  owy = 1.0_rt-wy;
    Real bc = owx * owy * Bxc(jg  ,kg  ,0)
        +     owx *  wy * Bxc(jg  ,kg-1,0)
        +      wx * owy * Bxc(jg+1,kg  ,0)
        +      wx *  wy * Bxc(jg+1,kg-1,0);

    // interp from fine staggered to fine nodal
    Real bf = 0.5_rt*(Bxf(j,k-1,0) + Bxf(j,k,0));

#else

    int lg = amrex::coarsen(l,2);
    Real wz = (l == lg*2) ? 0.0_rt : 0.5_rt;
    Real owz = 1.0_rt-wz;

    // interp from coarse nodal to fine nodal
    Real bg = owx * owy * owz * Bxg(jg  ,kg  ,lg  )
        +      wx * owy * owz * Bxg(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Bxg(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Bxg(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Bxg(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Bxg(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Bxg(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Bxg(jg+1,kg+1,lg+1);

    // interp from coarse staggered to fine nodal
    wy = 0.5_rt-wy;  owy = 1.0_rt-wy;
    wz = 0.5_rt-wz;  owz = 1.0_rt-wz;
    Real bc = owx * owy * owz * Bxc(jg  ,kg  ,lg  )
        +      wx * owy * owz * Bxc(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Bxc(jg  ,kg-1,lg  )
        +      wx *  wy * owz * Bxc(jg+1,kg-1,lg  )
        +     owx * owy *  wz * Bxc(jg  ,kg  ,lg-1)
        +      wx * owy *  wz * Bxc(jg+1,kg  ,lg-1)
        +     owx *  wy *  wz * Bxc(jg  ,kg-1,lg-1)
        +      wx *  wy *  wz * Bxc(jg+1,kg-1,lg-1);

    // interp from fine stagged to fine nodal
    Real bf = 0.25_rt*(Bxf(j,k-1,l-1) + Bxf(j,k,l-1) + Bxf(j,k-1,l) + Bxf(j,k,l));
#endif

    Bxa(j,k,l) = bg + (bf-bc);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp_nd_bfield_y (int j, int k, int l,
                               amrex::Array4<amrex::Real> const& Bya,
                               amrex::Array4<amrex::Real const> const& Byf,
                               amrex::Array4<amrex::Real const> const& Byc,
                               amrex::Array4<amrex::Real const> const& Byg)
{
    using namespace amrex;

    int jg = amrex::coarsen(j,2);
    Real wx = (j == jg*2) ? 0.0_rt : 0.5_rt;
    Real owx = 1.0_rt-wx;

    int kg = amrex::coarsen(k,2);
    Real wy = (k == kg*2) ? 0.0_rt : 0.5_rt;
    Real owy = 1.0_rt-wy;

#if (AMREX_SPACEDIM == 2)

    // interp from coarse nodal to fine nodal
    Real bg = owx * owy * Byg(jg  ,kg  ,0)
        +     owx *  wy * Byg(jg  ,kg+1,0)
        +      wx * owy * Byg(jg+1,kg  ,0)
        +      wx *  wy * Byg(jg+1,kg+1,0);

    // interp from coarse stagged (cell-centered for By) to fine nodal
    wx = 0.5_rt-wx;  owx = 1.0_rt-wx;
    wy = 0.5_rt-wy;  owy = 1.0_rt-wy;
    Real bc = owx * owy * Byc(jg  ,kg  ,0)
        +     owx *  wy * Byc(jg  ,kg-1,0)
        +      wx * owy * Byc(jg-1,kg  ,0)
        +      wx *  wy * Byc(jg-1,kg-1,0);

    // interp form fine stagger (cell-centered for By) to fine nodal
    Real bf = 0.25_rt*(Byf(j,k,0) + Byf(j-1,k,0) + Byf(j,k-1,0) + Byf(j-1,k-1,0));

#else

    int lg = amrex::coarsen(l,2);
    Real wz = (l == lg*2) ? 0.0_rt : 0.5_rt;
    Real owz = 1.0_rt-wz;

    // interp from coarse nodal to fine nodal
    Real bg = owx * owy * owz * Byg(jg  ,kg  ,lg  )
        +      wx * owy * owz * Byg(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Byg(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Byg(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Byg(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Byg(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Byg(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Byg(jg+1,kg+1,lg+1);

    // interp from coarse staggered to fine nodal
    wx = 0.5_rt-wx;  owx = 1.0_rt-wx;
    wz = 0.5_rt-wz;  owz = 1.0_rt-wz;
    Real bc = owx * owy * owz * Byc(jg  ,kg  ,lg  )
        +      wx * owy * owz * Byc(jg-1,kg  ,lg  )
        +     owx *  wy * owz * Byc(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Byc(jg-1,kg+1,lg  )
        +     owx * owy *  wz * Byc(jg  ,kg  ,lg-1)
        +      wx * owy *  wz * Byc(jg-1,kg  ,lg-1)
        +     owx *  wy *  wz * Byc(jg  ,kg+1,lg-1)
        +      wx *  wy *  wz * Byc(jg-1,kg+1,lg-1);

    // interp from fine stagged to fine nodal
    Real bf = 0.25_rt*(Byf(j-1,k,l-1) + Byf(j,k,l-1) + Byf(j-1,k,l) + Byf(j,k,l));

#endif

    Bya(j,k,l) = bg + (bf-bc);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp_nd_bfield_z (int j, int k, int l,
                               amrex::Array4<amrex::Real> const& Bza,
                               amrex::Array4<amrex::Real const> const& Bzf,
                               amrex::Array4<amrex::Real const> const& Bzc,
                               amrex::Array4<amrex::Real const> const& Bzg)
{
    using namespace amrex;

    int jg = amrex::coarsen(j,2);
    Real wx = (j == jg*2) ? 0.0_rt : 0.5_rt;
    Real owx = 1.0_rt-wx;

    int kg = amrex::coarsen(k,2);
    Real wy = (k == kg*2) ? 0.0_rt : 0.5_rt;
    Real owy = 1.0_rt-wy;

#if (AMREX_SPACEDIM == 2)

    // interp from coarse nodal to fine nodal
    Real bg = owx * owy * Bzg(jg  ,kg  ,0)
        +     owx *  wy * Bzg(jg  ,kg+1,0)
        +      wx * owy * Bzg(jg+1,kg  ,0)
        +      wx *  wy * Bzg(jg+1,kg+1,0);

    // interp from coarse staggered to fine nodal
    wx = 0.5_rt-wx;  owx = 1.0_rt-wx;
    Real bc = owx * owy * Bzc(jg  ,kg  ,0)
        +     owx *  wy * Bzc(jg  ,kg+1,0)
        +      wx * owy * Bzc(jg-1,kg  ,0)
        +      wx *  wy * Bzc(jg-1,kg+1,0);

    // interp from fine staggered to fine nodal
    Real bf = 0.5_rt*(Bzf(j-1,k,0) + Bzf(j,k,0));

#else

    int lg = amrex::coarsen(l,2);
    Real wz = (l == lg*2) ? 0.0_rt : 0.5_rt;
    Real owz = 1.0_rt-wz;

    // interp from coarse nodal to fine nodal
    Real bg = owx * owy * owz * Bzg(jg  ,kg  ,lg  )
        +      wx * owy * owz * Bzg(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Bzg(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Bzg(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Bzg(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Bzg(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Bzg(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Bzg(jg+1,kg+1,lg+1);

    // interp from coarse staggered to fine nodal
    wx = 0.5_rt-wx;  owx = 1.0_rt-wx;
    wy = 0.5_rt-wy;  owy = 1.0_rt-wy;
    Real bc = owx * owy * owz * Bzc(jg  ,kg  ,lg  )
        +      wx * owy * owz * Bzc(jg-1,kg  ,lg  )
        +     owx *  wy * owz * Bzc(jg  ,kg-1,lg  )
        +      wx *  wy * owz * Bzc(jg-1,kg-1,lg  )
        +     owx * owy *  wz * Bzc(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Bzc(jg-1,kg  ,lg+1)
        +     owx *  wy *  wz * Bzc(jg  ,kg-1,lg+1)
        +      wx *  wy *  wz * Bzc(jg-1,kg-1,lg+1);

    // interp from fine stagged to fine nodal
    Real bf = 0.25_rt*(Bzf(j-1,k-1,l) + Bzf(j,k-1,l) + Bzf(j-1,k,l) + Bzf(j,k,l));

#endif

    Bza(j,k,l) = bg + (bf-bc);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp_nd_efield_x (int j, int k, int l,
                               amrex::Array4<amrex::Real> const& Exa,
                               amrex::Array4<amrex::Real const> const& Exf,
                               amrex::Array4<amrex::Real const> const& Exc,
                               amrex::Array4<amrex::Real const> const& Exg)
{
    using namespace amrex;

    int jg = amrex::coarsen(j,2);
    Real wx = (j == jg*2) ? 0.0_rt : 0.5_rt;
    Real owx = 1.0_rt-wx;

    int kg = amrex::coarsen(k,2);
    Real wy = (k == kg*2) ? 0.0_rt : 0.5_rt;
    Real owy = 1.0_rt-wy;

#if (AMREX_SPACEDIM == 2)

    // interp from coarse nodal to fine nodal
    Real eg = owx * owy * Exg(jg  ,kg  ,0)
        +     owx *  wy * Exg(jg  ,kg+1,0)
        +      wx * owy * Exg(jg+1,kg  ,0)
        +      wx *  wy * Exg(jg+1,kg+1,0);

    // interp from coarse staggered to fine nodal
    wx = 0.5_rt-wx;  owx = 1.0_rt-wx;
    Real ec = owx * owy * Exc(jg  ,kg  ,0)
        +     owx *  wy * Exc(jg  ,kg+1,0)
        +      wx * owy * Exc(jg-1,kg  ,0)
        +      wx *  wy * Exc(jg-1,kg+1,0);

    // interp from fine staggered to fine nodal
    Real ef = 0.5_rt*(Exf(j-1,k,0) + Exf(j,k,0));

#else

    int lg = amrex::coarsen(l,2);
    Real wz = (l == lg*2) ? 0.0 : 0.5;
    Real owz = 1.0_rt-wz;

    // interp from coarse nodal to fine nodal
    Real eg = owx * owy * owz * Exg(jg  ,kg  ,lg  )
        +      wx * owy * owz * Exg(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Exg(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Exg(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Exg(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Exg(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Exg(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Exg(jg+1,kg+1,lg+1);

    // interp from coarse staggered to fine nodal
    wx = 0.5_rt-wx;  owx = 1.0_rt-wx;
    Real ec = owx * owy * owz * Exc(jg  ,kg  ,lg  )
        +      wx * owy * owz * Exc(jg-1,kg  ,lg  )
        +     owx *  wy * owz * Exc(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Exc(jg-1,kg+1,lg  )
        +     owx * owy *  wz * Exc(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Exc(jg-1,kg  ,lg+1)
        +     owx *  wy *  wz * Exc(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Exc(jg-1,kg+1,lg+1);

    // interp from fine staggered to fine nodal
    Real ef = 0.5_rt*(Exf(j-1,k,l) + Exf(j,k,l));

#endif

    Exa(j,k,l) = eg + (ef-ec);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp_nd_efield_y (int j, int k, int l,
                               amrex::Array4<amrex::Real> const& Eya,
                               amrex::Array4<amrex::Real const> const& Eyf,
                               amrex::Array4<amrex::Real const> const& Eyc,
                               amrex::Array4<amrex::Real const> const& Eyg)
{
    using namespace amrex;

    int jg = amrex::coarsen(j,2);
    Real wx = (j == jg*2) ? 0.0_rt : 0.5_rt;
    Real owx = 1.0_rt-wx;

    int kg = amrex::coarsen(k,2);
    Real wy = (k == kg*2) ? 0.0_rt : 0.5_rt;
    Real owy = 1.0_rt-wy;

#if (AMREX_SPACEDIM == 2)

    // interp from coarse nodal to fine nodal
    Real eg = owx * owy * Eyg(jg  ,kg  ,0)
        +     owx *  wy * Eyg(jg  ,kg+1,0)
        +      wx * owy * Eyg(jg+1,kg  ,0)
        +      wx *  wy * Eyg(jg+1,kg+1,0);

    // interp from coarse staggered to fine nodal (Eyc is actually nodal)
    Real ec = owx * owy * Eyc(jg  ,kg  ,0)
        +     owx *  wy * Eyc(jg  ,kg+1,0)
        +      wx * owy * Eyc(jg+1,kg  ,0)
        +      wx *  wy * Eyc(jg+1,kg+1,0);

    // interp from fine staggered to fine nodal
    Real ef = Eyf(j,k,0);

#else

    int lg = amrex::coarsen(l,2);
    Real wz = (l == lg*2) ? 0.0 : 0.5;
    Real owz = 1.0_rt-wz;

    // interp from coarse nodal to fine nodal
    Real eg = owx * owy * owz * Eyg(jg  ,kg  ,lg  )
        +      wx * owy * owz * Eyg(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Eyg(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Eyg(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Eyg(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Eyg(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Eyg(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Eyg(jg+1,kg+1,lg+1);

    // interp from coarse staggered to fine nodal
    wy = 0.5_rt-wy;  owy = 1.0_rt-wy;
    Real ec = owx * owy * owz * Eyc(jg  ,kg  ,lg  )
        +      wx * owy * owz * Eyc(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Eyc(jg  ,kg-1,lg  )
        +      wx *  wy * owz * Eyc(jg+1,kg-1,lg  )
        +     owx * owy *  wz * Eyc(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Eyc(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Eyc(jg  ,kg-1,lg+1)
        +      wx *  wy *  wz * Eyc(jg+1,kg-1,lg+1);

    // interp from fine staggered to fine nodal
    Real ef = 0.5_rt*(Eyf(j,k-1,l) + Eyf(j,k,l));

#endif

    Eya(j,k,l) = eg + (ef-ec);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp_nd_efield_z (int j, int k, int l,
                               amrex::Array4<amrex::Real> const& Eza,
                               amrex::Array4<amrex::Real const> const& Ezf,
                               amrex::Array4<amrex::Real const> const& Ezc,
                               amrex::Array4<amrex::Real const> const& Ezg)
{
    using namespace amrex;

    int jg = amrex::coarsen(j,2);
    Real wx = (j == jg*2) ? 0.0_rt : 0.5_rt;
    Real owx = 1.0_rt-wx;

    int kg = amrex::coarsen(k,2);
    Real wy = (k == kg*2) ? 0.0_rt : 0.5_rt;
    Real owy = 1.0_rt-wy;

#if (AMREX_SPACEDIM == 2)

    // interp from coarse nodal to fine nodal
    Real eg = owx * owy * Ezg(jg  ,kg  ,0)
        +     owx *  wy * Ezg(jg  ,kg+1,0)
        +      wx * owy * Ezg(jg+1,kg  ,0)
        +      wx *  wy * Ezg(jg+1,kg+1,0);

    // interp from coarse stagged to fine nodal
    wy = 0.5_rt-wy;  owy = 1.0_rt-wy;
    Real ec = owx * owy * Ezc(jg  ,kg  ,0)
        +     owx *  wy * Ezc(jg  ,kg-1,0)
        +      wx * owy * Ezc(jg+1,kg  ,0)
        +      wx *  wy * Ezc(jg+1,kg-1,0);

    // interp from fine staggered to fine nodal
    Real ef = 0.5_rt*(Ezf(j,k-1,0) + Ezf(j,k,0));

#else

    int lg = amrex::coarsen(l,2);
    Real wz = (l == lg*2) ? 0.0_rt : 0.5_rt;
    Real owz = 1.0_rt-wz;

    // interp from coarse nodal to fine nodal
    Real eg = owx * owy * owz * Ezg(jg  ,kg  ,lg  )
        +      wx * owy * owz * Ezg(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Ezg(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Ezg(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Ezg(jg  ,kg  ,lg+1)
        +      wx * owy *  wz * Ezg(jg+1,kg  ,lg+1)
        +     owx *  wy *  wz * Ezg(jg  ,kg+1,lg+1)
        +      wx *  wy *  wz * Ezg(jg+1,kg+1,lg+1);

    // interp from coarse staggered to fine nodal
    wz = 0.5_rt-wz;  owz = 1.0_rt-wz;
    Real ec = owx * owy * owz * Ezc(jg  ,kg  ,lg  )
        +      wx * owy * owz * Ezc(jg+1,kg  ,lg  )
        +     owx *  wy * owz * Ezc(jg  ,kg+1,lg  )
        +      wx *  wy * owz * Ezc(jg+1,kg+1,lg  )
        +     owx * owy *  wz * Ezc(jg  ,kg  ,lg-1)
        +      wx * owy *  wz * Ezc(jg+1,kg  ,lg-1)
        +     owx *  wy *  wz * Ezc(jg  ,kg+1,lg-1)
        +      wx *  wy *  wz * Ezc(jg+1,kg+1,lg-1);

    // interp from fine staggered to fine nodal
    Real ef = 0.5_rt*(Ezf(j,k,l-1) + Ezf(j,k,l));

#endif

    Eza(j,k,l) = eg + (ef-ec);
}

/**
 * \brief Arbitrary-order interpolation function used to interpolate a given field from the Yee grid
 * to the nodal grid, before gathering the field from the nodes to the particle positions
 * (momentum-conserving field gathering). With the FDTD solver, this performs simple linear interpolation.
 * With the PSATD solver, this performs arbitrary-order interpolation based on the Fornberg coefficients.
 * The result is stored in the output array \c arr_aux, allocated on the \c aux patch.
 *
 * \param[in] j index along x of the output array
 * \param[in] k index along y (in 3D) or z (in 2D) of the output array
 * \param[in] l index along z (in 3D, \c l = 0 in 2D) of the output array
 * \param[in,out] dst_arr output array allocated on the \c aux patch where interpolated values are stored
 * \param[in] src_arr input array allocated on the fine patch
 * \param[in] src_stag \c IndexType of the input array (Yee staggering)
 * \param[in] nox order of finite-order interpolation along x
 * \param[in] noy order of finite-order interpolation along y
 * \param[in] noz order of finite-order interpolation along z
 * \param[in] stencil_coeffs_x array of Fornberg coefficients for finite-order interpolation stencil along x
 * \param[in] stencil_coeffs_y array of Fornberg coefficients for finite-order interpolation stencil along y
 * \param[in] stencil_coeffs_z array of Fornberg coefficients for finite-order interpolation stencil along z
 */
AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void warpx_interp (const int j,
                   const int k,
                   const int l,
                   amrex::Array4<amrex::Real      > const& dst_arr,
                   amrex::Array4<amrex::Real const> const& src_arr,
                   const amrex::IntVect& src_stag,
                   const int nox = 2,
                   const int noy = 2,
                   const int noz = 2,
                   amrex::Real const* stencil_coeffs_x = nullptr,
                   amrex::Real const* stencil_coeffs_y = nullptr,
                   amrex::Real const* stencil_coeffs_z = nullptr)
{
    using namespace amrex;

    // Pad input array with zeros beyond ghost cells
    // for out-of-bound accesses due to large-stencil operations
    const auto src_arr_zeropad = [src_arr] (const int jj, const int kk, const int ll) noexcept
    {
        return src_arr.contains(jj,kk,ll) ? src_arr(jj,kk,ll) : 0.0_rt;
    };

    // Avoid compiler warnings
#if (AMREX_SPACEDIM == 2)
    amrex::ignore_unused(noy, stencil_coeffs_y);
#endif

    // Avoid compiler warnings
#ifndef WARPX_USE_PSATD
    amrex::ignore_unused(stencil_coeffs_x, stencil_coeffs_y, stencil_coeffs_z);
#endif

    // Staggering (s = 0 if cell-centered, s = 1 if nodal)
    const int sj = src_stag[0];
    const int sk = src_stag[1];
#if (AMREX_SPACEDIM == 3)
    const int sl = src_stag[2];
#endif

    // Number of points (+1) used for interpolation from the staggered grid to the nodal grid
    // (if the input field is nodal along a given direction, then s = 1, the variable n = 0
    // is actually never used, and there is no interpolation, only a copy of one single value)
    const int nj = (sj == 0) ? nox/2 + 1 : 0;
#if   (AMREX_SPACEDIM == 2)
    const int nk = (sk == 0) ? noz/2 + 1 : 0;
#elif (AMREX_SPACEDIM == 3)
    const int nk = (sk == 0) ? noy/2 + 1 : 0;
    const int nl = (sl == 0) ? noz/2 + 1 : 0;
#endif

    // Additional normalization factor
    const amrex::Real wj = (sj == 0) ? 0.5_rt : 1.0_rt;
    const amrex::Real wk = (sk == 0) ? 0.5_rt : 1.0_rt;
#if   (AMREX_SPACEDIM == 2)
    constexpr amrex::Real wl = 1.0_rt;
#elif (AMREX_SPACEDIM == 3)
    const amrex::Real wl = (sl == 0) ? 0.5_rt : 1.0_rt;
#endif

    // Auxiliary variables for stencil coefficients
#ifdef WARPX_USE_PSATD
    amrex::Real cj = 1.0_rt;
    amrex::Real ck = 1.0_rt;
    amrex::Real cl = 1.0_rt;
    int dj, dk;
#if (AMREX_SPACEDIM == 3)
    int dl;
#endif
#endif

    // Min and max for interpolation loop along j
    const int jmin = (sj == 0) ? j - nj + 1 : j;
    const int jmax = (sj == 0) ? j + nj - 2 : j;

    // Min and max for interpolation loop along k
    const int kmin = (sk == 0) ? k - nk + 1 : k;
    const int kmax = (sk == 0) ? k + nk - 2 : k;

    // Min and max for interpolation loop along l
#if   (AMREX_SPACEDIM == 2)
    // l = 0 always
    const int lmin = l;
    const int lmax = l;
#elif (AMREX_SPACEDIM == 3)
    const int lmin = (sl == 0) ? l - nl + 1 : l;
    const int lmax = (sl == 0) ? l + nl - 2 : l;
#endif

    amrex::Real res = 0.0_rt;

#ifndef WARPX_USE_PSATD // FDTD (linear interpolation)

    // Example of 1D interpolation from nodal grid to nodal grid:
    //
    //         j
    // --o-----o-----o--  result(j) = f(j)
    // --o-----o-----o--
    //  j-1    j    j+1
    //
    // Example of 1D interpolation from staggered grid to nodal grid:
    //
    //         j
    // --o-----o-----o--  result(j) = (f(j-1) + f(j)) / 2
    // -----x-----x-----
    //     j-1    j

    for (int ll = lmin; ll <= lmax; ll++)
    {
        for (int kk = kmin; kk <= kmax; kk++)
        {
            for (int jj = jmin; jj <= jmax; jj++)
            {
                res += src_arr_zeropad(jj,kk,ll);
            }
        }
    }

#else // PSATD (finite-order interpolation)

    // Example of 1D interpolation from nodal grid to nodal grid:
    //
    //         j
    // --o-----o-----o--  result(j) = f(j)
    // --o-----o-----o--
    //  j-1    j    j+1
    //
    // Example of 1D interpolation from staggered grid to nodal grid:
    //
    //                     j
    // --o-----o-----o-----o-----o-----o-----o--  result(j) = c_0 * (f(j-1) + f(j)  ) / 2
    // -----x-----x-----x-----x-----x-----x-----            + c_1 * (f(j-2) + f(j+1)) / 2
    //     j-3   j-2   j-1    j    j+1   j+2                + c_2 * (f(j-3) + f(j+2)) / 2
    //     c_2   c_1   c_0   c_0   c_1   c_2                + ...

    for (int ll = lmin; ll <= lmax; ll++)
    {
#if (AMREX_SPACEDIM == 3)
        dl = (l - ll > 0) ? l - ll - 1 : ll - l;
        if (sl == 0) cl = stencil_coeffs_z[dl];
#endif
        for (int kk = kmin; kk <= kmax; kk++)
        {
            dk = (k - kk > 0) ? k - kk - 1 : kk - k;
#if   (AMREX_SPACEDIM == 2)
            if (sk == 0) ck = stencil_coeffs_z[dk];
#elif (AMREX_SPACEDIM == 3)
            if (sk == 0) ck = stencil_coeffs_y[dk];
#endif
            for (int jj = jmin; jj <= jmax; jj++)
            {
                dj = (j - jj > 0) ? j - jj - 1 : jj - j;
                if (sj == 0) cj = stencil_coeffs_x[dj];

                res += cj * ck * cl * src_arr_zeropad(jj,kk,ll);
            }
        }
    }

#endif

    dst_arr(j,k,l) = wj * wk * wl * res;
}

#endif
