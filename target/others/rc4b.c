////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  WjCryptLib_RC4
//
//  An implementation of RC4 stream cipher
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "rc4b.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SwapBytes( Value1, Value2 )                 \
{                                                   \
    uint8_t temp = Value1;                          \
    Value1 = Value2;                                \
    Value2 = temp;                                  \
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4Initialise
//
//  Initialises an RC4 cipher and discards the specified number of first bytes.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Rc4Initialise
    (
        Rc4Context*     Context,        // [out]
        void const*     Key,            // [in]
        uint32_t        KeySize,        // [in]
        uint32_t        DropN           // [in]
    )
{
    uint32_t        i;
    uint32_t        j;
    uint32_t        n;

    // Setup key schedule
    for( i=0; i<256; i++ )
    {
        Context->S[i] = (uint8_t)i;
    }

    j = 0;
    for( i=0; i<256; i++ )
    {
        j = ( j + Context->S[i] + ((uint8_t*)Key)[i % KeySize] ) % 256;
        SwapBytes( Context->S[i], Context->S[j] );
    }

    i = 0;
    j = 0;

    // Drop first bytes (if requested)
    for( n=0; n<DropN; n++ )
    {
        i = ( i + 1 ) % 256;
        j = ( j + Context->S[i] ) % 256;
        SwapBytes( Context->S[i], Context->S[j] );
    }

    Context->i = i;
    Context->j = j;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4Output
//
//  Outputs the requested number of bytes from the RC4 stream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Rc4Output
    (
        Rc4Context*     Context,        // [in out]
        void*           Buffer,         // [out]
        uint32_t        Size            // [in]
    )
{
    uint32_t    n;

    for( n=0; n<Size; n++ )
    {
        Context->i = ( Context->i + 1 ) % 256;
        Context->j = ( Context->j + Context->S[Context->i] ) % 256;
        SwapBytes( Context->S[Context->i], Context->S[Context->j] );

        ((uint8_t*)Buffer)[n] = Context->S[ (Context->S[Context->i] + Context->S[Context->j]) % 256 ];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4Xor
//
//  XORs the RC4 stream with an input buffer and puts the results in an output buffer. This is used for encrypting
//  and decrypting data. InBuffer and OutBuffer can point to the same location for inplace encrypting/decrypting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Rc4Xor
    (
        Rc4Context*     Context,        // [in out]
        void const*     InBuffer,       // [in]
        void*           OutBuffer,      // [out]
        uint32_t        Size            // [in]
    )
{
    uint32_t    n;

    for( n=0; n<Size; n++ )
    {
        Context->i = ( Context->i + 1 ) % 256;
        Context->j = ( Context->j + Context->S[Context->i] ) % 256;
        SwapBytes( Context->S[Context->i], Context->S[Context->j] );

        ((uint8_t*)OutBuffer)[n] = ((uint8_t*)InBuffer)[n]
            ^ ( Context->S[ (Context->S[Context->i] + Context->S[Context->j]) % 256 ] );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4XorWithKey
//
//  This function combines Rc4Initialise and Rc4Xor. This is suitable when encrypting/decrypting data in one go with a
//  key that is not going to be reused.
//  InBuffer and OutBuffer can point to the same location for inplace encrypting/decrypting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Rc4XorWithKey
    (
        uint8_t const*      Key,                    // [in]
        uint32_t            KeySize,                // [in]
        uint32_t            DropN,                  // [in]
        void const*         InBuffer,               // [in]
        void*               OutBuffer,              // [out]
        uint32_t            BufferSize              // [in]
    )
{
    Rc4Context      context;

    Rc4Initialise( &context, Key, KeySize, DropN );
    Rc4Xor( &context, InBuffer, OutBuffer, BufferSize );
}


bool
    TestRc4
    (
        void
    )
{
    struct
    {
        char        Key [100];
        uint32_t    Drop;
        uint8_t     Output [16];
    } TestVectors [] =
        {
            { "Key",    0,   {0xeb,0x9f,0x77,0x81,0xb7,0x34,0xca,0x72,0xa7,0x19,0x4a,0x28,0x67,0xb6,0x42,0x95} },
            /*
            { "Wiki",   0,   {0x60,0x44,0xdb,0x6d,0x41,0xb7,0xe8,0xe7,0xa4,0xd6,0xf9,0xfb,0xd4,0x42,0x83,0x54} },
            { "Secret", 0,   {0x04,0xd4,0x6b,0x05,0x3c,0xa8,0x7b,0x59,0x41,0x72,0x30,0x2a,0xec,0x9b,0xb9,0x92} },
            { "Key",    1,   {0x9f,0x77,0x81,0xb7,0x34,0xca,0x72,0xa7,0x19,0x4a,0x28,0x67,0xb6,0x42,0x95,0x0d} },
            { "Key",    256, {0x92,0xfd,0xd9,0xb6,0xe4,0x04,0xef,0x4f,0xa0,0x75,0xf1,0xa3,0x44,0xed,0x81,0x6b} },
            */
        };

    Rc4Context      context;
    uint8_t         output [16];
    uint32_t        i;
    bool            success = true;

    for( i=0; i<(sizeof(TestVectors)/sizeof(TestVectors[0])); i++ )
    {
        Rc4Initialise( &context, TestVectors[i].Key, (uint8_t)strlen(TestVectors[i].Key), TestVectors[i].Drop );
        Rc4Output( &context, output, sizeof(output) );
        if( memcmp( output, TestVectors[i].Output, sizeof(output) ) != 0 )
        {
            printf( "TestRc4 - Failed test vector: %u\n", i );
            success = false;
        }
    }

    // Test by doing drop manually
    for( i=0; i<(sizeof(TestVectors)/sizeof(TestVectors[0])); i++ )
    {
        uint32_t x;

        Rc4Initialise( &context, TestVectors[i].Key, (uint8_t)strlen(TestVectors[i].Key), 0 );
        for( x=0; x<TestVectors[i].Drop; x++ )
        {
            Rc4Output( &context, output, 1 );
        }
        Rc4Output( &context, output, sizeof(output) );
        if( memcmp( output, TestVectors[i].Output, sizeof(output) ) != 0 )
        {
            printf( "TestRc4 - Failed test vector: %u [manual drop]\n", i );
            success = false;
        }
    }

    return success;
}

void main()
{
    bool res = false;

    res = TestRc4();
#if DEBUG
    if(res) {
        printf("TestRc4 passed\n");
    }
    else {
        printf("TestRc4 failed\n");
    }
#endif


}
