/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "GrStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"
#include  "Rec.h"

#include  "IdStore.h"
#include  "CpStore.h"
#include  "StStore.h"
#include  "MtStore.h"
#include  "SyStore.h"
#include  "MdStore.h"
#include  "TmStore.h"
#include  "XxStore.h"

#include  "CpListStore.h"
#include  "MtListStore.h"
#include  "TmEntryStore.h"

#include  "TBSort.h"
#include  "TbItem.h"

#include  "wxStringTokenizerEx.h"

#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>

// -----------------------------------------------------------------------
long  GrRec::QryToGroupWinner(const MtRec &mt)
{
  return mt.mtEvent.grID;
}


short GrRec::QryToRoundWinner(const MtRec &mt)
{
  if (!mt.mtEvent.mtChance)
  {
    if (mt.mtEvent.mtRound == NofRounds(false))
      return 0;
    return mt.mtEvent.mtRound + 1;
  }
  else
  {
    // Trostrunde
    if (mt.mtEvent.mtRound == NofRounds(true))
      return NofRounds(false);           // Sieger der Trostrunde ins Finale
    else
	    return mt.mtEvent.mtRound + 1;
	}
}


short GrRec::QryToMatchWinner(const MtRec &mt)
{
  // Abkuerzung fuer Play Off (Max. 32 Spieler)
  int tabPLO[4][8] = {{1,2,3,4,5,6,7,8},
                      {1,2,3,4,9,10,11,12},
                      {1,2,5,6,9,10,13,14},
                      {1,3,5,7,9,11,13,15}
										 };
										 
  switch (grModus)
  {
    case MOD_PLO :
    {
      int rd = 5 - NofRounds();
		  return tabPLO[mt.mtEvent.mtRound -1 + rd][(mt.mtEvent.mtMatch -1)/2];
	  }
          
    case MOD_SKO :
      return (mt.mtEvent.mtMatch + 1) / 2;
      
    case MOD_DKO :
    case MOD_MDK :
    {
      if (!mt.mtEvent.mtChance)                     
      {
        // Hauptrunde
		    return (mt.mtEvent.mtMatch + 1) / 2;
		  }
      else
      {
        // Trosrunde
        if (mt.mtEvent.mtRound == NofRounds(true))
          return 1;
        else if (mt.mtEvent.mtRound & 0x1)
          return mt.mtEvent.mtMatch;
        else
			    return (mt.mtEvent.mtMatch + 1) / 2;
      }
    }
    
    default :
      return 0;
  }
}


short GrRec::QryToAXWinner(const MtRec &mt)
{
  switch (grModus)
  {
    case MOD_PLO :
    case MOD_SKO :
        return (mt.mtEvent.mtMatch & 0x1) ? +1 : -1; // Aus ungrade auf A, sonst X

    case MOD_DKO :
    case MOD_MDK :
    {
      if (!mt.mtEvent.mtChance)
      {
        // Hauptrunde
        return (mt.mtEvent.mtMatch & 0x1) ? +1 : -1; // Aus ungrade auf A, sonst X
      }
      else
      {
        if (mt.mtEvent.mtRound == NofRounds(true)) // Aus dem Finale Trostrunde
          return -1;                               // wird X
        else if (mt.mtEvent.mtRound & 0x1)         // in ungeraden Runden auf A (bzw. X)
        {
          if (grModus == MOD_DKO)
            return +1;
          else
            return -1;
        }
        else
	        return (mt.mtEvent.mtMatch & 0x1) ? +1 : -1;  // sonst wie in Hauptrunde
      }
    }
    
    default :
      return 0;
  }  
}


long  GrRec::QryToGroupLoser(const MtRec &mt)
{
  return mt.mtEvent.grID;
}


short GrRec::QryToRoundLoser(const MtRec &mt)
{
  switch (grModus)
  {
    case  MOD_SKO :
      return 0;      // Scheiden aus

    case  MOD_PLO :
      if (mt.mtEvent.mtRound == NofRounds())
        return 0;
      else
		    return mt.mtEvent.mtRound + 1;

    case  MOD_DKO :
    case  MOD_MDK :
      if (mt.mtEvent.mtChance)
        return 0;   // Verlierer der Trostrunde scheiden aus

      if (mt.mtEvent.mtRound == NofRounds(false))
        return NofRounds(true);  // Aus erstem Endspiel in zweites Endspiel
      else if (mt.mtEvent.mtRound == 1)
        return 1;   // Trostrunde beginnt ebenfalls bei 1
      else
		    return (mt.mtEvent.mtRound - 1) * 2;
  } // end switch

  return 0;
}


short GrRec::QryToMatchLoser(const MtRec &mt)
{
	int   tabPLO[4][8] = {{9,10,11,12,13,14,15,16},
												{5,6,7,8,13,14,15,16},
												{3,4,7,8,11,12,15,16},
												{2,4,6,8,10,12,14,16}
											 };

	switch (grModus)
	{
		case  MOD_RR  : // In Round Robin eh alles bekannt
		case  MOD_SKO : // Verlierer in Einfach KO scheiden aus
      return 0;

	  case  MOD_PLO :
	  {
		  int  rd = 5 - NofRounds();
		  int  ret = tabPLO[mt.mtEvent.mtRound - 1 + rd][(mt.mtEvent.mtMatch - 1)/2];
      return ret <= NofMatches(mt.mtEvent.mtRound + 1) ? ret : 0;
    }
    
    case  MOD_DKO :
    {
      if (mt.mtEvent.mtChance)
        return 0;   // Zweimal verloren
      else if (mt.mtEvent.mtRound == NofRounds(false))
        return 1;   // Zweites Endspiel
      else if (mt.mtEvent.mtRound == 1)
		    return (mt.mtEvent.mtMatch + 1) / 2;
      else
			{
        int rd = (mt.mtEvent.mtRound-1) * 2;
        if (NofMatches(rd, true) == 1)
					return  1;
        else
			    return (mt.mtEvent.mtMatch & 0x1 ?
                  mt.mtEvent.mtMatch + 1   :
                  mt.mtEvent.mtMatch - 1);
      }
    }
      
    case MOD_MDK :
    {
      if (mt.mtEvent.mtChance)
        return 0;  // Zweimal verloren
        
      switch (mt.mtEvent.mtRound)
      {
        case 1 :
        {
          // 1. Runde vertauscht die Haelften (Zahl der Spiele ist grSize / 2)
          switch ( (mt.mtEvent.mtMatch - 1) / (grSize / 4))
          {
            case 0 :
              return (mt.mtEvent.mtMatch + 1) / 2 + grSize / 8;
            case 1 :
              return (mt.mtEvent.mtMatch + 1) / 2 - grSize / 8;
              
            default :
              return 0;
          }
        }
          
        case 2 :
        {
          // 2. Runde 1 : 1
          return mt.mtEvent.mtMatch;
        }
          
        case 3 :
        {
          // Rueckwaerts eingetragen? Vertauschte Haelften und Viertel?
          switch (mt.mtEvent.mtMatch)
          {
            case 1 :
              return 4;
            case 2 :
              return 3;
            case 3 :
              return 2;
            case 4 :
              return 1;
              
            default :
              return 0;
          }
        }
        
        case 4 :
        {
          // 4. Runde ??? 1 : 1
          return mt.mtEvent.mtMatch;
        }
        
        default :
          return 0;                      
      }
    }
    
  } // end case

  return 0;
}


short GrRec::QryToAXLoser(const MtRec &mt)
{
  switch (grModus)
	{
    case  MOD_SKO :    // Verlierer scheiden aus
    case  MOD_RR  :    // Sinnlos
      return 0;

    case  MOD_PLO :    // Aus ungeraden Spielen auf A, aus geraden auf X
      return (mt.mtEvent.mtMatch & 0x1) ? +1 : -1;

    case  MOD_DKO :
    case  MOD_MDK :
      if (mt.mtEvent.mtChance) // Scheiden aus
        return 0;

			// In Runde 1 haengt es vom Platz ab, sonst auf X (bzw. A)
      if (mt.mtEvent.mtRound == 1)
        return (mt.mtEvent.mtMatch & 0x1) ? +1 : -1;
			else
			{
			  if (grModus == MOD_DKO)
          return -1;
        else
          return 1;
      }        
  } // end case

  return 0;
}


