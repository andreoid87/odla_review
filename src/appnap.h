#ifndef APPNAP_H
#define APPNAP_H

#if !defined(__cplusplus)
#define C_API extern
#else
#define C_API extern "C"
#endif

C_API void disableAppNap();
C_API void enableAppNap();

#endif // APPNAP_H
