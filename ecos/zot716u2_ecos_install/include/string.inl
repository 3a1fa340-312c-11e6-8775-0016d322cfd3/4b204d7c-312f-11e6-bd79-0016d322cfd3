#ifndef CYGONCE_LIBC_STRING_INL
#define CYGONCE_LIBC_STRING_INL
//===========================================================================
//
//      string.inl
//
//      Inline functions for ANSI standard string and memory area
//      manipulation routines described in ANSI para 7.11
//
//===========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-02-13
// Purpose:     
// Description: 
// Usage:       #include <string.h> - do not include this file directly
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

// Include the C library?
#ifdef CYGPKG_LIBC     

// INCLUDES

#include <string.h>         // header for this file just in case, although
                            // it should have already been included


CYGPRI_LIBC_INLINE int
strcoll( const char *s1, const char *s2 )
{
    return strcmp(s1, s2);
} // strcoll()


#endif // ifdef CYGPKG_LIBC     

#endif // CYGONCE_LIBC_STRING_INL multiple inclusion protection

// EOF string.inl
