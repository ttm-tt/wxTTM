/* Copyright (C) 2020 Christoph Theis */

#ifndef REPORTSTORE_H
#define REPORTSTORE_H

// Nur ein Container fuer die View, die in diversen Reports und Exports verwendet werden
class  ReportStore
{
  public:
    static bool  CreateView();
    static bool  RemoveView();
    
  private:
    static bool  CreateListK();
};

#endif