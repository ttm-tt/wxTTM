/* Copyright (C) 2020 Christoph Theis */

// MAl was Neues: Klasse fuer ScoreSheet

# ifndef  RASTERSC_H
# define  RASTERSC_H

#include  <Raster.h>

struct  MtEntry;
struct  CpRec;
struct  GrRec;

class  RasterScore : public RasterBase
{
	public:
		RasterScore(Printer *, Connection *);
	 ~RasterScore();

	public:
		// ScoreSheet
		int  PrintScoreHeader(const CpRec &, const GrRec &, const MtEntry &);
		int  PrintScore(const MtEntry &);
		int  PrintScoreTM(const MtEntry &);
    int  PrintScoreExtras(const MtEntry &);
    int  PrintScoreRemarks(const MtEntry &);
    int  PrintScoreFooter(const MtEntry &);

		int  Print(const CpRec &, const GrRec &, const MtEntry &);
};

# endif
