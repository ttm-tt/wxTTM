/* Copyright (C) 2020 Christoph Theis */

// Aufteilung von GRRASTER
// Raster fuer Round Robin
// 02.09.95 (ChT)

# ifndef  RASTER_RR_H
# define  RASTER_RR_H

# include "Raster.h"

#include <vector>

// std::vector will ansheinend wissen, wie die Klassen aussehen
#include "MtStore.h"
#include "TbItem.h"


// +----------------------------------------------+
// +        abgeleitete Klasse RASTER_RR          +
// +        druckt Raster einer RR-Gruppe         +    
// +----------------------------------------------+

class  RasterRR : public RasterBase
{
  // Konstruktor
  public:
    RasterRR(Printer *prt, Connection *connPtr);
   ~RasterRR();

  // Aufruf
  public:
		int  Print(CpRec &, GrRec &, PrintRasterOptions &,
		           int *pofstX, int *pofstY, int *ppage);

    void  PrintCaption();           // Druckt Header
    void  PrintEvent(CRect &);      // Druckt Datum/Zeit/Tisch

  // interne Methoden
  private:
    void  PrintNames();     // Erster Teil: Namen
    void  PrintRaster();    // Zweiter Teil: Raster
    void  PrintResults();   // Driter Teil: Ergebnisse

    void  WriteNames();     // Schreiben: Namen
    void  WriteRaster();    // Schreiben: Raster
    void  WriteResults();   // Schreiben: Ergebnisse

    // int   SortEntries(GRSTORE &, int size);     // Bestimmt die Postionen

  // interne Variablen
  private:
    // TEAMDESC  *teamDesc;
    std::vector<TbItem *> tbList;
    std::vector<MtRec>  mtList;

		int  rw;        // Breite eines Elements im Raster
		int  baseLine;  // Unetere Rand der Texte
		long height;
		int  rrSchedule;  // Schedule in Raster drucken
    int  rrIgnore;    // Byes wie Spieler behandeln

    CRect  number;
    CRect  names;
    CRect  raster;
    CRect  match;
    CRect  games;
    CRect  place;
};


# endif
