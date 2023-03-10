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

#include <system.h>
#include "err_desc.h"


/*----------------------------------------------------------------------
       Return string describing the error

   Args: errnumber -- The system error number (errno)

 Result:  long string describing the error is returned
  ----*/
char *
error_description(int errnumber)
{
    static char buffer[50+1];

    buffer[0] = '\0';

    if(errnumber >= 0)
      snprintf(buffer, sizeof(buffer), "%s", strerror(errnumber));
    else
      snprintf(buffer, sizeof(buffer), "Unknown error #%d", errnumber);

    return ( (char *) buffer);
}


