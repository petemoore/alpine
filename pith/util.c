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
#include "../pith/util.h"


/*
 * Internal prototypes
 */


/*
 * Allocate space for an int and copy val into it.
 */
int *
cpyint(int val)
{
    int *ip;

    ip = (int *)fs_get(sizeof(int));

    *ip = val;

    return(ip);
}
