/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "GrListStore.h"
#include  "GrStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "CpStore.h"
#include  "TmStore.h"
#include  "MdStore.h"
#include  "MpStore.h"
#include  "SyStore.h"

#include  <stdio.h>
#include  <stdlib.h>

#include  <list>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  GrListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW GrList AS "
                     "SELECT grID, grName, grDesc, grStage, grModus, grSize, grWinner, "
                     "grBestOf, grQualRounds, grNofRounds, grNofMatches, "
                     "grNoThirdPlace, grOnlyThirdPlace, "
                     "grPublished, grNotes, grSortOrder, grPrinted, "
                     "CASE WHEN grNotes IS NULL THEN 0 ELSE 1 END AS grHasNotes, "
                     "cp.cpID, cpName, md.mdID, md.mdName, sy.syID, sy.syName "
                     "FROM GrRec gr INNER JOIN CpRec cp ON gr.cpID = cp.cpID "
                     "     LEFT OUTER JOIN SyRec sy ON gr.syID = sy.syID "
                     "     LEFT OUTER JOIN MdRec md ON gr.mdID = md.mdID ";

  try
  {
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  GrListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS GrList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
GrListStore::GrListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


GrListStore::~GrListStore()
{
}


void  GrListStore::Init()
{
  GrListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  GrListStore::SelectAll(const CpRec &cp)
{
  wxString  str = SelectString();
  str += " WHERE gr.cpID = " + ltostr(cp.cpID) + " ORDER BY grSortOrder, grStage, grName";

  try
  {
    if (!ExecuteQuery(str))
      return false;
       
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  GrListStore::SelectAll(const MdRec& md)
{
  wxString  str = SelectString();
  str += " WHERE grModus =  " + ltostr(MOD_RR) + " AND gr.mdID = " + ltostr(md.mdID) + " ORDER BY grSortOrder, grStage, grName";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException& e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  GrListStore::SelectAll(const MpRec& mp)
{
  wxString  str = SelectString();
  str += " INNER JOIN MdList md ON md.mdID = gr.mdID ";
  str += " WHERE md.mpID = " + ltostr(mp.mpID) + " ORDER BY grSortOrder, grStage, grName";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException& e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  GrListStore::SelectAll(const SyRec& sy)
{
  wxString  str = SelectString();
  str += " WHERE gr.syID = " + ltostr(sy.syID) + " ORDER BY grSortOrder, grStage, grName";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException& e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  GrListStore::SelectById(long id)
{
  wxString  str = SelectString();
  str += "WHERE grID = " + ltostr(id);

  try
  {
    if (!ExecuteQuery(str))
      return false;
 
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  GrListStore::SelectById(const std::set<long> &idList)
{
  wxString  str = SelectString();
  str += "WHERE grID IN (" + ltostr(idList) + ")";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool GrListStore::SelectByCpTm(const CpRec &cp, const TmRec &tm)
{
  wxString str = SelectString();
  str += " WHERE cp.cpID = " + ltostr(cp.cpID) + 
         "   AND gr.grID IN (SELECT grID FROM StRec "
         "                 WHERE tmID = " + ltostr(tm.tmID) + ")";

  try
  {
    if (!ExecuteQuery(str))
      return false;
 
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool GrListStore::SelectByTm(const TmRec &tm)
{
  wxString str = SelectString();
  str += " WHERE grID IN (SELECT grID FROM StRec "
         "                 WHERE tmID = " + ltostr(tm.tmID) + ")";

  try
  {
    if (!ExecuteQuery(str))
      return false;
 
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  GrListStore::SelectByStage(const CpRec &cp, const wxString &stage)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + 
         "   AND grStage = '" + TransformString(stage) + "'" +
         " ORDER BY grName";

  try
  {
    if (!ExecuteQuery(str))
      return false;
       
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
std::list<wxString> GrListStore::ListStages(const CpRec &cp)
{
  wxString str = "SELECT DISTINCT grStage FROM GrList";
  
  if (cp.cpID != 0)
    str += " WHERE cpID = " + ltostr(cp.cpID);
    
  str += " ORDER BY 1";

  std::list<wxString> res;

  wxChar stage[65];

  try
  {
    if (!ExecuteQuery(str))
      return res;

    BindCol(1, stage, sizeof(stage));

    while (Next())
    {
      if (WasNull(1))
        continue;

      res.push_back(wxString(stage));
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return res;
}

// -----------------------------------------------------------------------
bool  GrListStore::QryStarted() 
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;
  
  return gr.QryStarted();
}


bool  GrListStore::QryFinished()
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;
  
  return gr.QryFinished();
}


bool  GrListStore::QryChecked()
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;
  
  return gr.QryChecked();
}


bool  GrListStore::QryCombined()
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;

  return gr.QryCombined();
}

// -----------------------------------------------------------------------
long GrListStore::Count()
{
  Statement *stmtPtr = NULL;
  ResultSet *resPtr = NULL;

  wxString str = "SELECT COUNT(*) FROM GrList";

  long count = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();

    ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
    if (!resPtr || !resPtr->Next())
      count = 0;
    else if (!resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    count = 0;
  }

  delete resPtr;
  delete stmtPtr;

  return count;
}


wxString GrListStore::GetNote()
{
  return GrStore(*this, GetConnectionPtr()).GetNote();
}


short GrListStore::GetLastScheduledRound(long id)
{
  Statement *stmtPtr = NULL;
  ResultSet *resPtr = NULL;

  wxString str = 
      "SELECT MAX(mtRound) FROM MtList mt "
      "  WHERE grID = " + ltostr(id ? id : grID) + " AND DAY(mtDateTime) <> 0 ";

  short count = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();

    ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
    if (!resPtr || !resPtr->Next())
      count = 0;
    else if (!resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    count = 0;
  }

  delete resPtr;
  delete stmtPtr;

  return count;
}

// -----------------------------------------------------------------------
wxString  GrListStore::SelectString() const
{
  // For baackward compatibility ínclude a join with CpList, MdList, SyList
  // Remove them later
  wxString  str = 
    "SELECT grID, grName, grDesc, grStage, grModus, grSize, grWinner, "
    "       cp.cpID, cp.cpName, md.mdID, md.mdName, sy.syID, sy.syName, grBestOf, "
    "       grQualRounds, grNofRounds, grNofMatches, grNoThirdPlace, grOnlyThirdPlace, "
    "       grPublished, grHasNotes, grSortOrder, grPrinted "
    "  FROM GrList gr "
    "       INNER JOIN CpList cp ON gr.cpID = cp.cpID "
    "       LEFT OUTER JOIN MdList md ON gr.mdID = md.mdID "
    "       LEFT OUTER JOIN SyList sy ON gr.syID = sy.syID "
    ;

  return str;
}


bool  GrListStore::BindRec()
{
  int idx = 0;

  BindCol(++idx, &grID);
  BindCol(++idx, grName, sizeof(grName));
  BindCol(++idx, grDesc, sizeof(grDesc));
  BindCol(++idx, grStage, sizeof(grStage));
  BindCol(++idx, &grModus);
  BindCol(++idx, &grSize);
  BindCol(++idx, &grWinner);
  BindCol(++idx, &cpID);
  BindCol(++idx, cpName, sizeof(cpName));
  BindCol(++idx, &mdID);
  BindCol(++idx, mdName, sizeof(mdName));
  BindCol(++idx, &syID);
  BindCol(++idx, syName, sizeof(syName));
  BindCol(++idx, &grBestOf);
  BindCol(++idx, &grQualRounds);
  BindCol(++idx, &grNofRounds);
  BindCol(++idx, &grNofMatches);
  BindCol(++idx, &grNoThirdPlace);
  BindCol(++idx, &grOnlyThirdPlace);
  BindCol(++idx, &grPublished);
  BindCol(++idx, &grHasNotes);
  BindCol(++idx, &grSortOrder);
  BindCol(++idx, &grPrinted);

  return true;
}