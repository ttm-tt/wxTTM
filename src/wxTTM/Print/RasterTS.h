/* Copyright (C) 2023 Christoph Theis */
#pragma once

#include "Raster.h"

#include "SyListStore.h"

class RasterToss : public RasterBase
{
  public:
    RasterToss(Printer *, Connection *);
   ~RasterToss();

  public:
		int Print(const CpRec &, const GrRec &, const MtEntry &);

  protected:
    int PrintToss(const MtEntry &, const CRect &, bool ax);
    int PrintHeader(const MtEntry &, const CRect &);
    int PrintTeam(const TmEntry &, const CRect &);
    int PrintSystem(const SyListRec &, const CRect &);
    int PrintNomination(const SyListRec &, const CRect &, bool ax);

  protected:
    short headerFont = 0;
    short smallFont = 0;
};