bool  GrRec::QryToWinner(const MtRec &mt, MtRec *mtPtr, short &ax)
{
  if (!mtPtr)
    return false;
    
  if ( !(mtPtr->mtEvent.grID = QryToGroupWinner(mt))    ||
       !(mtPtr->mtEvent.mtRound = QryToRoundWinner(mt)) ||
       !(mtPtr->mtEvent.mtMatch = QryToMatchWinner(mt)) ||
       !(ax = QryToAXWinner(mt))
     )
    return false;
    
  if (grModus != MOD_DKO && grModus != MOD_MDK)
    return true;
    
  // Trostrundenflag setzen. 
  // Nur rein theoretisch kommen die Spieler auch weiter
  if (mt.mtEvent.mtRound < NofRounds(mt.mtEvent.mtChance ? true : false) - 1)
  {
    // Sieger bleiben bis zur vorletzten Runde in ihrer Haelfte
    mtPtr->mtEvent.mtChance = mt.mtEvent.mtChance;
  }
  else if (mt.mtEvent.mtRound == NofRounds(mt.mtEvent.mtChance ? true : false) - 1)
  {
    // Sieger der letzten regulaeren Runde komtm ins Endspiel der Hauptrunde
    mtPtr->mtEvent.mtChance = false; 
  }
  else if (!mt.mtEvent.mtChance && mt.mtEvent.mtRound == NofRounds(false))
  {
    // Sieger des ersten Endspiels kommt ins zweite Endspiel (oder auch nicht)
    mtPtr->mtEvent.mtChance = true;
  }

  return true;
}


bool  GrRec::QryToLoser(const MtRec &mt, MtRec *mtPtr, short &ax)
{
  if (!mtPtr)
    return false;
    
  // Keine dritte Chance
  if (mt.mtEvent.mtChance)
    return false;

  if ( !(mtPtr->mtEvent.grID = QryToGroupLoser(mt))    ||
       !(mtPtr->mtEvent.mtRound = QryToRoundLoser(mt)) ||
       !(mtPtr->mtEvent.mtMatch = QryToMatchLoser(mt)) ||       
       !(ax = QryToAXLoser(mt))
     )
    return false;
    
  // Doppel-KO und Konsorten setzen das mtChance-Bit.
  // Ich kann es oben nicht setzen, weil der Wertebereich eben 0/1 ist.
  if (grModus == MOD_DKO || grModus == MOD_MDK)
    mtPtr->mtEvent.mtChance = 1;

  return true;
}


// -----------------------------------------------------------------------
short GrRec::QryFromRound(const MtRec &mt, int ax)
{
  switch (grModus)
  {
    case MOD_RR :
      return 0;
      
    case MOD_PLO :
      return 0;
      
    case MOD_SKO :
      return mt.mtEvent.mtRound - 1;
      
    case MOD_DKO :
    case MOD_MDK :
      if (!mt.mtEvent.mtChance)
        return mt.mtEvent.mtRound - 1;
      else if (mt.mtEvent.mtRound == 1)
        return 1;
      else if (mt.mtEvent.mtRound % 0x1)
        return mt.mtEvent.mtRound - 1;
      else
      {
        if (grModus == MOD_DKO && ax > 0 ||
            grModus == MOD_MDK && ax < 0)
          return mt.mtEvent.mtRound - 1;
        else
          return mt.mtEvent.mtRound / 2 + 1;
      }
        
    default :
      return 0;      
  }
}


short GrRec::QryFromMatch(const MtRec &mt, int ax)
{
  switch (grModus)
  {
    case MOD_RR :
      return 0;
      
    case MOD_PLO :
      return 0;
      
    case MOD_SKO :
      if (ax > 0)
        return 2 * mt.mtEvent.mtMatch - 1;
      else
        return 2 * mt.mtEvent.mtMatch;
        
    case MOD_DKO :
      if (!mt.mtEvent.mtChance)
      {
        if (ax > 0)
          return 2 * mt.mtEvent.mtMatch - 1;
        else
          return 2 * mt.mtEvent.mtMatch;
      }
      else if (mt.mtEvent.mtRound == 1)
      {
        if (ax > 0)
          return 2 * mt.mtEvent.mtMatch - 1;
        else
          return 2 * mt.mtEvent.mtMatch;
      }
      else if (mt.mtEvent.mtRound % 1)
      {
        return mt.mtEvent.mtMatch;
      }
      else
      {
        if (ax > 0)
          return 2 * mt.mtEvent.mtMatch - 1;
        else
          return 2 * mt.mtEvent.mtMatch;
      }
      
    case MOD_MDK :
      if (!mt.mtEvent.mtChance)
      {
        if (ax > 0)
          return 2 * mt.mtEvent.mtMatch - 1;
        else
          return 2 * mt.mtEvent.mtMatch;
      }
      else if (mt.mtEvent.mtRound == 1)
      {
        MtRec  tmp = mt;
        tmp.mtEvent.mtChance = 0;
        tmp.mtEvent.mtRound = QryFromRound(mt, ax);
        for (int i = NofMatches(tmp.mtEvent.mtRound, false); i; i--)
        {
          tmp.mtEvent.mtMatch = i;
          if (QryToMatchLoser(tmp) == mt.mtEvent.mtMatch &&
              QryToAXLoser(tmp) == ax)
            return i;
        }                
      }
      else if ( (ax > 0 && (mt.mtEvent.mtRound %0x1) == 0))
      {
        MtRec  tmp = mt;
        tmp.mtEvent.mtChance = 0;
        tmp.mtEvent.mtRound = QryFromRound(mt, ax);
        for (int i = NofMatches(tmp.mtEvent.mtRound, false); i; i--)
        {
          tmp.mtEvent.mtMatch = i;
          if (QryToMatchLoser(tmp) == mt.mtEvent.mtMatch)
            return i;
        }
        
        return 0;
      }
      else
      {
        MtRec  tmp = mt;
        tmp.mtEvent.mtChance = 1;
        tmp.mtEvent.mtRound = QryFromRound(mt, ax);
        for (int i = NofMatches(tmp.mtEvent.mtRound, true); i; i--)
        {
          tmp.mtEvent.mtMatch = i;
          if (QryToMatchLoser(tmp) == mt.mtEvent.mtMatch)
            return i;
        }
        
        return 0;
      }
      
    default :
      return 0;
  }
}



