/* Copyright (C) Christoph Theis 2022 */
#pragma once

#include "StoreObj.h"

struct MpRec
{
  long    mpID;       // Unique ID
  wxChar  mpName[9];  // Short name
  wxChar  mpDesc[65]; // Description
  short   mpCount;

  struct MpItem
  {
    short mpResA = 0;
    short mpResX = 0;
    short mpPts = 0;
  } *mpList = nullptr;

  MpRec() { Init(); }
 ~MpRec() { delete[] mpList; }

  void Init() { delete[] mpList; memset(this, 0, sizeof(MpRec)); }

  void ChangeCount(int mt);

};


class MpStore : public StoreObj, public MpRec
{
public:
    // Create in database
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);
    
    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    MpStore(Connection * = 0);
   ~MpStore();

    virtual void Init();

  // Read, Create, Update, Remove
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name);
    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    bool  Next();

    // Check on mpName, if record exists
    bool  InsertOrUpdate();

};