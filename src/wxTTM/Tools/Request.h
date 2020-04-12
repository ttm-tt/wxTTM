/* Copyright (C) 2020 Christoph Theis */

// Schnittstelle zwischen Datenbank und Views

# ifndef  REQUEST_H
# define  REQUEST_H

class CRequest
{
  public:
	  // Construction....
	  CRequest();
    CRequest(const CRequest &req);
	 ~CRequest();

    // Datenbereich
    enum  Type
    {
      NOTYPE = 0,
      INSERT = 1,
      UPDATE = 2,
      REMOVE = 3,
      UPDATE_RESULT   = 4,
      UPDATE_SCHEDULE = 5,
      UPDATE_REVERSE  = 6,
      UPDATE_NOMINATION = 7
    } type;

    enum Record
    {
      NOREC =  0,
      CPREC =  1,
      GRREC =  2,
      IDREC =  3,
      LTREC =  4,
      MDREC =  5,
      MTREC =  6,
      NAREC =  7,
      NMREC =  8,
      NTREC =  9,
      PLREC = 10,
      PSREC = 11,
      RKREC = 12,
      STREC = 13,
      SYREC = 14,
      TMREC = 15,
      UPREC = 16
    } rec;

    long    id;

    HANDLE  tid;  // Thread-ID
};


# endif