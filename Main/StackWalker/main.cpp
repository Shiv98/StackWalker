/**********************************************************************
 *
 * main.cpp
 *
 *
 * History:
 *  2008-11-27   v1    - Header added
 *                       Samples for Exception-Crashes added...
 *  2009-11-01   v2    - Moved to stackwalker.codeplex.com
 *
 **********************************************************************/

#include "GraphCreator.h"
#include "stackwalker.h"
#include <DbgHelp.h>
#include <chrono>
#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <thread>
#include <tlhelp32.h>
#include <windows.h>

#define UNHANDLED_EXCEPTION_TEST
#define EXCEPTION_FILTER_TEST

#define THREADCOUNT 5
HANDLE ghThreads[THREADCOUNT];
HANDLE profThread;
std::map<std::string, std::vector<std::vector<std::string>>>
                           gCallTrees; // <threadName, callStackList>
std::map<std::string, int> gFunctionCounts;

//  Forward declarations:
HANDLE ListProcessThreads(DWORD dwOwnerPID);
void   printError(TCHAR* msg);
HANDLE GetMainThreadId(DWORD PID);
void   Func1();
void StackWalkTest(HANDLE threadHandle, std::string threadName);

// secure-CRT_functions are only available starting with VC8
#if _MSC_VER < 1400
#define _tcscpy_s _tcscpy
#define _tcscat_s _tcscat
#define _stprintf_s _stprintf
#define strcpy_s(a, b, c) strcpy(a, c)
#endif

int iGlobalInt = 0;
void (*pGlobalFuncPtr)() = 0;

// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole : public StackWalker
{
protected:
  virtual void OnOutput(LPCSTR szText) { printf("%s", szText); }
};

// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole_DifferentProcess : public StackWalker
{
public:
  StackWalkerToConsole_DifferentProcess(DWORD dwProcessId, HANDLE hProcess)
      : StackWalker(dwProcessId, hProcess)
  {
  }
  StackWalkerToConsole_DifferentProcess() //DWORD dwProcessId, HANDLE hProcess)
      : StackWalker()                     //dwProcessId, hProcess)
  {
  }

protected:
  virtual void OnOutput(LPCSTR szText) { printf("%s", szText); }
};

DWORD WINAPI ThreadProc(LPVOID);
DWORD WINAPI StartProfile(LPVOID);

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
  UNREFERENCED_PARAMETER(lpParam);

  Func1();
  return 0;
}

