/* Copyright 2023 Grant Johnson, Remi Lehe
 *
 *
 * This file is part of WarpX.
 *
 * License: BSD-3-Clause-LBNL
 */
#include "MultiFluidContainer.H"
#include "Fluids/WarpXFluidContainer.H"
#include "Utils/Parser/ParserUtils.H"

#include <string>

using namespace amrex;

MultiFluidContainer::MultiFluidContainer (int nlevs_max)
{
    const ParmParse pp_fluids("fluids");
    pp_fluids.queryarr("species_names", species_names);

    const int nspecies = static_cast<int>(species_names.size());

    allcontainers.resize(nspecies);
    for (int i = 0; i < nspecies; ++i) {
        allcontainers[i] = std::make_unique<WarpXFluidContainer>(nlevs_max, i, species_names[i]);
    }
}

void
MultiFluidContainer::AllocateLevelMFs (int lev, const BoxArray& ba, const DistributionMapping& dm, ablastr::fields::MultiFabRegister& m_fields)
{
    for (auto& fl : allcontainers) {
        fl->AllocateLevelMFs(lev, ba, dm, m_fields);
    }
}

void
MultiFluidContainer::InitData (int lev, amrex::Box init_box, amrex::Real cur_time, ablastr::fields::MultiFabRegister& m_fields)
{
    for (auto& fl : allcontainers) {
        fl->InitData(lev, init_box, cur_time,m_fields);
    }
}


void
MultiFluidContainer::DepositCharge (int lev, amrex::MultiFab &rho, ablastr::fields::MultiFabRegister& m_fields)
{
    for (auto& fl : allcontainers) {
        fl->DepositCharge(lev,rho,m_fields);
    }
}

void
MultiFluidContainer::DepositCurrent (int lev,
    amrex::MultiFab& jx, amrex::MultiFab& jy, amrex::MultiFab& jz, ablastr::fields::MultiFabRegister& m_fields)
{
    for (auto& fl : allcontainers) {
        fl->DepositCurrent(lev,jx,jy,jz,m_fields);
    }
}

void
MultiFluidContainer::Evolve (int lev,
                            const MultiFab& Ex, const MultiFab& Ey, const MultiFab& Ez,
                            const MultiFab& Bx, const MultiFab& By, const MultiFab& Bz,
                            MultiFab* rho, MultiFab& jx, MultiFab& jy, MultiFab& jz,
                            amrex::Real cur_time, ablastr::fields::MultiFabRegister& m_fields, bool skip_deposition)
{
    for (auto& fl : allcontainers) {
        fl->Evolve(lev, Ex, Ey, Ez, Bx, By, Bz, rho, jx, jy, jz, cur_time, m_fields, skip_deposition);
    }
}