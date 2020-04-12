/* Copyright (C) 2020 Christoph Theis */

// Relation PlRec, CpRec, TmRec

#include  "stdafx.h"
#include  "LtStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"
#include  "CpStore.h"
#include  "PlStore.h"
#include  "TmStore.h"
#include  "NaStore.h"
#include  "NtStore.h"

#include  "LtEntryStore.h"

#include  "Rec.h"
#include  "StrUtils.h"
#include  "wxStringTokenizerEx.h"


#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>

// -----------------------------------------------------------------------
bool  LtStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString  sql = 
    "CREATE TABLE LtRec (         "
    "ltID        "+INTEGER+"      NOT NULL,  "
    "plID        "+INTEGER+"      NOT NULL,  "
    "cpID        "+INTEGER+"      NOT NULL,  "
    "ltTimestamp "+TIMESTAMP+"    NOT NULL DEFAULT GETUTCDATE(), "
    "CONSTRAINT ltIdKey PRIMARY KEY (ltID) ,  "
    "ltCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "ltModifiedBy AS (SUSER_SNAME()), "
    "ltStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "ltEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (ltStartTime, ltEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.LtHist))";

  try
  {
    tmp->ExecuteUpdate(sql);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
    delete tmp;

    return false;
  }

  try
  {
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX ltCpPlKey ON LtRec (cpID, plID)");
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX ltPlCpKey ON LtRec (plID, cpID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  LtStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  else if (version < 87)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
    wxString str;
    
    try 
    {      
      str = "ALTER TABLE LtRec ADD ltTimestamp "+TIMESTAMP+" NOT NULL DEFAULT GETUTCDATE()";
      stmtPtr->ExecuteUpdate(str);        
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      delete stmtPtr;
      return false;
    }
    
    delete stmtPtr;
  }

  if (version < 149)
  {
    // History
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    wxString str;

    try
    {
      str = "ALTER TABLE LtRec ADD "
        "ltCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "ltModifiedBy AS (SUSER_SNAME()), "
        "ltStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "ltEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (ltStartTime, ltEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE LtRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.LtHist))";
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      delete stmtPtr;
      return false;
    }

    delete stmtPtr;
  }

  return true;
}


bool  LtStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE LtRec DROP CONSTRAINT ltCpRef");
    tmp->ExecuteUpdate("ALTER TABLE LtRec DROP CONSTRAINT ltPlRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE LtRec ADD CONSTRAINT ltCpRef "
      "FOREIGN KEY (cpID) REFERENCES CpRec (cpID) ON DELETE NO ACTION");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE LtRec ADD CONSTRAINT ltPlRef "
      "FOREIGN KEY (plID) REFERENCES PlRec (plID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  LtStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();
  return true;
}


// -----------------------------------------------------------------------
LtStore::LtStore(Connection *ptr)
       : StoreObj(ptr)
{
}


LtStore::~LtStore()
{
}


// -----------------------------------------------------------------------
void  LtStore::Init()
{
  LtRec::Init();
}