// =======================================================================
bool  GrStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  wxString  sql = 
    "CREATE TABLE GrRec (         "
    "grID        "+INTEGER+"      NOT NULL,  "
    "cpID        "+INTEGER+"      NOT NULL,  "
    "syID        "+INTEGER+",     "
    "mdID        "+INTEGER+",     "
    "grName      "+WVARCHAR+"(8)  NOT NULL,  "
	  "grDesc      "+WVARCHAR+"(64) NOT NULL,  "
    "grStage     "+WVARCHAR+"(64) NOT NULL,  "
    "grModus     "+SMALLINT+"     NOT NULL,  "
    "grSize      "+SMALLINT+"     NOT NULL,  "
    "grWinner    "+SMALLINT+"     NOT NULL,  "
    "grBestOf    "+SMALLINT+"     NOT NULL,  "
    "grQualRounds "+SMALLINT+"    DEFAULT 0 NOT NULL, "
    "grNofRounds "+SMALLINT+"     DEFAULT 0 NOT NULL, "
    "grNofMatches "+SMALLINT+"    DEFAULT 0 NOT NULL, "
    "grNoThirdPlace "+SMALLINT+"  DEFAULT 0 NOT NULL, "
    "grOnlyThirdPlace "+SMALLINT+" DEFAULT 0 NOT NULL, "
    "grPublished  "+SMALLINT+" DEFAULT 0 NOT NULL, "
    "grNotes     " + WVARCHAR + "(MAX) DEFAULT NULL, "
    "grSortOrder " + SMALLINT + " DEFAULT NULL, "
    "grPrinted   " + TIMESTAMP + " DEFAULT NULL, "
    "CONSTRAINT grIdKey PRIMARY KEY (grID),   "
    "grCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "grModifiedBy AS (SUSER_SNAME()), "
    "grStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "grEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (grStartTime, grEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.GrHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE INDEX grNameKey ON GrRec (grName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  GrStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  wxString str;
    
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  if (!connPtr)
    return false;

  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  Statement *stmtPtr = NULL;

  try
  {
    stmtPtr = connPtr->CreateStatement();
    if (!stmtPtr)
      return false;

    if (version < 74)
    {
      str = "ALTER TABLE GrRec ADD "
        "grQualRounds "+SMALLINT+" DEFAULT 0 NOT NULL"
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 84)
    {
      str = "ALTER TABLE GrRec ADD "
        "grNofRounds  "+SMALLINT+" DEFAULT 0 NOT NULL, "
        "grNofMatches "+SMALLINT+" DEFAULT 0 NOT NULL  "
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 90)
    {
      str = "ALTER TABLE GrRec ADD "
        "grNoThirdPlace "+SMALLINT+" DEFAULT 0 NOT NULL "
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 114)
    {
      str = "ALTER TABLE GrRec ADD "
        "grOnlyThirdPlace "+SMALLINT+" DEFAULT 0 NOT NULL "
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 119)
    {
      str = "ALTER TABLE GrRec ADD "
        "grPublish  " + SMALLINT + " DEFAULT 0 NOT NULL "
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 131)
    {
      str = "ALTER TABLE GrRec ADD "
        "grNote " + WVARCHAR + "(MAX)DEFAULT NULL "
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 137)
    {
      bool updateSmsTrigger = false;
      try
      {
        stmtPtr->ExecuteUpdate("DROP TRIGGER grSmsUpdateTrigger");
        updateSmsTrigger = true;
      }
      catch (SQLException)
      {
        // Nothing: Trigger didn't exist
      }

      str = "sp_rename 'dbo.GrRec.grPublish ', 'grPublished', 'COLUMN'";

      stmtPtr->ExecuteUpdate(str);

      str = "sp_rename 'dbo.GrRec.grNote ', 'grNotes', 'COLUMN'";

      stmtPtr->ExecuteUpdate(str);

      if (updateSmsTrigger)
      {
        str = "CREATE TRIGGER grSmsUpdateTrigger ON GrRec FOR UPDATE AS "
              " UPDATE smscenter_groups "
              "    SET state_enabled = inserted.grPublished "
              "   FROM inserted INNER JOIN deleted ON inserted.grID = deleted.grID "
              "  WHERE smscenter_groups.grID = inserted.grID "
              "    AND smscenter_groups.state_manual = 0 "
              "    AND inserted.grPublished <> deleted.grPublished "
          ;

        stmtPtr->ExecuteUpdate(str);
      }
    }

    if (version < 145)
    {
      str = "ALTER TABLE GrRec ADD "
        "grSortOrder " + SMALLINT + " DEFAULT NULL "
        ;

      stmtPtr->ExecuteUpdate(str);

      str = "UPDATE GrRec SET grSortOrder = "
        "(SELECT COUNT(DISTINCT grStage) FROM GrRec gr WHERE gr.cpID = GrRec.cpID AND gr.grStage <= GrRec.grStage)"
        ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 148)
    {
      str = "ALTER TABLE GrRec ADD "
        "grPrinted " + TIMESTAMP + " DEFAULT NULL "
        ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 149)
    {
      // History
      str = "ALTER TABLE GrRec ADD "
        "grCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "grModifiedBy AS (SUSER_SNAME()), "
        "grStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "grEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (grStartTime, grEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE GrRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.GrHist))";
      stmtPtr->ExecuteUpdate(str);
    }
  }
  catch (SQLException &ex)
  {
    infoSystem.Exception(str, ex);
    delete stmtPtr;
    return false;
  }

  delete stmtPtr;

  return true;
}


bool  GrStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteQuery("ALTER TABLE GrRec DROP CONSTRAINT grSyRef");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteQuery("ALTER TABLE GrRec DROP CONSTRAINT plMdRef");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteQuery("ALTER TABLE GrRec DROP CONSTRAINT grMdRef");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteQuery("ALTER TABLE GrRec DROP CONSTRAINT grCpRec");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteQuery("ALTER TABLE GrRec DROP CONSTRAINT grCpRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE GrRec ADD CONSTRAINT grSyRef "
      "FOREIGN KEY (syID) REFERENCES SyRec (syID) ON DELETE NO ACTION");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE GrRec ADD CONSTRAINT grMdRef "
      "FOREIGN KEY (mdID) REFERENCES MdRec (mdID) ON DELETE NO ACTION");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE GrRec ADD CONSTRAINT grCpRef "
      "FOREIGN KEY (cpID) REFERENCES CpRec (cpID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  GrStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
bool  GrStore::InsertOrUpdate()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  wxString  sql = "SELECT grID FROM GrRec WHERE cpID = " + ltostr(cpID) + " AND grName = '" + grName + "'";
  stmtPtr = GetConnectionPtr()->CreateStatement();
  if (!stmtPtr)
    return false;

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    grID = id;
    return Update();
  }
  else
  {
    CpRec cp;
    cp.cpID = cpID;
    return Insert(cp);
  }
}


bool  GrStore::Insert(const CpRec &cp)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO GrRec "
                    "      (grID, cpID, syID, mdID, grName, grDesc, "
                    "       grStage, grModus, grSize, grBestOf, grWinner, "
                    "       grQualRounds, grNofRounds, grNofMatches, grNoThirdPlace, grOnlyThirdPlace, grSortOrder) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  try
  {
    grID = IdStore::ID(GetConnectionPtr());
    cpID = cp.cpID;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &grID);
    stmtPtr->SetData(2, &cpID);
    stmtPtr->SetData(3, (syID ? &syID : (long *) NULL));
    stmtPtr->SetData(4, (mdID ? &mdID : (long *) NULL));
    stmtPtr->SetData(5, grName);
    stmtPtr->SetData(6, grDesc);
    stmtPtr->SetData(7, grStage);
    stmtPtr->SetData(8, &grModus);
    stmtPtr->SetData(9, &grSize);
    stmtPtr->SetData(10, &grBestOf);
    stmtPtr->SetData(11, &grWinner);
    stmtPtr->SetData(12, &grQualRounds);
    stmtPtr->SetData(13, &grNofRounds);
    stmtPtr->SetData(14, &grNofMatches);
    stmtPtr->SetData(15, &grNoThirdPlace);
    stmtPtr->SetData(16, &grOnlyThirdPlace);
    stmtPtr->SetData(17, &grSortOrder);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;
    return false;
  }

  delete stmtPtr;

  return true;
}


