/* Copyright (C) 2020 Christoph Theis */

// Basic includes
# ifndef  SQLINCLUDE_H
# define  SQLINCLUDE_H

# include  <windows.h>
# include  <sql.h>
# include  <sqlext.h>
# include  <sqltypes.h>
# include  <odbcss.h>

typedef TIMESTAMP_STRUCT  timestamp;

#ifdef UNICODE
# define SQL_TVARCHAR SQL_WVARCHAR
#else
# define SQL_TVARCHAR SQL_VARCHAR
#endif

// SQL_ERROR is only "-1", SQL_SUCCESS only 0, SQL_SUCCEEDED only SQL_SUCCESS, but we need to cover "-2" (SQL_INVALID_HANDLE) as well
// But not SQL_NO_DATA etc.
#define SQL_FAILED(ret) (ret == SQL_ERROR || ret == SQL_INVALID_HANDLE)

inline bool operator<(const timestamp &ts1, const timestamp &ts2)
{
  if (ts1.year < ts2.year)
    return true;
  if (ts1.year > ts2.year)
    return false;
    
  if (ts1.month < ts2.month)
    return true;
  if (ts1.month > ts2.month)
    return false;
    
  if (ts1.day < ts2.day)
    return true;
  if (ts1.day > ts2.day)
    return false;
    
  if (ts1.hour < ts2.hour)
    return true;
  if (ts1.hour > ts2.hour)
    return false;
    
  if (ts1.minute < ts2.minute)
    return true;
  if (ts1.minute > ts2.minute)
    return false;
    
  if (ts1.second < ts2.second)
    return true;
  if (ts1.second > ts2.second)
    return false;
    
  if (ts1.fraction < ts2.fraction)
    return true;
  if (ts1.fraction > ts2.fraction)
    return false;
    
  return false;
}


inline bool operator >=(const timestamp &ts1, const timestamp &ts2)
{
  return !operator<(ts1, ts2);
}



inline bool operator>(const timestamp &ts1, const timestamp &ts2)
{
  if (ts1.year > ts2.year)
    return true;
  if (ts1.year < ts2.year)
    return false;
    
  if (ts1.month > ts2.month)
    return true;
  if (ts1.month < ts2.month)
    return false;
    
  if (ts1.day > ts2.day)
    return true;
  if (ts1.day < ts2.day)
    return false;
    
  if (ts1.hour > ts2.hour)
    return true;
  if (ts1.hour < ts2.hour)
    return false;
    
  if (ts1.minute > ts2.minute)
    return true;
  if (ts1.minute < ts2.minute)
    return false;
    
  if (ts1.second > ts2.second)
    return true;
  if (ts1.second < ts2.second)
    return false;
    
  if (ts1.fraction > ts2.fraction)
    return true;
  if (ts1.fraction < ts2.fraction)
    return false;
    
  return false;
}


inline bool operator<=(const timestamp &ts1, const timestamp &ts2)
{
  return !operator>(ts1, ts2);
}


inline bool operator==(const timestamp &ts1, const timestamp &ts2)
{
  return ts1.year == ts2.year && 
         ts1.month == ts2.month &&
         ts1.day == ts2.day &&
         ts1.hour == ts2.hour &&
         ts1.minute == ts2.minute &&
         ts1.second == ts2.second &&
         ts1.fraction == ts2.fraction;
}

inline bool operator!=(const timestamp &ts1, const timestamp &ts2)
{
  return !operator==(ts1, ts2);
}


# endif