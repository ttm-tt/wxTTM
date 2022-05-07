/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Spielmodi
#ifndef  MDSTORE_H
#define  MDSTORE_H

#include  "StoreObj.h"

#include  <iostream>
#include  <string>


class  Statement;
class  ResultSet;

// Tabellendaten in eigene Struktur
struct MdRec
{
  long      mdID;        // Unique ID
  wxChar    mdName[9];   // Short name ("MD4")
  wxChar    mdDesc[65];  // Full description ("Modus for 4")
  short     mdSize;      // Number of entries
  short     mdMtPtsWin;  // Match points for a win
  short     mdMtPtsTie;  // Match points for a tie
  short     mdMtPtsLoss; // Match points for a loss
  long      mpID;        // Match points calculation

  struct    MdList
  {
    long    mdPlayerA;   // Verweis auf Setzung A
    long    mdPlayerX;   // Verweis auf Setzunx X
  } *mdList = nullptr;

  MdRec() {memset(this, 0, sizeof(MdRec));}
 ~MdRec() {delete[] mdList;}

  MdRec & operator=(const MdRec &);

  void    Init() {delete[] mdList; memset(this, 0, sizeof(MdRec));}

  short   Rounds() const  {return mdSize == 0 || mdSize & 0x1 ? mdSize : mdSize - 1;}
  short   Matches() const {return mdSize / 2;}
  long    GetPlayerA(short rd, short mt) const;
  long    GetPlayerX(short rd, short mt) const;

  // Aender der Liste durch MdEdit
  void  ChangeSize(short newSize);
  void  SetPlayerA(short rd, short mt, long plA);
  void  SetPlayerX(short rd, short mt, long plX);
};


inline MdRec& MdRec::operator=(const MdRec& md)
{
  mdID = md.mdID;
  wxStrcpy(mdName, md.mdName);
  wxStrcpy(mdDesc, md.mdDesc);
  mdSize = md.mdSize;
  mdMtPtsWin = md.mdMtPtsWin;
  mdMtPtsTie = md.mdMtPtsTie;
  mdMtPtsLoss = md.mdMtPtsLoss;
  mpID = md.mpID;

  mdList = nullptr;

  if (md.mdList)
  {
    mdList = new MdList[md.mdSize];
    for (int idx = 0; idx < md.mdSize; ++idx)
      mdList[idx] = md.mdList[idx];
  }

  return *this;
}


// Sicht auf Tabelle allein
class  MdStore : public StoreObj, public MdRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);
    
    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    MdStore(Connection * = 0);  // Defaultkonstruktor
   ~MdStore();

    virtual void Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name);
    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    bool  Next();

    // Check auf mdName, ob Modus existiert
    bool  InsertOrUpdate();
};


inline  long  MdRec::GetPlayerA(short rd, short mt) const
{
  if (!mdList)
    return 0;

  return mdList[Matches() * (rd-1) + (mt-1)].mdPlayerA;
}


inline  long  MdRec::GetPlayerX(short rd, short mt) const
{
  if (!mdList)
    return 0;

  return mdList[Matches() * (rd-1) + (mt-1)].mdPlayerX;
}


inline  void  MdRec::ChangeSize(short newSize)
{
  delete[] mdList;

  mdSize = newSize;
  mdList = new MdList[Rounds() * Matches()];
  
  memset(mdList, 0, Rounds() * Matches() * sizeof(MdList));
}


inline  void  MdRec::SetPlayerA(short rd, short mt, long plA)
{
  wxASSERT(mdList);
  if (!mdList)
    return;

  mdList[Matches() * (rd-1) + (mt-1)].mdPlayerA = plA;
}


inline  void  MdRec::SetPlayerX(short rd, short mt, long plX)
{
  wxASSERT(mdList);
  if (!mdList)
    return;

  mdList[Matches() * (rd-1) + (mt-1)].mdPlayerX = plX;
}

#endif
