/* Copyright (C) 2020 Christoph Theis */

// Tabelle der internen IDs
#ifndef  IDSTORE_H
#define  IDSTORE_H

#include "StoreObj.h"

class  Image;
class  Connection;



// Basisklasse: C ??
struct IdRec
{
  SQLINTEGER  idLast;
  SQLINTEGER  idVersion;
};


class  IdStore : public StoreObj, public IdRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateTable(long dbVersion);
    static  bool  UpdateTable(long version, long dbVersion);

  // Konstruktor
  protected:
    IdStore();

    virtual void Init();

  // Neue ID anlegen
  public:
    static long  ID(Connection *connPtr);
    static long  IdVersion();
    
    static void  SetType(short type);
    static short GetType();
    
    static void  SetTable(short table);
    static short GetTable();
    
    static void SetReportTitle(const wxString &title);
    static wxString GetReportTitle();

    static void SetReportSubtitle(const wxString &subtitle);    
    static wxString GetReportSubtitle();    
    
    static void SetBannerImage(const wxString &);
    static bool GetBannerImage(wxImage &);

    static void SetLogoImage(const wxString &);
    static bool GetLogoImage(wxImage &);
    
    static void SetSponsorImage(const wxString &);
    static bool GetSponsorImage(wxImage &);

    static bool GetPrintScoreCoaches();
    static void SetPrintScoreCoaches(bool);

    static bool GetPrintScoreUmpires();
    static void SetPrintScoreUmpires(bool);

    static bool GetPrintScoreUmpireName();
    static void SetPrintScoreUmpireName(bool);

    static bool GetPrintPlayersSignature();
    static void SetPrintPlayersSignature(bool);

    static bool GetPrintScoreExtras();
    static void SetPrintScoreExtras(bool);

    static bool GetPrintScoreStartEnd();
    static void SetPrintScoreStartEnd(bool);

    static bool GetPrintScoreRemarks();
    static void SetPrintScoreRemarks(bool);

    static bool GetPrintScoreServiceTimeout();
    static void SetPrintScoreServiceTimeout(bool);

    static bool GetPrintScoreCards();
    static void SetPrintScoreCards(bool);

    static bool GetPrintScoreSides();
    static void SetPrintScoreSides(bool);

private:
    static bool GetFlag(const wxString &name);
    static void SetFlag(const wxString &name, bool f);
};

#endif