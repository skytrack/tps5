#ifndef CDCOMISEARCH_H__
#define CDCOMISEARCH_H__
//------------------------------------------------------------------------------
// COPYRIGHT (C) 2010 by CDCOM. All rights reserved.
//------------------------------------------------------------------------------
// Code designed and written by Igor V. Pronyushkin.
//------------------------------------------------------------------------------
// $Id: isearch.h 4073 2010-12-27 08:59:47Z pronyushkin $
//------------------------------------------------------------------------------

/**
 * @file search interface
 */

#ifdef  __cplusplus
extern "C" {
#endif

char const * initSearch(char const * searchfilespath);

void * makeSearcher();
//char const * search(void * searcher, double lon,double lat,int radius);
char const * searchNear(void * searcher, double lon,double lat,int radius);
char const * searchPoi( void * searcher, double lon,double lat,int radius);


void   dropSearcher(void * searcher);

void freeSearch();

#ifdef  __cplusplus
} /* extern "C" */
#endif

#endif // end file guardian
