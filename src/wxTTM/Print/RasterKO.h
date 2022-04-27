/* Copyright (C) 2020 Christoph Theis */

// Aufspaltung von GRRASTER
// Raster fuer KO Systeme (impelemntiert nur fuer Einfach KO)
// 02.09.95 (ChT)

# ifndef  RASTER_KO_H
# define  RASTER_KO_H

#include  "Raster.h"


// +---------------------------------------------+
// +       Zwischenstufe: RASTER_KO              +
// +---------------------------------------------+

struct SettingKO;

class  RasterKO : public RasterBase
{
  // Konstruktor
  public:
    RasterKO(Printer *prt, Connection *connPtr) : RasterBase(prt, connPtr) {};

  // oeffentliche Methoden
  public:
    // Drucke Kopfzeile
    virtual int  PrintHeading(SettingKO &) {return 0;}
    // Drucke Ergebnis von Spiel
    int  PrintResult(MtRec &, const CRect &, const std::vector<MtSet> & = std::vector<MtSet>());
    // Drucke Austragungszeit von Spiel
    int  PrintTime(MtRec &, const CRect &, SettingKO &);
    // Drucke einzelnes Spiel von KO in Region
    int  PrintMatch(MtRec &, const CRect &, SettingKO &, const std::vector<MtSet> & = std::vector<MtSet>());
    // Drucke linke Randspalte mit Nation der Spieler
		int  PrintKOMargin(SettingKO &, long flags);
    // Drucke Band aus Rastern
    int  PrintBand(SettingKO &, int = 0);
    // Drucke (EKO)-Raster ab Runde und Match in Trostrunde
		int  PrintRaster(SettingKO &);
		
		// Caption etwas tiefer starten
		virtual void  PrintCaption();

	protected:
		// Font fuer die Spielernamen etc.
    short  nameFont;
};

// +----------------------------------------------+
// +        abgeleitete Klasse RASTER_EKO         +
// +        druckt Raster einer EKO_Gruppe        +    
// +----------------------------------------------+

class  RasterSKO : public RasterKO
{
  // Konstruktor
  public:
    RasterSKO(Printer *prt, Connection *connPtr);

  // Kopfzeile drucken
  public:
    virtual  int  PrintHeading(SettingKO &);
    
  protected:
    virtual void PrintSponsor();

  // Aufruf
  public:
    int  Print(const CpRec &, const GrRec &, const PrintRasterOptions &, int *pofstX, int *pofstY, int *ppage);  
};



// +----------------------------------------------+
// +        abgeleitete Klasse RASTER_DKO         +
// +        druckt Raster einer DKO_Gruppe        +    
// +----------------------------------------------+

class  RasterDKO : public RasterKO
{
  // Konstruktor
  public:
    RasterDKO(Printer *prt, Connection *connPtr);

  // Kopfzeile drucken
  public:
    virtual  int  PrintHeading(SettingKO &);

  // Aufruf
  public:
    int  Print(CpRec &, GrRec &, PrintRasterOptions &, int *pofstX, int *pofstY, int *ppage);
};




// +----------------------------------------------+
// +        abgeleitete Klasse RASTER_PLO         +
// +        druckt Raster einer PLO_Gruppe        +    
// +----------------------------------------------+

class  RasterPLO : public RasterKO
{
  // Konstruktor
  public:
    RasterPLO(Printer *prt, Connection *connPtr);

  // Kopfzeile drucken
  public:
    virtual  int  PrintHeading(SettingKO &);

  // Aufruf
  public:
    int  Print(CpRec &, GrRec &, PrintRasterOptions &, int *pofstX, int *pofstY, int *ppage);
};


# endif