bool  GrStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE GrRec "
                    "SET grName = ?, grDesc = ?, grStage = ?, grWinner = ?, syID = ?, mdID = ?, "
                    "grQualRounds = ?, grNofRounds = ?, grNofMatches = ?, grNothirdPlace = ?, grOnlyThirdPlace = ?, grBestOf = ?, grSortOrder = ? "
                    "WHERE grID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, grName);
    stmtPtr->SetData(2, grDesc);
    stmtPtr->SetData(3, grStage);
    stmtPtr->SetData(4, &grWinner);
    stmtPtr->SetData(5, syID ? &syID : (long *) NULL);
    stmtPtr->SetData(6, mdID ? &mdID : (long *) NULL);
    stmtPtr->SetData(7, &grQualRounds);
    stmtPtr->SetData(8, &grNofRounds);
    stmtPtr->SetData(9, &grNofMatches);
    stmtPtr->SetData(10, &grNoThirdPlace);
    stmtPtr->SetData(11, &grOnlyThirdPlace);
    stmtPtr->SetData(12, &grBestOf);
    stmtPtr->SetData(13, &grSortOrder);
    stmtPtr->SetData(14, &grID);

    stmtPtr->Execute();
    
    // Best Of aendern
    {
      str = "UPDATE MtRec SET mtBestOf = ";
      str += ltostr(grBestOf);
      str += " WHERE grID = ";
      str += ltostr(grID);
      str += " AND mtResA = 0 AND mtResX = 0";
    
      Statement *tmp = GetConnectionPtr()->CreateStatement();
      tmp->ExecuteUpdate(str);
    
      delete tmp;
    }
    
    // Spielsystem koennte sich geaendert haben
    if (syID != 0)
    {
      str = "UPDATE MtRec SET mtMatches = (SELECT syMatches FROM SyRec WHERE syID = ";
      str += ltostr(syID);
      str += ") WHERE grID = " + ltostr(grID);
        
      Statement *tmp = GetConnectionPtr()->CreateStatement();
      tmp->ExecuteUpdate(str);
      
      delete tmp;
    }
    
    // Update Gruppenmodus
    if (mdID != 0)
    {
      str ="UPDATE MtRec SET stA = "
           "  (SELECT stID FROM StRec WHERE stNr = "
           "      (SELECT mdPlayerA FROM MdMatch "
           "        WHERE MdMatch.mdRound = MtRec.mtRound "
           "          AND MdMatch.mdMatch = MtRec.mtMatch "
           "          AND MdMatch.mdID = ";
      str += ltostr(mdID);
      str += "    ) AND StRec.grID = ";
      str += ltostr(grID);
      str += "), stX = "
           "  (SELECT stID FROM StRec WHERE stNr = "
           "      (SELECT mdPlayerX FROM MdMatch "
           "        WHERE MdMatch.mdRound = MtRec.mtRound "
           "          AND MdMatch.mdMatch = MtRec.mtMatch "
           "          AND MdMatch.mdID = ";
      str += ltostr(mdID);
      str += "    ) AND StRec.grID = ";
      str += ltostr(grID);
      str += ") WHERE MtRec.grID = ";
      str += ltostr(grID);
        
      Statement *tmp = GetConnectionPtr()->CreateStatement();
      tmp->ExecuteUpdate(str);
      
      delete tmp;
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;
    return false;
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::GRREC;
  update.id   = grID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  GrStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = grID;

  if (!id)
    return true;

  wxString str = "DELETE FROM GrRec WHERE grID = ";
  str += ltostr(id);
  str += "";

  try
  {
    // Spiele und Setzung loeschen. MT und ST brauchen die ID
    grID = id;
  
    // All XxRec mit GR als Quelle und Ziel loeschen
    XxStore(GetConnectionPtr()).Remove(*this, 0); 
    // MtStore(GetConnectionPtr()).Remove(*this);
    // StStore(GetConnectionPtr()).Remove(*this);

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
  update.rec  = CRequest::GRREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool  GrStore::SelectAll(const CpRec &cp)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool GrStore::SelectAll(const CpRec &cp, const wxString &stage)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID);
  if (!stage.IsEmpty())
  {
    str += "   AND grStage = '";
    str += stage;
    str += "' ";
  }  
  str += " ORDER BY grName";
         
  try
  {
    if (!ExecuteQuery(str))
      return false;
      
    BindRec();    
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }
  
  return true;
}


bool  GrStore::SelectById(long id)
{
  wxString str = SelectString();
  str += "WHERE grID = ";
  str += ltostr(id);
  str += "";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool  GrStore::SelectByName(const wxString &name, const CpRec &cp)
{
  wxString str = SelectString();
  str += "WHERE grName = '";
  str += name;
  str += "' AND cpID = " + ltostr(cp.cpID);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


wxString  GrStore::SelectString() const
{
  wxString  str = 
    "SELECT grID, cpID, syID, mdID, grName, grDesc, grStage, "
    "       grModus, grSize, grWinner, grBestOf, grQualRounds, "
    "       grNofRounds, grNofMatches, grNoThirdPlace, grOnlyThirdPlace,  "
    "       grPublished, grSortOrder, CASE WHEN grNotes IS NULL THEN 0 ELSE 1 END AS grHasNotes, "
    "       grPrinted "
    "  FROM GrRec ";

  return str;
}


bool  GrStore::BindRec()
{
  BindCol(1, &grID);
  BindCol(2, &cpID);
  BindCol(3, &syID);
  BindCol(4, &mdID);
  BindCol(5, grName, sizeof(grName));
  BindCol(6, grDesc, sizeof(grDesc));
  BindCol(7, grStage, sizeof(grStage));
  BindCol(8, &grModus);
  BindCol(9, &grSize);
  BindCol(10, &grWinner);
  BindCol(11, &grBestOf);
  BindCol(12, &grQualRounds);
  BindCol(13, &grNofRounds);
  BindCol(14, &grNofMatches);
  BindCol(15, &grNoThirdPlace);
  BindCol(16, &grOnlyThirdPlace);
  BindCol(17, &grPublished);
  BindCol(18, &grSortOrder);
  BindCol(19, &grHasNotes);
  BindCol(20, &grPrinted);

  return true;
}


void  GrStore::Init()
{
  GrRec::Init();
}

// -----------------------------------------------------------------------
bool  GrStore::SetWinner(const MtRec &rec)
{
  // Macht keinen Sinn in Round Robin
	if (grModus == MOD_RR)
    return true;

  // Abbruchbedingung ist, wenn an entsprechender Stelle schon NULL steht.

  short    ax;               // Gewinner wird A/X im naechsten Spiel
  long     stID;
  MtStore  mt(GetConnectionPtr());
  MtStore  mtw(GetConnectionPtr());

  // mt muss neu eingelesen werden, da in Folge nur die 
  // stID gesetzt wird, aber nicht die tmID.
  if (!mt.SelectById(rec.mtID) || !mt.Next())
    return false;
  mt.Close();

	switch (mt.QryWinnerAX())
	{
		case  0:  // kein Gewinner
      stID = 0;
			break;

		case +1:  // Sieger ist A
      stID = mt.stA;
			break;

		case -1:  // Sieger ist X
      stID = mt.stX;
			break;
	}

	// TODO: Update von st
	// CalculatePosWinner(st, mt);

	// evt. eine Abfrage zuviel. 
  // Liefert false, wenn das naechste Spiel nicht existiert
  // XxRec berucksichtigen. Derzeit nur Sieger KO-Gruppe
  if ( mt.mtEvent.mtRound == NofRounds() )
  {
    int pos = 0;
    
    if (grModus == MOD_SKO)
      pos = 1;
    else
      return true;
      
    XxStore xx(GetConnectionPtr());
    xx.Select(*this, pos);
    if (!xx.Next())
      return true;
    xx.Close();
    
    StStore st(GetConnectionPtr());
    st.SelectById(xx.stID);
    if (!st.Next())
      return true;      
    st.Close();
    
    long tmID;
    if (stID == 0)
      tmID = 0;
    else
    {
      StStore stTM(GetConnectionPtr());
      stTM.SelectById(stID);
      if (!stTM.Next())
        return false;
      stTM.Close();  
      
      tmID = stTM.tmID;
    }
    
    return st.SetTeam(tmID);
  }
  
	if (!QryToWinner(mt, &mtw, ax))
    return true;

	if (!mtw.SelectByEvent(mtw.mtEvent) || !mtw.Next())
		return false;

  mtw.Close();

	if (ax == +1)               // Gewinner wird Spieler A
  {
    // Abbruchbedingung: Ich muss weiterlaufen, bis der Spieler unbekannt ist.
    // Erst danach koennen nur noch unbekannte Spieler kommen. Breche ich
    // vorher ab, wenn mtw.stA == stID, kann ich nicht entscheiden, ob im 
    // naechsten Spiel ein Freilos durch einen Spieler ersetzt wurde und 
    // damit ein Spiel keinen Sieger mehr hat.
    if (mtw.stA == stID && stID == 0)
      return true;

		if (!mtw.SetTeamAById(stID))
      return false;
  }
	else if (ax == -1)          // Gewinner wird Spieler X
  {
    // Abbruchbedingung: s.o.
    if (mtw.stX == stID && stID == 0)
      return true;

		if (!mtw.SetTeamXById(stID))
      return false;
  }
  else
  {
    // Fehler!
    return false;
  }

	// Sieger fortschreiben
	if (!SetWinner(mtw))
    return false;

	if (!SetLoser(mtw))
    return false;

	return true;
}


bool  GrStore::SetLoser(const MtRec &rec)
{
  // Abbruchbedingung ist, wenn an entsprechender Stelle schon NULL steht.

  short    ax;               // Gewinner wird A/X im naechsten Spiel
  long     stID;
  MtStore  mt(GetConnectionPtr());
  MtStore  mtl(GetConnectionPtr());

	// evt. eine Abfrage zuviel. 
  // Liefert false, wenn das naechste Spiel nicht existiert
	if (!QryToLoser(rec, &mtl, ax))
    return true;

  // mt muss neu eingelesen werden, da in Folge nur die 
  // stID gesetzt wird, aber nicht die tmID.
  if (!mt.SelectById(rec.mtID) || !mt.Next())
    return false;
    
  mt.Close();

	switch (mt.QryWinnerAX())
	{
		case  0:  // kein Gewinner
      stID = 0;
			break;

		case +1:  // Sieger ist A, Verlierer also X
      stID = mt.stX;
			break;

		case -1:  // Sieger ist X, Verlierer also A
      stID = mt.stA;
			break;
	}

	// TODO: Update von st
	// CalculatePosWinner(st, mt);

	if (!mtl.SelectByEvent(mtl.mtEvent) || !mtl.Next())
		return false;

  mtl.Close();

	if (ax == +1)               // Verlierer wird Spieler A
  {
    // Abbruchbedingung: s.o.
    if (mtl.stA == stID && stID == 0)
      return true;

		if (!mtl.SetTeamAById(stID))
      return false;
  }
	else if (ax == -1)          // Verlierer wird Spieler X
  {
    // Abbruchbedingung: s.o.
    if (mtl.stX == stID && stID == 0)
      return true;

		if (!mtl.SetTeamXById(stID))
      return false;
  }
  else
  {
    // Fehler!
    return false;
  }

	// Sieger fortschreiben
	if (!SetWinner(mtl))
    return false;

	if (!SetLoser(mtl))
    return false;

	return true;
}


// -----------------------------------------------------------------------
bool  GrStore::ClearAllTeams()
{
  StStore  st(GetConnectionPtr());
  return st.ClearAllTeams(*this);
}


bool  GrStore::SetTeam(short pos, const TmEntry &tm, unsigned flags)
{
  StStore  st(GetConnectionPtr());
  st.SelectByNr(grID, pos);
  st.Next();
  st.Close();

  // Hier wird ausdruecklich ein Teilnehmer gesetzt.
  // Daher ein bestehendes XxStore loeschen.
  XxStore(GetConnectionPtr()).Remove(st);
  
  if (!st.SetTeam(*this, pos, tm, flags))
    return false;

  if (grModus == MOD_RR && QryCombined())
  {
    // Wenn die Gruppe combined score sheet ist, fuer alle Spiele mtPrinted zuruecksetzen
    // Die DB-Trigger beruecksichtigen nur die Spiele, fuer die sich der Spieler aendert
    MtStore mt(GetConnectionPtr());
    if (!mt.UpdateScorePrintedForGroup(grID, false))
      return false;
  }

  // Spieler werden in Trigger vorgetragen
  
  return MtStore(GetConnectionPtr()).UpdateTimestamp(st);
}


bool  GrStore::SetTeam(short pos, const TmRec &tm, unsigned flags)
{
  StStore  st(GetConnectionPtr());
  st.SelectByNr(grID, pos);
  st.Next();
  st.Close();

  if (!st.SetTeam(*this, pos, tm.tmID, flags))
    return false;

  if (grModus == MOD_RR && QryCombined())
  {
    // Wenn die Gruppe combined score sheet ist, fuer alle Spiele mtPrinted zuruecksetzen
    // Die DB-Trigger beruecksichtigen nur die Spiele, fuer die sich der Spieler aendert
    MtStore mt(GetConnectionPtr());
    if (!mt.UpdateScorePrintedForGroup(grID, false))
      return false;
  }

  if (grModus != MOD_RR)
  {
    MtStore::MtEvent  mtEvent;
    mtEvent.grID = grID;
    mtEvent.mtRound = 1;
    mtEvent.mtMatch = (pos +1) / 2;
    mtEvent.mtChance = 0;

    MtListStore  mt(GetConnectionPtr());
    if (!mt.SelectByEvent(mtEvent) || !mt.Next())
      return false;

    mt.Close();

    if (!SetWinner(mt))
      return false;

    if (!SetLoser(mt))
      return false;
  }
  
  return MtStore(GetConnectionPtr()).UpdateTimestamp(st);
}


// -----------------------------------------------------------------------
bool GrStore::SetPublish(bool publish)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE GrRec SET grPublished = ? WHERE grID = ?";

  try
  {
    short tmp = publish ? 1 : 0;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &tmp);
    stmtPtr->SetData(2, &grID);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::GRREC;
  update.id   = grID;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
short GrStore::CountGroups(const CpRec &cp)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr = 0;

  wxString str = "SELECT COUNT(*) FROM GrRec "
    "WHERE cpId = " + ltostr(cp.cpID);

  short res;

  try
  {
    stmtPtr = connPtr->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &res);
    if (!resPtr->Next() || resPtr->WasNull(1))
      res = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    res = 0;
  }

  delete resPtr;
  delete stmtPtr;

  return res;
}


