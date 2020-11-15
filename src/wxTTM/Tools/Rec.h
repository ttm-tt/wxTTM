/* Copyright (C) 2020 Christoph Theis */

// Globale definitionen
#ifndef  REC_H
#define  REC_H

#define  SEX_MALE    (1)
#define  SEX_FEMALE  (2)
#define  SEX_MIXED   (3)


#define  CP_OPEN     (0)
#define  CP_SINGLE   (1)
#define  CP_DOUBLE   (2)
#define  CP_MIXED    (3)
#define  CP_TEAM     (4)
#define  CP_INDIVIDUAL (8)
#define  CP_GROUP    (16)


#define  MOD_RR      (1)
#define  MOD_SKO     (2)
#define  MOD_DKO     (3)
#define  MOD_PLO     (4)
#define  MOD_MDK     (5)

// Turniertypen
enum
{
  TT_YOUTH = 1,
  TT_REGULAR = 2,
  TT_SCI = 3
};

enum
{
  TT_ITTF = 1,
  TT_DTTB = 2
};


#endif