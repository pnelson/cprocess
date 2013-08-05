#ifndef _PROCESS_H
#define _PROCESS_H

#include <Windows.h>

#include "StringExA.h"

class CProcess
{
public:
  CProcess();
  virtual ~CProcess();

  BOOL Start(const CStringEx szFileName, const CStringEx szArguments);
  BOOL Start(const CStringEx szFileName, const CStringEx szArguments, const STARTUPINFO& pStartupInfo);

  BOOL Close() const;
  BOOL Kill() const;

  HWND GetWindowHandle() const;

  BOOL WaitForInputIdle(DWORD dwMilliseconds = INFINITE) const;
  BOOL WaitForExit(DWORD dwMilliseconds = INFINITE) const;

  BOOL isResponding() const;
  BOOL isTerminated() const;

protected:
  static BOOL CALLBACK ThreadProc(LPVOID lpParam);
  static BOOL CALLBACK GetWindowHandle(HWND hWnd, LPARAM lParam);

private:
  HANDLE m_hThread;

  STARTUPINFO m_StartupInfo;
  PROCESS_INFORMATION m_ProcessInformation;

  HWND m_hWnd;
};

#endif