short GrStore::CountGroups(const CpRec &cp, const wxString &stage)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString str = "SELECT COUNT(*) FROM GrRec "
    "WHERE cpId = " + ltostr(cp.cpID) + " AND grStage = '" + stage + "'";

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

  return res;
}


short GrStore::CountGroups(const MdRec &md)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString str = "SELECT COUNT(*) FROM GrRec "
    "WHERE mdId = " + ltostr(md.mdID);

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

  return res;
}


short GrStore::CountGroups(const SyRec &sy)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString str = "SELECT COUNT(*) FROM GrRec "
    "WHERE syId = " + ltostr(sy.syID);

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

  return res;
}


// -----------------------------------------------------------------------
bool  GrStore::ClearDraw(const CpRec &cp, const wxString &stage)
{
  Connection * connPtr = GetConnectionPtr();
  Statement *stmtPtr  = connPtr->CreateStatement();

  wxString str = "UPDATE StRec SET tmID = NULL "
    "WHERE stSeeded <> 1 AND grID IN "
    "(SELECT grID FROM GrRec WHERE cpID = " + ltostr(cp.cpID) + 
    " AND grStage = '" + stage + "')";

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }  

  // Delete XxRec from / for this group  
  str = "DELETE XxRec WHERE stID IN (SELECT stID FROM StRec WHERE grID IN "
        " (SELECT grID FROM GrRec WHERE cpID = " + ltostr(cp.cpID) + " AND "
        " grStage = '" + stage + "'))";

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }  
  
  str = "DELETE XxRec WHERE grID IN (SELECT grID FROM GrRec "
        " WHERE cpID = " + ltostr(cp.cpID) + " AND grStage = '" + stage + "')";
  
  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }  
  
  delete stmtPtr;

  return true;
}

bool  GrStore::ClearDraw(long grID)
{
  Connection * connPtr = GetConnectionPtr();
  Statement *stmtPtr = connPtr->CreateStatement();

  wxString str = "UPDATE StRec SET tmID = NULL "
    "WHERE stSeeded <> 1 AND grID = " + ltostr(grID);

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }

  // Delete XxRec from / for this group  
  str = "DELETE XxRec WHERE stID IN (SELECT stID FROM StRec WHERE grID = " + ltostr(grID) + ")";

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }

  str = "DELETE XxRec WHERE grID = " + ltostr(grID);

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  return true;
}

// -----------------------------------------------------------------------
bool  GrStore::QryStarted()
{
  Connection *connPtr = GetConnectionPtr();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr = 0;
  
  wxString str = "SELECT COUNT(*) FROM MtMatch "
    "WHERE ("
    " mtResA <> 0 OR mtResX <> 0 OR "
    " mtWalkOverA <> 0 OR mtWalkOverX <> 0 OR "
    " mtInjuredA <> 0 OR mtInjuredX <> 0 OR "
    " mtDisqualifiedA <> 0 OR mtDisqualifiedX <> 0) "
    " AND mtID IN (SELECT mtID FROM MtRec WHERE grID = " + ltostr(grID) + ")";
    
  long count;

  try
  {
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

    if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;

    delete resPtr;
    delete stmtPtr;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }
      
  return (count ? true : false);
}