DWORD WINAPI StartProfile(LPVOID lpParam)
{
  UNREFERENCED_PARAMETER(lpParam);

  HANDLE thread = GetCurrentThread();
  //DWORD tid =16348;
  //DWORD  pid = 16520;
  //HANDLE tHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
  

  for (size_t j = 0; j < 2; j++)
  {
    for (int i = 0; i < THREADCOUNT; i++)
    {
      /*char* intStr = (char*)malloc(2);
      intStr = itoa(i, intStr, 0);*/
      //sprintf(threadName, "%d", i);
      //StackWalkTest(pid,tHandle, "TID");
      StackWalkTest(ghThreads[i], std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return 0;
}

void CreateMultipleThreads()
{
  DWORD dwThreadID;

  for (int i = 0; i < THREADCOUNT; i++)
  {
    // TODO: More complex scenarios may require use of a parameter
    //   to the thread procedure, such as an event per thread to
    //   be used for synchronization.
    ghThreads[i] = CreateThread(NULL,       // default security
                                0,          // default stack size
                                ThreadProc, // name of the thread function
                                NULL,       // no thread parameters
                                0,          // default startup flags
                                &dwThreadID);

    if (ghThreads[i] == NULL)
    {
      printf("CreateThread failed (%d)\n", GetLastError());
      return;
    }
  }
}

void CreateProfilerThread()
{
  DWORD dwThreadID;

  // TODO: More complex scenarios may require use of a parameter
  //   to the thread procedure, such as an event per thread to
  //   be used for synchronization.
  profThread = CreateThread(NULL,         // default security
                            0,            // default stack size
                            StartProfile, // name of the thread function
                            NULL,         // no thread parameters
                            0,            // default startup flags
                            &dwThreadID);

  if (profThread == NULL)
  {
    printf("CreateThread failed (%d)\n", GetLastError());
    return;
  }
}

void Func5(HANDLE threadHandle, std::string threadName)
{
  /*HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  StackWalker sw(pid, hProcess);
  SuspendThread(threadHandle);
  CONTEXT c;
  memset(&c, 0, sizeof(CONTEXT));
  c.ContextFlags = CONTEXT_FULL;
  


  // TODO: Detect if you want to get a thread context of a different process, which is running a different processor architecture...
  // This does only work if we are x64 and the target process is x64 or x86;
  // It cannot work, if this process is x64 and the target process is x64... this is not supported...
  // See also: http://www.howzatt.demon.co.uk/articles/DebuggingInWin64.html
  if (GetThreadContext(threadHandle, &c) != FALSE)
  {
    sw.gLogFile << "\"" << threadName << "\":[";
    sw.ShowCallstack(threadHandle, &c);
    sw.gLogFile << "]";
    // Build tree. Will have to make this thread-safe.
    // Maintain a set - sorted list for arranging based on sample count.
    // GetCurrentEC - for executing thread - from this, get DSP and VI name -- Make this root of this sub-tree.
    // Merge this sub-tree with the main tree.
    // Secondary storage for the info.

    if (gCallTrees.find(threadName) != gCallTrees.end())
    {
      auto value = gCallTrees[threadName];
      value.push_back(sw.callStackList);
      gCallTrees.erase(threadName);
      gCallTrees.insert({threadName, value});

      int currentCount = gFunctionCounts[threadName];
      gFunctionCounts.erase(threadName);
      gFunctionCounts.insert({threadName, currentCount + 1});
    }
    else
    {
      std::vector<std::vector<std::string>> newValue;
      newValue.push_back(sw.callStackList);
      gCallTrees.insert({threadName, newValue});

      gFunctionCounts.insert({threadName, 1});
    }
  }

  ResumeThread(threadHandle);*/
  StackWalkerToConsole sw;
  SuspendThread(threadHandle);
  CONTEXT c;
  memset(&c, 0, sizeof(CONTEXT));
  c.ContextFlags = CONTEXT_FULL;
  if (GetThreadContext(threadHandle, &c) != FALSE)
  {
    sw.gLogFile << "\"" << threadName << "\":[";
    sw.ShowCallstack(threadHandle, &c);
    sw.gLogFile << "]";

    if (gCallTrees.find(threadName) != gCallTrees.end())
    {
      auto value = gCallTrees[threadName];
      value.push_back(sw.callStackList);
      gCallTrees.erase(threadName);
      gCallTrees.insert({threadName, value});

      int currentCount = gFunctionCounts[threadName];
      gFunctionCounts.erase(threadName);
      gFunctionCounts.insert({threadName, currentCount + 1});
    }
    else
    {
      std::vector<std::vector<std::string>> newValue;
      newValue.push_back(sw.callStackList);
      gCallTrees.insert({threadName, newValue});

      gFunctionCounts.insert({threadName, 1});
    }
  }

  ResumeThread(threadHandle);
}

HANDLE GetMainThreadId(DWORD PID)
{
  //const std::tr1::shared_ptr<void> hThreadSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0),
  //                                                 CloseHandle);
  //if (hThreadSnapshot.get() == INVALID_HANDLE_VALUE)
  //{
  //  throw std::runtime_error("GetMainThreadId failed");
  //}
  //THREADENTRY32 tEntry;
  //tEntry.dwSize = sizeof(THREADENTRY32);
  //DWORD threadID = 0;
  ////PID = GetCurrentProcessId();
  //for (BOOL success = Thread32First(hThreadSnapshot.get(), &tEntry);
  //     !threadID && success && GetLastError() != ERROR_NO_MORE_FILES;
  //     success = Thread32Next(hThreadSnapshot.get(), &tEntry))
  //{
  //  if (tEntry.th32OwnerProcessID == PID)
  //  {
  //    threadID = tEntry.th32ThreadID;
  //  }
  //}
  return OpenThread(THREAD_ALL_ACCESS, FALSE, 21768);
}

int WaitForAllThreads()
{
  // The handle for each thread is signaled when the thread is
  // terminated.
  DWORD dwWaitResult = WaitForMultipleObjects(THREADCOUNT, // number of handles in array
                                              ghThreads,   // array of thread handles
                                              TRUE,        // wait until all are signaled
                                              INFINITE);

  switch (dwWaitResult)
  {
      // All thread objects were signaled
    case WAIT_OBJECT_0:
      printf("All threads ended, cleaning up for application exit...\n");
      printf("Returned %d\n", dwWaitResult);
      break;

      // An error occurred
    default:
      printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
      return 1;
  }
  return 0;
}

int WaitForProfilerThread()
{
  // The handle for each thread is signaled when the thread is
  // terminated.
  DWORD dwWaitResult = WaitForMultipleObjects(1,           // number of handles in array
                                              &profThread, // array of thread handles
                                              TRUE,        // wait until all are signaled
                                              INFINITE);

  switch (dwWaitResult)
  {
      // All thread objects were signaled
    case WAIT_OBJECT_0:
      printf("All threads ended, cleaning up for application exit...\n");
      printf("Returned %d\n", dwWaitResult);
      break;

      // An error occurred
    default:
      printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
      return 1;
  }
  return 0;
}

HANDLE ListProcessThreads(DWORD dwOwnerPID)
{
  HANDLE        hThreadSnap = INVALID_HANDLE_VALUE;
  THREADENTRY32 te32;

  // Take a snapshot of all running threads
  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap == INVALID_HANDLE_VALUE)
    return (INVALID_HANDLE_VALUE);

  // Fill in the size of the structure before using it.
  te32.dwSize = sizeof(THREADENTRY32);

  // Retrieve information about the first thread,
  // and exit if unsuccessful
  if (!Thread32First(hThreadSnap, &te32))
  {
    printError(TEXT("Thread32First")); // Show cause of failure
    CloseHandle(hThreadSnap);          // Must clean up the snapshot object!
    return (INVALID_HANDLE_VALUE);
  }

  // Now walk the thread list of the system,
  // and display information about each thread
  // associated with the specified process
  do
  {
    if (te32.th32OwnerProcessID == dwOwnerPID)
    {
      _tprintf(TEXT("\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
      _tprintf(TEXT("\n     base priority  = %d"), te32.tpBasePri);
      _tprintf(TEXT("\n     delta priority = %d"), te32.tpDeltaPri);
    }
  } while (Thread32Next(hThreadSnap, &te32));

  _tprintf(TEXT("\n"));

  //  Don't forget to clean up the snapshot object.
  //CloseHandle(hThreadSnap);
  return (hThreadSnap);
}

void printError(TCHAR* msg)
{
  DWORD  eNum;
  TCHAR  sysMsg[256];
  TCHAR* p;

  eNum = GetLastError();
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, eNum,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                sysMsg, 256, NULL);

  // Trim the end of the line and terminate it with a null
  p = sysMsg;
  while ((*p > 31) || (*p == 9))
    ++p;
  do
  {
    *p-- = 0;
  } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

  // Display the message
  _tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}

void Func6()
{
  std::cout << " Blah blah" << std::endl;
}

void Func4()
{
  Func6();
}

void Func3()
{
  for (size_t i = 0; i < 50; i++)
  {
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << i;
  }
  Func4();
}

void Func2()
{
  for (size_t i = 0; i < 50; i++)
  {
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Func3();
  }
}

void Func1()
{
  for (size_t i = 0; i < 50; i++)
  {
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Func2();
  }
}

void StackWalkTest(HANDLE threadHandle, std::string threadName)
{
  Func5(threadHandle, threadName);
}

void GlobalIntTest()
{
  StackWalkerToConsole sw;
  sw.ShowObject(&iGlobalInt);
}

void GlobalFunctionPointerTest()
{
  StackWalkerToConsole sw;
  sw.ShowObject(&pGlobalFuncPtr);
}

// Simple implementation of an additional output to the console:
class MyStackWalker : public StackWalker
{
public:
  MyStackWalker() : StackWalker() {}
  MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
  virtual void OnOutput(LPCSTR szText)
  {
    printf(szText);
    StackWalker::OnOutput(szText);
  }
};


int main(int argc, _TCHAR* argv[])
{
  //printf("\n\n\nShow a simple callstack of the current thread:\n\n\n");
  CreateMultipleThreads(); // Create these as a placeholder for LV threads.

  CreateProfilerThread();

  //WaitForAllThreads();
  WaitForProfilerThread();

  CreateGraphAndJSON(gCallTrees);
  return 0;
}
