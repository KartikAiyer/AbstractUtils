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
#include<embUnit.h>
#include <ThreadInterface.h>
#include <ConsoleLog.h>

extern TestRef PoolTest_ApiTests();
extern TestRef KThreadTest_ApiTests();
extern TestRef PriorityWakeTest();
extern TestRef PriorityDonateChainTest();

#define TEST_RUNNER_STACK_SIZE          ( 1 << 14 )
static uint8_t s_testRunnerThreadStack[ TEST_RUNNER_STACK_SIZE ];
static KThread s_testRunnerThread;

static void TestRunner( void* arg )
{
  TestRunner_start();
  {
    TestRunner_runTest( PoolTest_ApiTests() );
    TestRunner_runTest( KThreadTest_ApiTests() );
    ConsoleLog( "ALL DONE\n" );
    //TestRunner_runTest( PriorityWakeTest() );
    //TestRunner_runTest( PriorityDonateChainTest() );
  }
  TestRunner_end();
  ConsoleLogFlush();
  for ( ; ; );
}
int main (int argc, const char* argv[])
{
  KThreadCreateParams param = {
    .pThreadName = "TestRunnerThread",
    .threadPriority = SEMANTIC_THREAD_PRIORITY_LOWEST,
    .threadArg = NULL,
    .pStack = s_testRunnerThreadStack,
    .stackSizeInBytes = TEST_RUNNER_STACK_SIZE,
    .fn = TestRunner
  };
  KThreadCreate( &s_testRunnerThread, &param );
  KThreadInit();
}