bool  GrStore::QryFinished()
{
  Connection *connPtr = GetConnectionPtr();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr = 0;
  
  // Das Statement zaehlt nur Einzelspiele, die noch 0:0 stehen.
  // Allerdings loesche ich alle Einzelspiele, die kein Ergebnis
  // haben und damit ist die Anzahl immer 0.
  
  /*
  wxString str = "SELECT COUNT(*) FROM MtMatch "
    "WHERE (mtResA = 0 AND mtResX = 0) AND mtMS = 0 AND mtID IN "
    "(SELECT mtID FROM MtRec mt "
    "   LEFT OUTER JOIN StRec stA ON mt.stA = stA.stID "
    "   LEFT OUTER JOIN StRec stX ON mt.stX = stX.stID "
    "  WHERE mt.grID = " + ltostr(grID) + " "
    "    AND (mt.stA IS NULL OR stA.tmID IS NOT NULL) "
    "    AND (mt.stX IS NULL OR stX.tmID IS NOT NULL) "
    ")";
  */

  // Alle Spiele zaehlen, die noch nicht fertig sind
  wxString str = "SELECT COUNT(*) FROM MtRec mt "
        " LEFT OUTER JOIN StRec stA ON mt.stA = stA.stID "
        " LEFT OUTER JOIN StRec stX ON mt.stX = stX.stID "
        " WHERE mt.grID = " + ltostr(grID) + " AND "
        "   (stA.stID IS NULL OR stA.tmID IS NOT NULL) AND "
        "   (stX.stID IS NULL OR stX.tmID IS NOT NULL) AND "
		    "   (mtWalkOverA = 0 AND mtWalkOverX = 0) AND "
		    "   (mtInjuredA = 0 AND mtInjuredX = 0) AND "
		    "   (mtDisqualifiedA = 0 AND mtDisqualifiedX = 0) AND "
        "   ((mtMatches = 1 AND 2 * mtResA < mtBestOf AND 2 * mtResX < mtBestOf) OR "
        "    (mtMatches > 1 AND 2 * mtResA < mtMatches AND 2 * mtResX < mtMatches)) "
  ;
  long count;

  try
  {
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

    if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;

    delete resPtr;
    delete stmtPtr;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }
      
  return (count ? false : true);
}


bool  GrStore::QryChecked()
{
  Connection *connPtr = GetConnectionPtr();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr = 0;
  
  // Alle Spiele zaehlen, die noch nicht geprueft sind
  wxString str = "SELECT COUNT(*) FROM MtRec mt "
        " LEFT OUTER JOIN StRec stA ON mt.stA = stA.stID "
        " LEFT OUTER JOIN StRec stX ON mt.stX = stX.stID "
        " WHERE mt.grID = " + ltostr(grID) + " AND "
        "   (stA.stID IS NULL OR stA.tmID IS NOT NULL) AND "
        "   (stX.stID IS NULL OR stX.tmID IS NOT NULL) AND "
        "   mtChecked = 0 "
  ;
  long count;

  try
  {
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

    if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;

    delete resPtr;
    delete stmtPtr;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }
      
  return (count ? false : true);
}


bool  GrStore::QryDraw()
{
  Connection *connPtr = GetConnectionPtr();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr = 0;
  
  wxString str = "SELECT COUNT(*) FROM StRec "
    "WHERE grID = " + ltostr(grID) + 
    " AND stSeeded = 0 AND tmID <> 0";
    
  long count;

  try
  {
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

    if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;

    delete resPtr;
    delete stmtPtr;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }
      
  return (count ? true : false);
}


bool  GrStore::QryScheduled()
{
  Connection *connPtr = GetConnectionPtr();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr = 0;
  
  wxString str = "SELECT COUNT(*) FROM MtRec "
    "WHERE grID = " + ltostr(grID) + 
    " AND mtDateTime IS NOT NULL";
    
  long count;

  try
  {
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

    if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;

    delete resPtr;
    delete stmtPtr;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }
      
  return (count ? true : false);
}


bool  GrStore::QryCombined()
{
  Connection *connPtr = GetConnectionPtr();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr = 0;
  
  wxString str = 
    "SELECT COUNT(*) FROM MtRec "
    " WHERE grID = " + ltostr(grID) +
    " GROUP BY mtDateTime, mtTable"
  ;

  long cols = 0;
  long count = 0;

  try
  {
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

	  while (resPtr->Next())
	  {
      long tmp;
      if (!resPtr->GetData(1, tmp) || resPtr->WasNull())  
      {
        count = 0;
        break;
      }
      else if (++count > 1)
        break;
      else
        cols += tmp;
	  }

    delete resPtr;
    delete stmtPtr;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }
      
  return count == 1 && cols > 1;
}


// -----------------------------------------------------------------------
bool  GrStore::SetTable()
{
  if (grModus != MOD_RR)
    return false;

  StStore  st(GetConnectionPtr());
  st.SelectAll(*this);

  std::vector<TbItem *> tbList;
  std::vector<MtRec>  mtList;

  while (st.Next())
  {
    if (st.tmID)
      tbList.push_back(new TbItem(st, syID ? CP_TEAM : CP_SINGLE));
  }

  // Matches koennen nur einzeln ausgewaehlt werden, 
  // weil ich die Ergebnisse mit lesen muss. Das geschieht
  // jeweils in einem extra Statement, SQL-Server unterstuetzt
  // jedoch nur _ein_ offenes HSTMT
  MtStore  mt(GetConnectionPtr());
  mt.SelectByGr(*this);
  
  while (mt.Next())
    mtList.push_back(mt);
  
  TbSort::Sort(*this, (syID ? CP_TEAM : CP_SINGLE), tbList, mtList);

  for (std::vector<TbItem *>::iterator itTB = tbList.begin(); 
       itTB != tbList.end(); itTB++)
  {
    StStore  st(GetConnectionPtr());
    st.stID = (*itTB)->st.stID;
    st.stPos = (*itTB)->result.pos;

    st.UpdatePosition();

    delete (*itTB);
  }

  tbList.erase(tbList.begin(), tbList.end());
  mtList.erase(mtList.begin(), mtList.end());

  return true;
}


bool  GrStore::ClearTable()
{
  Connection * connPtr = GetConnectionPtr();
  Statement *stmtPtr  = connPtr->CreateStatement();

  wxString str = "UPDATE StRec SET stPos = 0"
    "WHERE grID = " + ltostr(grID);

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete stmtPtr;

    return false;
  }  

  delete stmtPtr;

  return true;
}


