/* Copyright (C) 2020 Christoph Theis */

// Liste der "Mannschafts"meldungen
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "NtEntryStore.h"
#include  "TmStore.h"


bool  NtEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    // Alle Spielermeldungen
    str = "CREATE VIEW NtEntryList AS                                      "
          "  SELECT plNr, psLast, psFirst, psSex, psBirthday,              "
          "         naName, naDesc, naRegion,                              "
          "         plRankPts, ltRankPts,                                  "
          "         lt.ltID, cpID, plID, naID, tmID, ntNr                  "
          "    FROM NtRec nt INNER JOIN LtEntryList lt ON nt.ltID = lt.ltID";

    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  NtEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS NtEntryList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
NtEntryStore::NtEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
}


void  NtEntryStore::Init()
{
  NtEntry::Init();
}


bool  NtEntryStore::Next()
{
  if (!StoreObj::Next())
    return false;
    
  nt.ltID = ltID;
  PlRec::plID = LtRec::plID;

  return true;
}


// -----------------------------------------------------------------------
bool  NtEntryStore::SelectByTm(const TmRec &tm)
{
  wxString  str = SelectString();
  str += " WHERE  tmID = " + ltostr(tm.tmID);

  try
  {
    ExecuteQuery(str);

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  NtEntryStore::SelectByLt(const LtRec &lt)
{
  wxString  str = SelectString();
  str += " WHERE  ltID = " + ltostr(lt.ltID);

  try
  {
    ExecuteQuery(str);

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
wxString  NtEntryStore::SelectString() const
{
  wxString  str;

   str = "SELECT plNr, psLast, psFirst, psSex, naName, naDesc, "
         "       ltID, cpID, plID, ntNr, tmID  "
         "  FROM NtEntryList ";
  return str;
}


bool  NtEntryStore::BindRec()
{
  int  col = 1;

  BindCol(col++, &plNr);
  BindCol(col++, psName.psLast, sizeof(psName.psLast));
  BindCol(col++, psName.psFirst, sizeof(psName.psFirst));
  BindCol(col++, &psSex);
  BindCol(col++, naName, sizeof(naName));
  BindCol(col++, naDesc, sizeof(naDesc));
  BindCol(col++, &ltID);
  BindCol(col++, &cpID);
  BindCol(col++, &(LtRec::plID));
  BindCol(col++, &nt.ntNr);
  BindCol(col++, &nt.tmID);

  return true;
}