// -----------------------------------------------------------------------
bool  LtStore::SelectById(long id)
{
  wxString str = SelectString();
  str += "WHERE ltID = ";
  str += ltostr(id);

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


bool  LtStore::SelectByPl(long id)
{
  wxString str = SelectString();
  str += "WHERE lt.plID = ";
  str += ltostr(id);

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


bool  LtStore::SelectByCp(long id)
{
  wxString str = SelectString();
  str += "WHERE lt.cpID = ";
  str += ltostr(id);

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


bool  LtStore::SelectByTm(long id)
{
  wxString str = SelectString();
  str += "WHERE ltID IN ("
         "SELECT ltID FROM NtRec WHERE tmID = " + ltostr(id) + ")";

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


bool  LtStore::SelectByCpPl(long idCp, long idPl)
{
  wxString str = SelectString();
  str += "WHERE lt.cpID = ";
  str += ltostr(idCp);
  str += " AND lt.plID = ";
  str += ltostr(idPl);

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
bool  LtStore::SelectBuddy(const LtRec &lt)
{
  wxString str = SelectString();
  str += " WHERE ltID IN ( "
         "    SELECT ntbd.ltID "
         "      FROM NtRec ntpl INNER JOIN NtRec ntbd ON ntpl.tmID = ntbd.tmID "
         "     WHERE ntpl.ltID = " + ltostr(lt.ltID) + " AND ntpl.ntNr <> ntbd.ntNr "
         " ) ";

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
bool  LtStore::Insert(const CpRec &cp, const PlRec &pl)
{
  cpID = cp.cpID;
  plID = pl.plID;

  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO LtRec (ltID, cpID, plID) "
                    "            VALUES(?,    ?,    ?   ) ";

  try
  {
    ltID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &ltID);
    stmtPtr->SetData(2, &cpID);
    stmtPtr->SetData(3, &plID);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::INSERT;
  update.rec  = CRequest::LTREC;
  update.id   = ltID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  LtStore::Update()
{
  return true;
}


bool  LtStore::InsertOrUpdate(const CpRec &cp, const PlRec &pl)
{
  long  id;
  Statement *stmtPtr;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT ltID FROM LtRec WHERE plID = ";
  sql += ltostr(pl.plID);
  sql += " AND cpID = ";
  sql += ltostr(cp.cpID);

  ResultSet *resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    ltID = id;
    return Update();
  }
  else
    return Insert(cp, pl);  
}


bool  LtStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = ltID;

  if (!id)
    return true;
  
  wxString  str;
  
  try
  {
    str = "DELETE FROM NtRec WHERE ltID = ";
    str += ltostr(id);
    ExecuteUpdate(str);

    str = "DELETE FROM LtRec WHERE ltID = ";
    str += ltostr(id);
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::REMOVE;
  update.rec  = CRequest::LTREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
wxString  LtStore::SelectString() const
{
  wxString  str = 
    "SELECT ltID, lt.cpID, lt.plID, ISNULL((SELECT rpRankPts FROM RpRec rp WHERE rp.plID = lt.plID AND rpYear = cp.cpYear), pl.plRankPts) AS ltRankPts "
    "  FROM LtRec lt INNER JOIN PlRec pl ON lt.plID = pl.plID INNER JOIN CpRec cp ON lt.cpID = cp.cpID ";

  return str;
}


bool  LtStore::BindRec()
{
  BindCol(1, &ltID);
  BindCol(2, &cpID);
  BindCol(3, &plID);
  BindCol(4, &ltRankPts);

  return true;
}


// -----------------------------------------------------------------------
// Import / Export
// Siehe PlStore zur Verwendung von std::ifstream
bool LtStore::Import(const wxString &name)
{
  bool warnChangedDoubles = true;

  wxTextFile ifs(name);
  if (!ifs.Open())
    return false;

  std::list<wxString> unknownPlayerList;

  wxString line = ifs.GetFirstLine();

  // Skip leading whitespacae
  if (line.GetChar(0) == '#')
  {
    if (wxStrcmp(line, "#ENTRIES"))
    {
      if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#ENTRIES"), line.wx_str()))
        return false;
    }
  }

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  // HACK: Die Variablen duerfen nicht laenger leben als connPtr [
  {
  CpStore  cp(connPtr);
  NaStore  na(connPtr);

  for (; !ifs.Eof(); line = ifs.GetNextLine())
  {
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    wxStringTokenizerEx tokens(line, ",;\t");

    wxString strPl = tokens.GetNextToken().Strip(wxString::both);
    wxString strCp = tokens.GetNextToken().Strip(wxString::both);

    // char *strCp = strtok(line, ",;\t");
    // char *strPl = strtok(NULL, ",;\t");

    if (strCp.IsEmpty() || strPl.IsEmpty())
      continue;
      
    if (*strCp.t_str() == '#')
      continue;
      
    bool commit = false;

    if (wxStrcoll(cp.cpName, strCp))
    {
      cp.SelectByName(strCp);
      cp.Next();
      cp.Close();
            
      if (!cp.cpID)
      {
        infoSystem.Error(_("Event %s does not exist"), strCp.t_str());
        
        // Damit beim naechsten mal nicht nocheinmal gesucht wird 
        // und vor allem damit es keine weitere Fehlermeldung gibt.
        wxStrncpy(cp.cpName, strCp, sizeof(cp.cpName) / sizeof(wxChar));
        cp.cpName[sizeof(cp.cpName) - 1] = 0;
      }
    }
    
    if (!cp.cpID)
      continue;

    connPtr->StartTransaction();

    switch (cp.cpType)
    {
      case CP_SINGLE :
      {
        wxString strBd = tokens.GetNextToken().Strip(wxString::both);
        wxString strNa = tokens.GetNextToken().Strip(wxString::both);
        wxString strRk = tokens.GetNextToken().Strip(wxString::both);
        wxString strIt = tokens.GetNextToken().Strip(wxString::both);

        // Spieler melden
        PlStore  pl(connPtr);
        LtStore  lt(connPtr);
        NaRec    na;

        pl.SelectByNr(_strtol(strPl));
        if (!pl.Next())
        {
          pl.Close();
          pl.SelectByExtId(strPl);
          if (!pl.Next()) {
            pl.Close();
            pl.SelectByName(strPl);
            if (!pl.Next() || pl.Next())
            {
              unknownPlayerList.push_back(strCp + ": " + strPl);
              break;
            }

            pl.SelectByName(strPl);
            pl.Next();
          }
        }

        if (!cp.EnlistPlayer(pl, lt))
        {
          unknownPlayerList.push_back(strCp + ": " + strPl);
          break;
        }
          
#if 0          
        NtStore nt(connPtr);
        nt.SelectByLt(lt);
        if (nt.Next())
        {
          nt.Close();
          break;
        }
        
        nt.Close();
#endif        

        na.naID = pl.naID;
        commit = cp.CreateSingle(lt, na, _strtos(strRk), _strtos(strIt));

        break;
      }

      case CP_DOUBLE :
      case CP_MIXED  :
      {
        wxString strBd = tokens.GetNextToken().Strip(wxString::both);
        wxString strNa = tokens.GetNextToken().Strip(wxString::both);
        wxString strRk = tokens.GetNextToken().Strip(wxString::both);
        wxString strIt = tokens.GetNextToken().Strip(wxString::both);

        // Spieler und Partner melden
        PlStore  pl(connPtr);
        PlStore  bd(connPtr);
        LtStore  ltpl(connPtr);
        LtStore  ltbd(connPtr);

        pl.SelectByNr(_strtol(strPl));
        if (!pl.Next())
        {
          pl.Close();
          pl.SelectByExtId(strPl);
          if (!pl.Next()) {
            pl.Close();
            pl.SelectByName(strPl);
            if (!pl.Next() || pl.Next())
            {
              unknownPlayerList.push_back(strCp + ": " + strPl);
              break;
            }

            pl.SelectByName(strPl);
            pl.Next();
          }
        }

        if (!cp.EnlistPlayer(pl, ltpl))
        {
          unknownPlayerList.push_back(strCp + ": " + strPl);
          break;
        }
          
        NtStore ntpl(connPtr);
        ntpl.SelectByLt(ltpl);
        if (ntpl.Next())
        {
          ntpl.Close();
          // break;
        }

        if (!strBd.IsEmpty() && strBd != "0")
        {
          bd.SelectByNr(_strtol(strBd));
          if (!bd.Next())
          {
            bd.Close();
            bd.SelectByExtId(strBd);
            if (!bd.Next())
            {
              bd.Close();
              bd.SelectByName(strBd);
              if (!bd.Next() || bd.Next())
              {
                unknownPlayerList.push_back(strCp + ": " + strBd);

                // Aber commit des Spielers
                commit = true;
                break;
              }

              bd.SelectByName(strBd);
              bd.Next();
            }
          }
        }

        // Gleicher Spieler
        if (bd.plID == pl.plID)
          break;
        
        if (bd.plID)
        {
          if (!cp.EnlistPlayer(bd, ltbd))
          {
            unknownPlayerList.push_back(strCp + ": " + strBd);
            break;
          }
          
          NtStore ntbd(connPtr);
          ntbd.SelectByLt(ltbd);
          ntbd.Next();
          ntbd.Close();

          if (warnChangedDoubles && ntpl.tmID != ntbd.tmID)
          {
            if (!infoSystem.Question(wxT("Players %s, %s and %s, %s are not yet playing together. Do you want to continue?"), pl.psName.psLast, pl.psName.psFirst, bd.psName.psLast, bd.psName.psFirst))
            {
              commit = false;
              break;
            }
            else
            {
              warnChangedDoubles = false;
            }
          }
        }

        // Doppel wird nur einmal erzeugt
        // (auskommentiert. Sonst gibt es Probleme, wenn die
        // Bedingung in den Importfiles nicht eingehalten wird)
        // if (bd.plID && bd.plNr < pl.plNr)
        //   break;

        if (strNa.IsEmpty())
        {
          na.naID = 0;
          *na.naName = '\0';
          *na.naDesc = '\0';
        }
        else if (wxStrcoll(na.naName, strNa))
        {
          na.SelectByName(strNa);
          na.Next();
        }

        if (!bd.plID)
        {
          commit = true;
        }
        else if (pl.naID == bd.naID ? cp.CheckDoubleOrder(LtEntry(ltpl, pl), LtEntry(ltbd, bd), NaRec()) : cp.CheckDoubleOrder(LtEntry(ltpl, pl), LtEntry(ltbd, bd), na))
        {
          if (!na.naID)
            na.naID = pl.naID;
			    commit = cp.CreateDouble(ltpl, ltbd, na, _strtos(strRk), _strtos(strIt));
        }
        else
        {
          if (!na.naID)
            na.naID = bd.naID;
			    commit = cp.CreateDouble(ltbd, ltpl, na,  _strtos(strRk), _strtos(strIt));
        }

        break;
      }

      case CP_TEAM :
      {
        if (_strtol(strPl) || strPl.Find(' ') != wxNOT_FOUND)
        {
          // Spieler zu einer Mannschaft
          wxString strTm = tokens.GetNextToken().Strip(wxString::both);
          wxString strRk = tokens.GetNextToken().Strip(wxString::both);

          PlStore  pl(connPtr);
          LtStore  lt(connPtr);
          TmStore  tm(connPtr);

          if (_strtol(strPl))
          {
            pl.SelectByNr(_strtol(strPl));
            if (!pl.Next())
              break;
          }
          else
          {
            // Auswahl per Namen, aber nur, wenn eindeutig
            pl.SelectByName(strPl);
            if (!pl.Next() || pl.Next())
            {
              unknownPlayerList.push_back(strCp + ": " + strPl);
              break;
            }

            pl.SelectByName(strPl);
            pl.Next();
          }

          if (!cp.EnlistPlayer(pl, lt))
          {
            unknownPlayerList.push_back(strCp + ": " + strPl);
            break;
          }

          if (strTm.IsEmpty())
          {
            commit = true;
            break;
          }

          tm.SelectByCpTmName(cp, strTm);
          if (!tm.Next())
          {
            NaStore na(connPtr);
            na.SelectById(pl.naID);
            if (!na.Next())
              break;

            // Gibt es eine Nation mit gleichem Namen?
            if (strTm != na.naName)
              break;

            wxStrcpy(tm.tmName, na.naName);
            wxStrcpy(tm.tmDesc, na.naDesc);
            
            if (!cp.CreateTeam(tm, na))
              break;

            tm.SelectByCpTmName(cp, strTm);
            if (!tm.Next())
              break;
          }
            
          tm.Close();
          
          // Gibt es den Spieler schon in der Mannschaft?
          NtStore nt(connPtr);
          nt.SelectByLt(lt);
          
          if (!nt.Next())
            commit = tm.AddEntry(lt,  _strtos(strRk));
            
          nt.Close();
        }
        else
        {
          // Neue Mannschaft
          wxString strName = strPl;
          wxString strDesc = tokens.GetNextToken().Strip(wxString::both);
          wxString strNa   = tokens.GetNextToken().Strip(wxString::both);
          wxString strRk   = tokens.GetNextToken().Strip(wxString::both);
          wxString strIt   = tokens.GetNextToken().Strip(wxString::both);

          NaStore  na(connPtr);
          if (!strNa.IsEmpty())
          {
            na.SelectByName(strNa);
            na.Next();
          }

          TmStore  tm(connPtr);
          tm.SelectByCpTmName(cp, strName);
          if (tm.Next())
          {
            // Mannschaft existiert
            commit = true;
          }
          else
          { 
            // Neue Mannschaft
            wxStrcpy(tm.tmName, strName);
            wxStrcpy(tm.tmDesc, strDesc);
            commit = cp.CreateTeam(tm, na, _strtos(strRk), _strtos(strIt));
          }
        }

        break;
      }

      default :
        break;
    }

    if (commit)
      connPtr->Commit();
    else
      connPtr->Rollback();
  }
  }  // end HACK ]

  try
  {
    delete connPtr;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("<none>", e);
  }

  ifs.Close();

  if (unknownPlayerList.size())
  {
    std::list<wxString>::const_iterator it = unknownPlayerList.begin();
    wxString msg = (*it);

    while (++it != unknownPlayerList.end())
    {
      msg += "\n";
      msg += *it;
    }

    infoSystem.Information(_("The following player names were not found, are not unique, or are not allowed to play in that event:\n%s"), msg.wx_str());
  }
    
  return true;
}


bool  LtStore::Export(const wxString &name)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  
  std::ofstream  os(name.t_str(), std::ios::out);

  const wxString bom(wxChar(0xFEFF));
  os << bom.ToUTF8();

  os << "#ENTRIES" << std::endl;

  wxChar   cpName[9];
  short    cpType;
  short    plNr, ntNr, bdNr;
  wxChar   tmName[9];
  wxChar   tmDesc[65];
  wxChar   naName[9];
  short    rkNatlRank;
  short    rkIntlRank;
  
  wxString str;
  
  // Zuerst die Mannschaften
  str = "SELECT cp.cpName, tm.tmName, tm.tmDesc, na.naName, rk.rkNatlRank, rk.rkIntlRank "
        "  FROM TmRec tm INNER JOIN CpRec cp ON cp.cpID = tm.cpID "
        "                LEFT OUTER JOIN RkRec rk ON rk.tmID = tm.tmID "
        "                LEFT OUTER JOIN NaRec na ON na.naID = rk.naID "
        "  WHERE (cp.cpType = 4) "
        "  ORDER BY cp.cpName, tm.tmName";
        
  Statement *stmtTmPtr = connPtr->CreateStatement();
  ResultSet *resTmPtr  = 0;
  
  try
  {
    if ( stmtTmPtr->Execute(str) )
      resTmPtr = stmtTmPtr->GetResultSet(false);
  }
  catch (SQLException e)
  {
    infoSystem.Exception(str, e);
    return false;
  }
  
  resTmPtr->BindCol(1, cpName, sizeof(cpName));
  resTmPtr->BindCol(2, tmName, sizeof(tmName));
  resTmPtr->BindCol(3, tmDesc, sizeof(tmDesc));
  resTmPtr->BindCol(4, naName, sizeof(naName));
  resTmPtr->BindCol(5, &rkNatlRank);
  resTmPtr->BindCol(6, &rkIntlRank);
  
  tmDesc[0] = naName[0] = 0;
  rkNatlRank = rkIntlRank = 0;
  
  os << "# Team Name; Event; Team Description; "
     << "Association; Natl. Ranking; Int'l. Ranking" << std::endl;
  
  while (resTmPtr->Next())
  {
    wxString ofs;

    ofs << tmName << ";" << cpName << ";" << tmDesc << ";" 
        << naName << ";" << rkNatlRank << ";" << rkIntlRank;

    os << ofs.ToUTF8() << std::endl;
    
    tmDesc[0] = naName[0] = 0;
    rkNatlRank = rkIntlRank = 0;
  }
  
  delete resTmPtr;
  delete stmtTmPtr;

  // Dann die Spieler
  str = "SELECT cp.cpName, cp.cpType, pl.plNr, ntpl.ntNr, "
        "       NULL AS bdplNr, tm.tmName, na.naName,rk.rkNatlRank, rk.rkIntlRank "
        "  FROM LtRec ltpl INNER JOIN CpRec cp ON ltpl.cpID = cp.cpID "
        "                  INNER JOIN PlRec pl ON ltpl.plID = pl.plID "
        "                  LEFT OUTER JOIN NtRec ntpl ON ntpl.ltID = ltpl.ltID "
        "                  LEFT OUTER JOIN TmRec tm ON tm.tmID = ntpl.tmID "
        "                  LEFT OUTER JOIN RkRec rk ON rk.tmID = tm.tmID "
        "                  LEFT OUTER JOIN NaRec na ON na.naID = rk.naID "
        "  WHERE (cp.cpType = 1 OR cp.cpType = 4) "
        " UNION "
        "SELECT cp.cpName, cp.cpType, pl.plNr, ntpl.ntNr, "
        "       bd.plNr AS bdplNr, tm.tmName, na.naName,rk.rkNatlRank, rk.rkIntlRank "
        "  FROM LtRec ltpl INNER JOIN CpRec cp ON ltpl.cpID = cp.cpID "
        "                  INNER JOIN PlRec pl ON ltpl.plID = pl.plID "
        "                  LEFT OUTER JOIN NtRec ntpl ON ntpl.ltID = ltpl.ltID "
        "                  LEFT OUTER JOIN TmRec tm ON tm.tmID = ntpl.tmID "
        "                  LEFT OUTER JOIN RkRec rk ON rk.tmID = tm.tmID "
        "                  LEFT OUTER JOIN NaRec na ON na.naID = rk.naID "        
        "                  LEFT OUTER JOIN NtRec ntbd ON ntbd.tmID = ntpl.tmID AND ntbd.ntNr <> ntpl.ntNr "
        "                  LEFT OUTER JOIN LtRec ltbd ON ltbd.ltID = ntbd.ltID "
        "                  LEFT OUTER JOIN PlRec bd ON ltbd.plID = bd.plID "
        " WHERE ((cp.cpType = 2) OR (cp.cpType = 3)) AND (ntbd.ntNr IS NULL OR ntbd.ntNr <> ntpl.ntNr) "
        " ORDER BY cp.cpName, pl.plNr";
    
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr  = 0;
  
  try
  {
    if ( stmtPtr->Execute(str) )
      resPtr = stmtPtr->GetResultSet(false);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }
    
  resPtr->BindCol(1, cpName, sizeof(cpName));
  resPtr->BindCol(2, &cpType);
  resPtr->BindCol(3, &plNr);
  resPtr->BindCol(4, &ntNr);
  resPtr->BindCol(5, &bdNr);
  resPtr->BindCol(6, tmName, sizeof(tmName));
  resPtr->BindCol(7, naName, sizeof(naName));
  resPtr->BindCol(8, &rkNatlRank);
  resPtr->BindCol(9, &rkIntlRank);
  
  ntNr = bdNr = rkNatlRank = rkIntlRank = 0;
  tmName[0] = naName[0] = 0; 
  
  short lastCpType = 0;   
    
  while (resPtr->Next())
  {
    wxString ofs;

    switch (cpType)
    {
      case CP_SINGLE :
        if (cpType != lastCpType)
          ofs << "# Pl. No.; Event; Partner No.; "
                 "Association; Natl. Ranking; Int'l Ranking" << '\n';
          
        ofs << plNr << ";" << cpName << ";;" 
            << naName << ";" << rkNatlRank << ";" << rkIntlRank << '\n';
        break;
        
      case CP_DOUBLE :
      case CP_MIXED :
        if (cpType != lastCpType)
          ofs << "# Pl. No.; Event; Partner No.; "
                 "Association; Natl. Ranking; Int'l Ranking" << '\n';
              
        ofs << plNr << ";" << cpName << ";" << bdNr << ";" 
            << naName << ";" << rkNatlRank << ";" << rkIntlRank << '\n';
        break;
        
      case CP_TEAM :
        if (cpType != lastCpType)
          ofs << "# Pl. No.; Event; Team; Team Pos." << '\n';
          
        ofs << plNr << ";" << cpName << ";" << tmName << ";" << ntNr << '\n';
        break;
    }

    os << ofs.ToUTF8();

    lastCpType = cpType;
    
    ntNr = bdNr = rkNatlRank = rkIntlRank = 0;
    tmName[0] = naName[0] = 0;        
  }
  
  delete resPtr;
  delete stmtPtr;

  return true;
}


// -----------------------------------------------------------------------
bool LtStore::RemoveFromDoubles(const wxString &name)
{
  wxTextFile ifs(name);
  if (!ifs.Open())
    return false;

  wxString line = ifs.GetFirstLine();

  if (wxStrcmp(line, "#REMOVE DOUBLES"))
  {
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#REMOVE DOUBLES"), line.wx_str()))
      return false;
  }

  std::list<wxString> unknownPlayerList;

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  // HACK: Die Variablen duerfen nicht laenger leben als connPtr [
  {
  for (; !ifs.Eof(); line = ifs.GetNextLine())
  {
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    wxStringTokenizerEx tokens(line, ",;\t");

    wxString strPl = tokens.GetNextToken().Strip(wxString::both);

    PlStore pl(connPtr);
    pl.SelectByExtId(strPl);
    if (!pl.WasOK())
    {
      pl.Close();
      CTT32App::ProgressBarStep();
      continue;
    }

    pl.Close();

    LtStore lt(connPtr);
    lt.SelectByPl(pl.plID);
    std::list<long> cpList;

    while (lt.Next())
      cpList.push_back(lt.cpID);

    lt.Close();

    while (cpList.size() > 0)
    {
      long cpID = cpList.front();
      cpList.pop_front();

      CpStore cp(connPtr);
      cp.SelectById(cpID);
      cp.Next();
      cp.Close();

      if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
        cp.RemovePlayer(pl);
    }

    connPtr->Commit();
    CTT32App::ProgressBarStep();
  }
  } // end HACK ]

  try
  {
    delete connPtr;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("<none>", e);
  }

  return true;
}