// -----------------------------------------------------------------------
// Eine Gruppe mit allen Abhaengigkeiten anlegen
unsigned  GrStore::CreateGroup(void *arg)
{
  CpRec cp      = ((CreateGroupStruct *) arg)->cp;
  GrRec grTempl = ((CreateGroupStruct *) arg)->gr;
  int   start   = ((CreateGroupStruct *) arg)->start;
  int   count   = ((CreateGroupStruct *) arg)->count;
  wxString  nameTempl = ((CreateGroupStruct *) arg)->nameTempl;
  wxString  descTempl = ((CreateGroupStruct *) arg)->descTempl;
  bool  isAlpha   = ((CreateGroupStruct *) arg)->numeric == 2;
  bool  delConnPtr = ((CreateGroupStruct *) arg)->delConnPtr;

  Connection *connPtr = ((CreateGroupStruct *) arg)->connPtr;

  bool defaultConnection = true;
  
  if (!connPtr)
  {
    connPtr = TTDbse::instance()->GetDefaultConnection();
    delConnPtr = false;
  }
  else
    defaultConnection = false;

  wxASSERT(connPtr);

  delete ((CreateGroupStruct *) arg);

  connPtr->StartTransaction();

  // Scope der Variablen begrenzen. 
  // Sie muessen vor dem delete connPtr geloescht werden
  {
  GrStore  gr(connPtr);
  SyStore  sy(connPtr);
  MdStore  md(connPtr);

  if (cp.cpType == CP_TEAM)
  {
    if (!sy.SelectById(grTempl.syID) || !sy.Next())
    {
      connPtr->Rollback();
      if (!defaultConnection)
        delete connPtr;

      return 0;
    }
  }

  if (grTempl.grModus == MOD_RR)
  {
    if (!md.SelectById(grTempl.mdID) || !md.Next())
    {
      connPtr->Rollback();
      if (!defaultConnection)
        delete connPtr;
      return 0;
    }
  }
  
  int nameOffset = (isAlpha ? 'A' - 1 : 0);
  int descOffset = (isAlpha ? 'A' - 1 : 0);

  for (int nr = start; nr < (start + count); nr++)
  {
    gr = grTempl;

    if (nameTempl.Contains("%"))
      wxSnprintf(gr.grName, sizeof(gr.grName), nameTempl.t_str(), nr + nameOffset);
    if (descTempl.Contains("%"))
      wxSnprintf(gr.grDesc, sizeof(gr.grDesc), descTempl.t_str(), nr + descOffset);

    if (!gr.Insert(cp))
    {
      connPtr->Rollback();
      if (!defaultConnection)
        delete connPtr;

      return 0;
    }

    CTT32App::ProgressBarStep();

    // StStore
    StStore  st(connPtr);
    for (int i = 1; i <= gr.grSize; i++)
    {
      st.stNr = i;
      if (!st.Insert(gr))
      {
        connPtr->Rollback();
        if (!defaultConnection)
          delete connPtr;

        return 0;
      }

      CTT32App::ProgressBarStep();
    }

    // MtStore 
    MtStore  mt(connPtr);

    mt.mtBestOf = gr.grBestOf;

    // Anzahl Spiele ohne "Only Third Place" berechnen
    int only3rdPlace = gr.grOnlyThirdPlace;
    gr.grOnlyThirdPlace = 0;
    
    for (int sc = 0; sc <= 1; sc++)
    {
      mt.mtEvent.mtChance = sc;
      for (int round = 1; round <= gr.NofRounds(sc ? true : false); round++)
      {
        mt.mtEvent.mtRound = round;

        for (int match = 1; match <= gr.NofMatches(round, sc ? true : false); match++)
        {
          mt.mtEvent.mtMatch = match;

          if (sy.syID)
            mt.mtMatches = sy.syMatches;
          else
            mt.mtMatches = 1;

          if (!mt.Insert(gr))
          {
            connPtr->Rollback();
            if (!defaultConnection)
              delete connPtr;

            return 0;
          }

          // Spieler A / X setzen
          switch (gr.grModus)
          {
            case MOD_RR :
              mt.SetTeamsByNr(md.GetPlayerA(round, match), md.GetPlayerX(round, match));
              break;

            case MOD_SKO :
              if (mt.mtEvent.mtChance == 0)
              {
                mt.SetTeamsByNr(
                    (1 + (match-1) * (1 << round)), 
                    (1 + (match-1) * (1 << round)) + (1 << (round-1)) );
              }
              break;

            case MOD_MDK :
            case MOD_DKO :
            case MOD_PLO :
            // Ich erspare mir hier die Berechnung und setze unten Sieger und Verlierer
              if (round == 1)
              {
                mt.SetTeamsByNr(2 * match - 1, 2 * match);
              }
              break;              

            default :
              break;
          }

          CTT32App::ProgressBarStep();
        }
      }
    }

    gr.grOnlyThirdPlace = only3rdPlace;
    
    if (gr.grModus == MOD_PLO || gr.grModus == MOD_DKO || gr.grModus == MOD_MDK)
    {
      MtRec::MtEvent mtEvent;
      mtEvent.grID = gr.grID;
      mtEvent.mtRound = 1;
      mtEvent.mtChance = 0;
      
      for (int i = 0; i < gr.grSize / 2; i++)
      {
        mtEvent.mtMatch = i + 1;
        mt.SelectByEvent(mtEvent);
        mt.Next();
        mt.Close();
        gr.SetWinner(mt);
        gr.SetLoser(mt);
      }
    }

    connPtr->Commit();

    // Notify Views
    CRequest update;
    update.type = CRequest::INSERT;
    update.rec  = CRequest::GRREC;
    update.id   = gr.grID;

    CTT32App::NotifyChange(update);
  }

  } // Ende des Scopes

  if (!defaultConnection && delConnPtr)
    delete connPtr;

  return 1;
}


// Eine Gruppe mit allen Abhaengigkeiten aendern
unsigned  GrStore::UpdateGroup(void *arg)
{
  CpRec cp      = ((CreateGroupStruct *) arg)->cp;
  GrRec grTempl = ((CreateGroupStruct *) arg)->gr;
  int   start   = ((CreateGroupStruct *) arg)->start;
  int   count   = ((CreateGroupStruct *) arg)->count;
  wxString  nameTempl = ((CreateGroupStruct *) arg)->nameTempl;
  wxString  descTempl = ((CreateGroupStruct *) arg)->descTempl;
  bool  isAlpha   = ((CreateGroupStruct *) arg)->numeric == 2;
  bool  delConnPtr = ((CreateGroupStruct *) arg)->delConnPtr;

  Connection *connPtr = ((CreateGroupStruct *) arg)->connPtr;

  bool defaultConnection = true;
  
  if (!connPtr)
  {
    connPtr = TTDbse::instance()->GetDefaultConnection();
    delConnPtr = false;
  }
  else
    defaultConnection = false;

  wxASSERT(connPtr);

  delete ((CreateGroupStruct *) arg);

  connPtr->StartTransaction();

  // Scope der Variablen begrenzen. 
  // Sie muessen vor dem delete connPtr geloescht werden
  {
  GrStore  gr(connPtr);
  SyStore  sy(connPtr);
  MdStore  md(connPtr);

  if (cp.cpType == CP_TEAM)
  {
    if (!sy.SelectById(grTempl.syID) || !sy.Next())
    {
      connPtr->Rollback();
      if (!defaultConnection)
        delete connPtr;

      return 0;
    }
  }

  if (grTempl.grModus == MOD_RR)
  {
    if (!md.SelectById(grTempl.mdID) || !md.Next())
    {
      connPtr->Rollback();
      if (!defaultConnection)
        delete connPtr;
      return 0;
    }
  }
  
  int nameOffset = (isAlpha ? 'A' - 1 : 0);
  int descOffset = (isAlpha ? 'A' - 1 : 0);

  for (int nr = start; nr < (start + count); nr++)
  {
    gr = grTempl;

    wxSnprintf(gr.grName, sizeof(gr.grName), nameTempl.t_str(), nr + nameOffset);

    bool ret = gr.SelectByName(gr.grName, cp);
    ret &= gr.Next();
    gr.Close();
    
    gr.mdID = grTempl.mdID;
    gr.syID = grTempl.syID;
    gr.grWinner = grTempl.grWinner;
    gr.grBestOf = grTempl.grBestOf;
    wxSnprintf(gr.grDesc, sizeof(gr.grDesc), descTempl.t_str(), nr + descOffset);
    wxSnprintf(gr.grStage, sizeof(gr.grStage), grTempl.grStage);
    
    ret &= gr.Update();
    
    if (!ret)
    {
      connPtr->Rollback();
      if (!defaultConnection)
        delete connPtr;
      
      return 0;
    }    
    
    CTT32App::ProgressBarStep();

    connPtr->Commit();

    // Notify Views
    CRequest update;
    update.type = CRequest::UPDATE;
    update.rec  = CRequest::GRREC;
    update.id   = gr.grID;

    CTT32App::NotifyChange(update);
  }

  } // Ende des Scopes

  if (!defaultConnection && delConnPtr)
    delete connPtr;

  return 1;
}


// -----------------------------------------------------------------------
// Insert note
bool GrStore::InsertNote(const wxString &note)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE GrRec SET grNotes = ? WHERE grID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, note.IsEmpty() ? NULL : (wxChar *)note.wx_str());
    stmtPtr->SetData(2, &grID);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec = CRequest::GRREC;
  update.id = grID;

  CTT32App::NotifyChange(update);

  return true;
}


// Get Note
wxString GrStore::GetNote()
{
  wxString str = "SELECT grNotes FROM GrRec WHERE grID = " + ltostr(grID);
  wxString note;

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  wxChar *tmp = new wxChar[4096];
  memset(tmp, 0, 4096 * sizeof(wxChar));

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    bool ret = resPtr->Next() && resPtr->GetData(1, tmp, 4095);
    if (!ret || resPtr->WasNull(1))
      *tmp = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete resPtr;
  delete stmtPtr;

  note = tmp;
  delete[] tmp;

  return note;
}


