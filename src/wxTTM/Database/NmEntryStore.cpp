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
      "    plID, plNr, psLast, psFirst, psSex, naName, naDesc, naRegion, plExtID, ltID ) "
      "AS "
      "SELECT 1, nmSingle.nmNr, nm.nmID, nm.mtID, nm.tmID,       "
      "       lt.plID, lt.plNr, lt.psLast, lt.psFirst, lt.psSex, lt.naName, lt.naDesc, lt.naRegion, lt.plExtID, lt.ltID "
      "  FROM NmSingle nmSingle INNER JOIN NmRec nm ON nmSingle.nmID = nm.nmID "
      "                    LEFT OUTER JOIN LtEntryList lt ON nmSingle.ltA = lt.ltID";

    tmp->ExecuteUpdate(str);

    str = 
      "CREATE VIEW NmDoubleList (   "
      "    nmType, nmNr, nmID, mtID, tmID,       "
      "    plAplID, plAplNr, plApsLast, plApsFirst, plApsSex, plAnaName, plAnaDesc, plAnaRegion, plAplExtID, plAltID, "
      "    plBplID, plBplNr, plBpsLast, plBpsFirst, plBpsSex, plBnaName, plBnaDesc, plBnaRegion, plBplExtID, plBltID) "
      "AS "
      "SELECT 2, nmDouble.nmNr, nm.nmID, nm.mtID, nm.tmID, "
      "       ltA.plID, ltA.plNr, ltA.psLast, ltA.psFirst, ltA.psSex, ltA.naName, ltA.naDesc, ltA.naRegion, ltA.plExtID, ltA.ltID, "
      "       ltB.plID, ltB.plNr, ltB.psLast, ltB.psFirst, ltB.psSex, ltB.naName, ltB.naDesc, ltB.naRegion, ltB.plExtID, ltB.ltID  "
      "  FROM (NmDouble nmDouble INNER JOIN NmRec nm ON nmDouble.nmID = nm.nmID     "
      "                     LEFT OUTER JOIN LtEntryList ltA ON nmDouble.ltA = ltA.ltID) "
      "                     LEFT OUTER JOIN LtEntryList ltB ON nmDouble.ltB = ltB.ltID  ";

    tmp->ExecuteUpdate(str);

    str = 
      "CREATE VIEW NmEntryList (    "
      "    nmType, nmNr, nmID, mtID, tmID,                       "
      "    plAplID, plAplNr, plApsLast, plApsFirst, plApsSex, plAnaID, plAnaName,  plAnaDesc, plAnaRegion, plAplextID, ltAltID,  "
      "    plBplID, plBplNr, plBpsLast, plBpsFirst, plBpsSex, plBnaID, plBnaName,  plBnaDesc, plBnaRegion, plBplextID, ltBltID ) "
      "AS "
      "SELECT 1, nmSingle.nmNr, nm.nmID, nm.mtID, nm.tmID,             "
      "       lt.plID, lt.plNr, lt.psLast, lt.psFirst, lt.psSex, lt.naID, lt.naName, lt.naDesc, lt.naRegion, lt.plExtID, lt.ltID,  "
      "       NULL,    NULL,    NULL,      NULL,       NULL,     NULL,    NULL,      NULL,      NULL,        NULL,       NULL      "
      "  FROM NmSingle nmSingle INNER JOIN NmRec nm                    "
      "       ON nmSingle.nmID = nm.nmID                               "
      "       LEFT OUTER JOIN LtEntryList lt ON nmSingle.ltA = lt.ltID "
      "UNION "
      "SELECT 2, nmDouble.nmNr, nm.nmID, nm.mtID, nm.tmID,               "
      "       lta.plID, ltA.plNr, ltA.psLast, ltA.psFirst, ltA.psSex, ltA.naID, ltA.naName, ltA.naDesc, ltA.naRegion, ltA.plExtID, ltA.ltID,   "
      "       ltB.plID, ltB.plNr, ltB.psLast, ltB.psFirst, ltB.psSex, ltB.naID, ltB.naName, ltB.naDesc, ltB.naRegion, ltB.plExtID, ltB.ltID    "
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
         "       plAplNr, plApsLast, plApsFirst, plApsSex, plAnaName, plAnaDesc, "
         "       plBplNr, plBpsLast, plBpsFirst, plBpsSex, plBnaName ,plBnaDesc  "
         "  FROM NmEntryList ";
}


void  NmEntryStore::BindRec()
{
  int idx = 0;
  BindCol(++idx, &team.cpType);
  BindCol(++idx, &nmNr);
  BindCol(++idx, &nmID);
  BindCol(++idx, &mtID);
  BindCol(++idx, &tmID);

  BindCol(++idx, &ltA);
  BindCol(++idx, &ltB);

  BindCol(++idx, &team.pl.plNr);
  BindCol(++idx, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
  BindCol(++idx, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
  BindCol(++idx, &team.pl.psSex);

  BindCol(++idx, team.pl.naName, sizeof(team.pl.naName));
  BindCol(++idx, team.pl.naDesc, sizeof(team.pl.naDesc));

  BindCol(++idx,  &team.bd.plNr);
  BindCol(++idx, team.bd.psName.psLast, sizeof(team.bd.psName.psLast));
  BindCol(++idx, team.bd.psName.psFirst, sizeof(team.bd.psName.psFirst));
  BindCol(++idx, team.bd.naName, sizeof(team.bd.naName));
  BindCol(++idx, team.bd.naDesc, sizeof(team.bd.naDesc));
}