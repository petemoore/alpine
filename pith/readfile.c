/*
 * ========================================================================
 * Copyright 2013-2022 Eduardo Chappa
 * Copyright 2006 University of Washington
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * ========================================================================
 */

#include "../pith/headers.h"

#include "../pith/store.h"

#include "readfile.h"


/*----------------------------------------------------------------------
    Read whole file into memory

  Args: filename -- path name of file to read

  Result: Returns pointer to malloced memory with the contents of the file
          or NULL

This won't work very well if the file has NULLs in it.
 ----*/
char *
read_file(char *filename, int so_get_flags)
{
    STORE_S *in_file = NULL, *out_store = NULL;
    unsigned char c;
    char *return_text = NULL;

    if((in_file = so_get(FileStar, filename, so_get_flags | READ_ACCESS))){


	if(!(out_store = so_get(CharStar, NULL, EDIT_ACCESS))){
	    so_give(&in_file);
	    return NULL;
	}

	/*
	 * We're just using the READ_FROM_LOCALE flag to translate
	 * to UTF-8.
	 */
	while(so_readc(&c, in_file))
	  so_writec(c, out_store);

	if(in_file)
	  so_give(&in_file);

	if(out_store){
	    return_text = (char *) so_text(out_store);
	    /* avoid freeing this */
	    if(out_store->txt)
	      out_store->txt = NULL;

	    so_give(&out_store);
	}
    }

    return(return_text);
}

/* our copy, to_file and from_file must be full paths. from_file
 * must exist.
 */
int
our_copy(char *to_file, char *from_file)
{
   STORE_S *in_cert, *out_cert;
   unsigned char c;

   in_cert  = so_get(FileStar, from_file, READ_ACCESS | READ_FROM_LOCALE);
   if (in_cert == NULL)
     return -1;

   out_cert = so_get(FileStar, to_file, WRITE_ACCESS | WRITE_TO_LOCALE);
   if (out_cert == NULL){
     so_give(&in_cert);
     return -1;
   }

   so_seek(out_cert, 0L, 0);
   so_truncate(out_cert, 0);

   while(so_readc(&c, in_cert) > 0)
     so_writec(c, out_cert);

   so_give(&in_cert);
   so_give(&out_cert);

   return 0;
}
