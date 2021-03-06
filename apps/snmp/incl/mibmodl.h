/*
 * mib_module.h --
 *
 * This file contains the list of the system dependent MIB modules.
 *
 * Copyright (c) 1997
 *
 * Erik Schoenfelder		TU Braunschweig, Germany
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of CMU not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  
 * 
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 *
 * Adding your mib-module to the agent:
 *
 * - create a yourmib.c file containing your code.
 *   see mib_example.c for a skeleton.
 *
 * - add your module enclosing define to this header.
 *
 * - add your module initialization to mib_module.c.
 *
 * - add the module to apps/Makefile (or apps/Makefile.in and rerun
 *   configure) to include the module in compilation.
 */

#ifndef MIB_MODULE_H
#define MIB_MODULE_H

//
// The initialization function which calls the module specific
// initialization functions during agent startup.
//

void init_modules(void);

#endif  MIB_MODULE_H
