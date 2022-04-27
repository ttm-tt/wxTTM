/* Copyright (C) 2022 Christoph Theis */
#pragma once

#include "StoreObj.h"
#include "MpStore.h"

struct MpListRec : public MpRec
{
  MpListRec() { Init(); }

  void Init() { memset(this, 0, sizeof(MpListRec)); }
};


class MpListStore : public StoreObj, public MpListRec
{
  public:
    static bool CreateView();
    static bool RemoveView();

  public:
    MpListStore(Connection *connPtr = nullptr);
   ~MpListStore();

    virtual void Init();

  public:
    bool SelectAll();
    bool SelectById(long id);

  private:
    wxString SelectString() const;
    bool BindRec();
};


