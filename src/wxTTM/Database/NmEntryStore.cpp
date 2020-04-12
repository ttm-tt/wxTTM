/* Copyright (C) 2020 Christoph Theis */

// View der Meldungen fuer ein Spiel

#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "NmEntryStore.h"
#include  "TmEntryStore.h"
#include  "MtStore.h"


// -----------------------------------------------------------------------
bool  NmEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    str = 
      "CREATE VIEW NmSingleList (   "
      "    nmType, nmNr, nmID, mtID, tmID,       "
      "    plNr, psLast, psFirst, naName, naDesc, naRegion, plExtID, ltID ) "
      "AS "
      "SELECT 1, nmSingle.nmNr, nm.nmID, nm.mtID, nm.tmID,       "
      "       lt.plNr, lt.psLast, lt.psFirst, lt.naName, lt.naDesc, lt.naRegion, lt.plExtID, lt.ltID "
      "  FROM NmSingle nmSingle INNER JOIN NmRec nm ON nmSingle.nmID = nm.nmID "
      "                    LEFT OUTER JOIN LtEntryList lt ON nmSingle.ltA = lt.ltID";

    tmp->ExecuteUpdate(str);

    str = 
      "CREATE VIEW NmDoubleList (   "
      "    nmType, nmNr, nmID, mtID, tmID,       "
      "    plAplNr, plApsLast, plApsFirst, plAnaName, plAnaDesc, plAnaRegion, plAplExtID, plAltID, "
      "    plBplNr, plBpsLast, plBpsFirst, plBnaName, plBnaDesc, plBnaRegion, plBplExtID, plBltID) "
      "AS "
      "SELECT 2, nmDouble.nmNr, nm.nmID, nm.mtID, nm.tmID, "
      "       ltA.plNr, ltA.psLast, ltA.psFirst, ltA.naName, ltA.naDesc, ltA.naRegion, ltA.plExtID, ltA.ltID, "
      "       ltB.plNr, ltB.psLast, ltB.psFirst, ltB.naName, ltB.naDesc, ltB.naRegion, ltB.plExtID, ltB.ltID  "
      "  FROM (NmDouble nmDouble INNER JOIN NmRec nm ON nmDouble.nmID = nm.nmID     "
      "                     LEFT OUTER JOIN LtEntryList ltA ON nmDouble.ltA = ltA.ltID) "
      "                     LEFT OUTER JOIN LtEntryList ltB ON nmDouble.ltB = ltB.ltID  ";

    tmp->ExecuteUpdate(str);

    str = 
      "CREATE VIEW NmEntryList (    "
      "    nmType, nmNr, nmID, mtID, tmID,                       "
      "    plAplNr, plApsLast, plApsFirst, plAnaID, plAnaName,  plAnaDesc, plAnaRegion, plAplextID, ltAltID,  "
      "    plBplNr, plBpsLast, plBpsFirst, plBnaID, plBnaName,  plBnaDesc, plBnaRegion, plBplextID, ltBltID ) "
      "AS "
      "SELECT 1, nmSingle.nmNr, nm.nmID, nm.mtID, nm.tmID,             "
      "       lt.plNr, lt.psLast, lt.psFirst, lt.naID, lt.naName, lt.naDesc, lt.naRegion, lt.plExtID, lt.ltID,  "
      "       NULL,    NULL,      NULL,       NULL,    NULL,      NULL,      NULL,        NULL,       NULL      "
      "  FROM NmSingle nmSingle INNER JOIN NmRec nm                    "
      "       ON nmSingle.nmID = nm.nmID                               "
      "       LEFT OUTER JOIN LtEntryList lt ON nmSingle.ltA = lt.ltID "
      "UNION "
      "SELECT 2, nmDouble.nmNr, nm.nmID, nm.mtID, nm.tmID,               "
      "       ltA.plNr, ltA.psLast, ltA.psFirst, ltA.naID, ltA.naName, ltA.naDesc, ltA.naRegion, ltA.plExtID, ltA.ltID,   "
      "       ltB.plNr, ltB.psLast, ltB.psFirst, ltB.naID, ltB.naName, ltB.naDesc, ltB.naRegion, ltB.plExtID, ltB.ltID    "
      "  FROM (NmDouble nmDouble INNER JOIN NmRec nm                      "
      "       ON nmDouble.nmID = nm.nmID                                  "
      "       LEFT OUTER JOIN LtEntryList ltA ON nmDouble.ltA = ltA.ltID) "
      "       LEFT OUTER JOIN LtEntryList ltB ON nmDouble.ltB = ltB.ltID  ";
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  NmEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW NmEntryList");
    tmp->ExecuteUpdate("DROP VIEW NmSingleList");
    tmp->ExecuteUpdate("DROP VIEW NmDoubleList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
NmEntryStore::NmEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
}


void  NmEntryStore::Init()
{
  NmEntry::Init();
}


// -----------------------------------------------------------------------
bool  NmEntryStore::SelectByMtTm(const MtRec &mt, const TmEntry &tm)
{
  wxString  str = SelectString();
  str += " WHERE mtID = " + ltostr(mt.mtID) +
         "   AND tmID = " + ltostr(tm.tmID) +
         " ORDER BY nmType, nmNr";

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


bool  NmEntryStore::ExistsForMt(const MtRec &mt)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString str = 
      "SELECT COUNT(*) FROM NmRec nm "  
      " WHERE mtID = " + ltostr(mt.mtID);

  short res;

  try
  {
    stmtPtr = connPtr->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &res);
    if ( !resPtr->Next() || resPtr->WasNull(1) )
      res = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    res = 0;
  }  
  
  delete resPtr;
  delete stmtPtr;

  return res > 0 ? true : false;

}

// -----------------------------------------------------------------------
wxString  NmEntryStore::SelectString() const
{
  return "SELECT nmType, nmNr, nmID, mtID, tmID, ltAltID, ltBltID, "
         "       plAplNr, plApsLast, plApsFirst, plAnaName, plAnaDesc, "
         "       plBplNr, plBpsLast, plBpsFirst, plBnaName ,plBnaDesc  "
         "  FROM NmEntryList ";
}


void  NmEntryStore::BindRec()
{
  BindCol(1, &team.cpType);
  BindCol(2, &nmNr);
  BindCol(3, &nmID);
  BindCol(4, &mtID);
  BindCol(5, &tmID);

  BindCol(6, &ltA);
  BindCol(7, &ltB);

  BindCol(8, &team.pl.plNr);
  BindCol(9, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
  BindCol(10, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
  BindCol(11, team.pl.naName, sizeof(team.pl.naName));
  BindCol(12, team.pl.naDesc, sizeof(team.pl.naDesc));

  BindCol(13,  &team.bd.plNr);
  BindCol(14, team.bd.psName.psLast, sizeof(team.bd.psName.psLast));
  BindCol(15, team.bd.psName.psFirst, sizeof(team.bd.psName.psFirst));
  BindCol(16, team.bd.naName, sizeof(team.bd.naName));
  BindCol(17, team.bd.naDesc, sizeof(team.bd.naDesc));
}