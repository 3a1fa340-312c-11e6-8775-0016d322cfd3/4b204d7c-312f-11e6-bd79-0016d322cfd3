#ifndef CYGONCE_KERNEL_MFIXIMPL_INL
#define CYGONCE_KERNEL_MFIXIMPL_INL

//==========================================================================
//
//      mfiximpl.inl
//
//      Memory pool with fixed block class declarations
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-03-23
// Purpose:     Define Mfiximpl class interface
// Description: Inline class for constructing a fixed block allocator
// Usage:       #include <cyg/kernel/mfiximpl.hxx>
//              #include <cyg/kernel/mfiximpl.inl>      
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_arch.h>          // HAL_LSBIT_INDEX magic asm code


Cyg_Mempool_Fixed_Implementation::Cyg_Mempool_Fixed_Implementation(
        cyg_uint8 *base,
        cyg_int32 size,
        CYG_ADDRWORD alloc_unit )
{
    cyg_int32 i;
    bitmap = (cyg_uint32 *)base;
    blocksize = alloc_unit;

    CYG_ASSERT( blocksize > 0, "Bad blocksize" );
    CYG_ASSERT( size > 2, "Bad blocksize" );
    CYG_ASSERT( blocksize < size, "blocksize, size bad" );

    numblocks = size / blocksize;
    top = base + size;

    CYG_ASSERT( numblocks >= 2, "numblocks bad" );

    i = (numblocks + 31)/32;        // number of words to map blocks
    while ( (i * 4 + numblocks * blocksize) > size ) {
        numblocks --;               // steal one block for admin
        i = (numblocks + 31)/32;    // number of words to map blocks
    }

    CYG_ASSERT( 0 < i, "Bad word count for bitmap after fitment" );
    CYG_ASSERT( 0 < numblocks, "Bad block count after fitment" );

    maptop = i;
    // this should leave space for the bitmap and maintain alignment
    mempool = top - (numblocks * blocksize);
    CYG_ASSERT( base < mempool && mempool < top, "mempool escaped" );
    CYG_ASSERT( (cyg_uint8 *)(&bitmap[ maptop ]) <= mempool,
                "mempool overwrites bitmap" );
    CYG_ASSERT( &mempool[ numblocks * blocksize ] <= top,
                "mempool overflows top" );
    freeblocks = numblocks;
    firstfree = 0;

    // clear out the bitmap; no blocks allocated yet
    for ( i = 0; i < maptop; i++ )
        bitmap[ i ] = 0;
    // apart from the non-existent ones at the top
    for ( i = ((numblocks-1)&31) + 1; i < 32; i++ )
        bitmap[ maptop - 1 ] |= ( 1 << i );
}

Cyg_Mempool_Fixed_Implementation::~Cyg_Mempool_Fixed_Implementation()
{
}

inline cyg_uint8 *
Cyg_Mempool_Fixed_Implementation::alloc( cyg_int32 size )
{
    // size parameter is not used
    CYG_UNUSED_PARAM( cyg_int32, size );
    if ( 0 >= freeblocks )
        return NULL;
    cyg_int32 i = firstfree;
    cyg_uint8 *p = NULL;
    do {
        if ( 0xffffffff != bitmap[ i ] ) {
            // then there is a free block in this bucket
            register cyg_uint32 j, k;
            k = ~bitmap[ i ];       // look for a 1 in complement
            HAL_LSBIT_INDEX( j, k );
            CYG_ASSERT( 0 <= j && j <= 31, "Bad bit index" );
            CYG_ASSERT( 0 == (bitmap[ i ] & (1 << j)), "Found bit not clear" );
            bitmap[ i ] |= (1 << j); // set it allocated
            firstfree = i;
            freeblocks--;
            CYG_ASSERT( freeblocks >= 0, "allocated too many" );
            p = &mempool[ ((32 * i) + j) * blocksize ];
            break;
        }
        if ( ++i >= maptop )
            i = 0;                  // wrap if at top
    } while ( i != firstfree );     // prevent hang if internal error
    CYG_ASSERT( NULL != p, "Should have a block here" );
    CYG_ASSERT( mempool <= p  && p <= top, "alloc mem escaped" );
    return p;
}
    
inline cyg_bool
Cyg_Mempool_Fixed_Implementation::free( cyg_uint8 *p, cyg_int32 size )
{
    // size parameter is not used
    CYG_UNUSED_PARAM( cyg_int32, size );
    if ( p < mempool || p >= top )
        return false;               // address way out of bounds
    cyg_int32 i = p - mempool;
    i = i / blocksize;
    if ( &mempool[ i * blocksize ] != p )
        return false;               // address not aligned
    cyg_int32 j = i / 32;
    CYG_ASSERT( 0 <= j && j < maptop, "map index escaped" );
    i = i - 32 * j;
    CYG_ASSERT( 0 <= i && i < 32, "map bit index escaped" );
    if ( ! ((1 << i) & bitmap[ j ] ) )
        return false;               // block was not allocated
    bitmap[ j ] &=~(1 << i);        // clear the bit
    freeblocks++;                   // count the block
    CYG_ASSERT( freeblocks <= numblocks, "freeblocks overflow" );
    return true;
}

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_MFIXIMPL_INL
// EOF mfiximpl.inl
