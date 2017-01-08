/**
 * The MIT License (MIT)
 *
 * Copyright (c) <2014> <Kartik Aiyer>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <embUnit.h>
#include <Pool.h>


#define POOL_TEST_UNIT_SIZE( dataType )     ( sizeof( dataType ) )

typedef struct _PoolTest1DataUnit
{
  uint32_t val;
}PoolTest1DataUnit;

#define POOL_TEST1_STORE_COUNT              ( 65 )

typedef struct _PoolTestBasicData
{
  uint8_t poolStore[ POOL_STORE_SIZE( POOL_TEST1_STORE_COUNT, POOL_TEST_UNIT_SIZE( PoolTest1DataUnit ) ) ];
  bool poolCreated;
  MemPool pool;
}PoolTestBasicData;

static PoolTestBasicData s_poolTestBasicData;
static void setUp()
{
  s_poolTestBasicData.poolCreated = PoolCreate( &s_poolTestBasicData.pool,
                                                s_poolTestBasicData.poolStore,
                                                sizeof( s_poolTestBasicData.poolStore ),
                                                POOL_TEST1_STORE_COUNT );
}

static void tearDown()
{
  PoolRelease( &s_poolTestBasicData.pool );
}

static void PoolCanBeCreated()
{
  TEST_ASSERT_EQUAL_INT( true, s_poolTestBasicData.poolCreated );
}

static void PoolCanAllocateOne()
{
  void* pBuf = PoolAlloc( &s_poolTestBasicData.pool );
  TEST_ASSERT_NOT_NULL( pBuf );
  TEST_ASSERT( ( pBuf >= (void*)s_poolTestBasicData.poolStore &&
                 pBuf < (void*)s_poolTestBasicData.poolStore + sizeof( s_poolTestBasicData.poolStore ) ) );
  PoolFree( &s_poolTestBasicData.pool, pBuf );
}

static void PoolAllocateAll()
{
  for( uint32_t i = 0; i < POOL_TEST1_STORE_COUNT; i++ )
  {
    void* pBuf = PoolAlloc( &s_poolTestBasicData.pool );
    TEST_ASSERT_NOT_NULL( pBuf );
    TEST_ASSERT( pBuf == ( s_poolTestBasicData.poolStore + i * sizeof( PoolTest1DataUnit ) ) );
  }
}

static void PoolAllocateAfterReleasingFullPool()
{
  void* pBuf[ POOL_TEST1_STORE_COUNT ] = { 0 };
  for( uint32_t i = 0; i < POOL_TEST1_STORE_COUNT; i++ ) {
    pBuf[ i ] = PoolAlloc( &s_poolTestBasicData.pool );
  }
  void *pOneMore = PoolAlloc( &s_poolTestBasicData.pool );
  TEST_ASSERT_NULL( pOneMore );
  //Free a Buffer
  PoolFree( &s_poolTestBasicData.pool, pBuf[ 0 ] );
  pBuf[ 0 ] = 0;
  TEST_ASSERT_NULL( pBuf[ 0 ] );
  pBuf[ 0 ] = PoolAlloc( &s_poolTestBasicData.pool );
  TEST_ASSERT_NOT_NULL( pBuf[ 0 ] );
  TEST_ASSERT( pBuf[ 0 ] == s_poolTestBasicData.poolStore );
}

static void PoolShouldCompletlyFreeUp()
{
  void* pBuf[ POOL_TEST1_STORE_COUNT ] = { 0 };
  for( uint32_t i = 0; i < POOL_TEST1_STORE_COUNT; i++ )
    pBuf[ i ] = PoolAlloc( &s_poolTestBasicData.pool );
  TEST_ASSERT_NULL( PoolAlloc( &s_poolTestBasicData.pool ) );
  for( uint32_t i = 0; i < POOL_TEST1_STORE_COUNT; i++ ) {
    PoolFree( &s_poolTestBasicData.pool, pBuf[i] );
    pBuf[ i ] = 0;
  }
  PoolAllocateAll();
}

TestRef PoolTest_ApiTests(void)
{
  EMB_UNIT_TESTFIXTURES(fixtures) {
    new_TestFixture( "PoolCanBeCreated", PoolCanBeCreated),
    new_TestFixture( "PoolCanAllocateOne", PoolCanAllocateOne ),
    new_TestFixture( "PoolAllocateAll", PoolAllocateAll ),
    new_TestFixture( "PoolAllocateAfterReleasingFullPool", PoolAllocateAfterReleasingFullPool ),
    new_TestFixture( "PoolShouldCompletlyFreeUp", PoolShouldCompletlyFreeUp )
  };
  EMB_UNIT_TESTCALLER( PoolBasicApiTest, "PoolBasicApiTest", setUp, tearDown, fixtures );
  return (TestRef)&PoolBasicApiTest;
}