// Get sort order
short GrStore::CalculateSortOrder()
{
  // Defaults
  if (cpID == 0 || *grStage == 0)
    return 0;

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  // Gibt es bereits eine fuer diese Stufe?
  short res = 0;

  wxString str;

  try
  {
    str = "SELECT grSortOrder FROM GrRec WHERE cpID = " + ltostr(cpID) + " AND grStage = '" + grStage + "'";

    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);

    if (!resPtr->Next() || !resPtr->GetData(1, res) || resPtr->WasNull(1))
      res = 0;

    delete resPtr;
    resPtr = 0;

    // Wenn die Stufe noch nicht existiert, dann Stufen zaehlen
    if (res == 0)
    {
      str = "SELECT 1 + COUNT(DISTINCT grStage) FROM GrRec WHERE cpID = " + ltostr(cpID) + " AND grStage < '" + grStage + "'";
      resPtr = stmtPtr->ExecuteQuery(str);

      if (!resPtr->Next() || !resPtr->GetData(1, res) || resPtr->WasNull(1))
        res = 0;

      delete resPtr;
      resPtr = 0;
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;
  delete resPtr;

  return res ? res : 1;
}


// Update grPrinted
bool GrStore::SetPrinted(timestamp &ts)
{
  wxString sql = "UPDATE GrRec SET grPrinted = ? WHERE grID = ? ";

  PreparedStatement *stmtPtr = NULL;

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(sql);
    stmtPtr->SetData(1, &ts);
    stmtPtr->SetData(2, &grID);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec = CRequest::GRREC;
  update.id = grID;

  CTT32App::NotifyChange(update);

  return true;
}

// -----------------------------------------------------------------------
// Import / Export
// Siehe PlStore zur Verwendung von std::ifstream
bool  GrStore::Import(const wxString &name)
{
  long version = 1;
  wxTextFile ifs(name);
  if (!ifs.Open())
    return false;

  wxString line = ifs.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#GROUPS", version))
  {
    ifs.Close();
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#GROUPS"), line.c_str()))
      return false;
  }

  if (version > 1)
  {
    infoSystem.Error(_("Version %d of import file is not supported"), version);
    return false;
  }

  // Read all CP, SY, and MD: there are not so many
  std::map<wxString, long> cpMap;
  std::map<wxString, long> syMap;
  std::map<wxString, long> mdMap;

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  // HACK: Die Variablen duerfen nicht laenger leben als connPtr [
  {
  CpStore cp(connPtr);
  SyStore sy(connPtr);
  MdStore md(connPtr);

  cp.SelectAll();
  while (cp.Next())
    cpMap[cp.cpName] = cp.cpID;

  cpMap[wxEmptyString] = 0;

  sy.SelectAll();
  while (sy.Next())
    syMap[sy.syName] = sy.syID;

  syMap[wxEmptyString] = 0;

  md.SelectAll();
  while (md.Next())
    mdMap[md.mdName] = md.mdID;
  
  mdMap[wxEmptyString];

  for (; !ifs.Eof(); line = ifs.GetNextLine())
  {
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    wxStringTokenizerEx tokens(line, wxT(",;\t"));

    wxString strCpName = tokens.GetNextToken();
    wxString strGrName = tokens.GetNextToken();
    wxString strGrDesc = tokens.GetNextToken();
    wxString strGrStage = tokens.GetNextToken();
    wxString strGrModus = tokens.GetNextToken();
    wxString strGrSize = tokens.GetNextToken();
    wxString strGrBestOf = tokens.GetNextToken();
    wxString strGrWinner = tokens.GetNextToken();
    wxString strMdName = tokens.GetNextToken();
    wxString strSyName = tokens.GetNextToken();
    wxString strGrNofRounds = tokens.GetNextToken();
    wxString strGrNofMatches = tokens.GetNextToken();
    wxString strGrThirdPlace = tokens.GetNextToken(); // Empy or Yes / No / Only

    if (strCpName.IsEmpty() || cpMap.find(strCpName) == cpMap.end())
      continue;
    if (strGrName.IsEmpty())
      continue;
    if (strGrDesc.IsEmpty())
      continue;
    if (strGrStage.IsEmpty())
      continue;
    if (strGrBestOf.IsEmpty())
      continue;
    if (strGrWinner.IsEmpty())
      continue;
    if (!strSyName.IsEmpty() && syMap.find(strSyName) == syMap.end())
    {
      infoSystem.Error(_("Unkown team system %s"), _strtos(strSyName));
      continue;
    }
    if (!strMdName.IsEmpty() && mdMap.find(strSyName) == mdMap.end())
    { 
      infoSystem.Error(_("Unkown group mdous %s"), _strtos(strMdName));
      continue;
    }

    // TODO: Don't change modus, system, size of existing groups

    GrStore gr(connPtr);

    wxStrncpy((wxChar *) gr.grName, strGrName.t_str(), sizeof(grName) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) gr.grDesc, strGrDesc.t_str(), sizeof(grDesc) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) gr.grStage, strGrStage.t_str(), sizeof(grStage) / sizeof(wxChar) -1);

    gr.cpID = cpMap[strCpName];
    gr.syID = syMap[strSyName];
    gr.mdID = mdMap[strMdName];

    if (strGrModus == "RR")
      gr.grModus = MOD_RR;
    else if (strGrModus == "KO" || strGrModus == "SKO")
      gr.grModus = MOD_SKO;
    else if (strGrModus == "PO" || strGrModus == "PLO")
      gr.grModus = MOD_PLO;
    else if (strGrModus == "DK" || strGrModus == "DKO")
      gr.grModus = MOD_DKO;
    else if (strGrModus == "MK" || strGrModus == "MDK")
      gr.grModus = MOD_MDK;
    else
    {
      infoSystem.Error(_("Unkown group type %s"), _strtos(strGrModus));
      continue;
    }

    gr.grSize = _strtos(strGrSize);
    gr.grBestOf = _strtos(strGrBestOf);
    gr.grWinner = _strtos(strGrWinner);

    if (!strGrNofRounds.IsEmpty())
      gr.grNofRounds = _strtos(strGrNofRounds);
    if (!strGrNofMatches.IsEmpty())
      gr.grNofMatches = _strtos(strGrNofMatches);

    switch (*strGrThirdPlace.t_str())
    {
      case '\0' :
      case 'Y'  :
        break;
      case 'N' : 
        gr.grNoThirdPlace = 1;
        break;
      case 'O' :
        gr.grOnlyThirdPlace = 1;
        break;

      default :
        continue;
    }

    connPtr->StartTransaction();

    if (gr.InsertOrUpdate())
      connPtr->Commit();
    else
      connPtr->Rollback();
  }
  }  // HACK ]

  delete connPtr;

  ifs.Close();
  
  return true;
}


bool  GrStore::Export(wxTextBuffer &os, short cpType, const std::vector<long> & idList, bool append)
{
  long version = 1;

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  
  if (!append)
  {
    os.AddLine(wxString::Format("#GROUPS %d", version));
    os.AddLine("# CP; Name; Description; Stage; Modus; Size; Best of; Winner; Group Modus; Team System; Nof Rounds; Nof Matches; 3d Place; ");
  }

  // Read all CP, SY, and MD: there are not so many
  std::map<long, wxString> cpMap;
  std::map<long, wxString> syMap;
  std::map<long, wxString> mdMap;

  CpStore cp(connPtr);
  SyStore sy(connPtr);
  MdStore md(connPtr);

  cp.SelectAll();
  while (cp.Next())
    cpMap[cp.cpID] = cp.cpName;

  cpMap[0] = wxEmptyString;

  sy.SelectAll();
  while (sy.Next())
    syMap[sy.syID] = sy.syName;

  syMap[0] = wxEmptyString;

  md.SelectAll();
  while (md.Next())
    mdMap[md.mdID] = md.mdName;
  
  mdMap[0] = wxEmptyString;

  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {
    static wxString modus[] = {"", "RR", "KO", "DK", "PO", "MK"};

    wxString line;
    long grID = (*it);

    GrStore  gr;
    if (!gr.SelectById(grID) || !gr.Next())
      continue;
    gr.Close();

    line << cpMap[gr.cpID] << ";"
         << gr.grName << ";"
         << gr.grDesc << ";"
         << gr.grStage << ";"
         << modus[gr.grModus] << ";"
         << gr.grSize << ";"
         << gr.grBestOf << ";"
         << gr.grWinner << ";"
         << mdMap[gr.mdID] << ";"
         << syMap[gr.syID] << ";"
         << gr.grNofRounds << ";"
         << gr.grNofMatches << ";"
    ;

    switch (gr.grModus)
    {
      case MOD_RR :
        break;
      case MOD_SKO :
        if (gr.grOnlyThirdPlace)
          line << "O;";
        else
          line << "N;";
        break;
      case MOD_PLO :
        if (gr.grNoThirdPlace)
          line << "N;";
        else
          line << "Y;";
        break;
      default :
        line << ";";
        break;
    }

    os.AddLine(line);
  }

  return true;
}


