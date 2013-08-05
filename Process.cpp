#include "Process.h"

CProcess::CProcess()
{
  m_hThread = NULL;

  ::ZeroMemory(&m_StartupInfo, sizeof(SHELLEXECUTEINFO));
  ::ZeroMemory(&m_ProcessInformation, sizeof(PROCESS_INFORMATION));

  m_StartupInfo.cb = sizeof(SHELLEXECUTEINFO);

  m_hWnd = NULL;
}

CProcess::~CProcess()
{
  ::CloseHandle(m_ProcessInformation.hProcess);
  ::CloseHandle(m_ProcessInformation.hThread);
}

BOOL CProcess::Start(const CStringEx szFileName, const CStringEx szArguments)
{
  CStringEx szPath = szFileName.Left(szFileName.ReverseFind('\\'));
  CStringEx szFileNameArguments = szFileName + " " + szArguments;

  if (!::CreateProcess(szFileName, szPath, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, szPath, &m_StartupInfo, &m_ProcessInformation))
    return FALSE;

  HANDLE hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CProcess::ThreadProc, this, NULL, NULL);

  ::CloseHandle(m_hThread);

  return TRUE;
}

BOOL CProcess::Start(const CStringEx szFileName, const CStringEx szArguments, const STARTUPINFO& pStartupInfo)
{
  ::memcpy_s(&m_StartupInfo, sizeof(STARTUPINFO), &pStartupInfo, sizeof(STARTUPINFO));

  return Start(szFileName, szArguments);
}

BOOL CProcess::Close() const
{
  return ::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
}

BOOL CProcess::Kill() const
{
  return ::TerminateProcess(m_ProcessInformation.hProcess, EXIT_FAILURE);
}

HWND CProcess::GetWindowHandle() const
{
  return m_hWnd;
}

BOOL CProcess::WaitForInputIdle(DWORD dwMilliseconds) const
{
  return ::WaitForInputIdle(m_ProcessInformation.hProcess, dwMilliseconds) != NULL;
}

BOOL CProcess::WaitForExit(DWORD dwMilliseconds) const
{
  return ::WaitForSingleObject(m_ProcessInformation.hProcess, dwMilliseconds) == WAIT_OBJECT_0;
}

// MSDN documentation sucked for this, but http://support.microsoft.com/kb/q231844/ was good.
BOOL CProcess::isResponding() const
{
  return ::SendMessageTimeout(m_hWnd, WM_NULL, NULL, NULL, (SMTO_ABORTIFHUNG | SMTO_BLOCK), 1000, NULL);
}

// needs PROCESS_QUERY_INFORMATION or PROCESS_QUERY_LIMITED_INFORMATION access rights
// http://msdn.microsoft.com/en-us/library/ms684880(VS.85).aspx
BOOL CProcess::isTerminated() const
{
  DWORD dwExitCode = NULL;

  ::GetExitCodeProcess(m_ProcessInformation.hProcess, &dwExitCode);

  return dwExitCode != STILL_ACTIVE;
}

BOOL CALLBACK CProcess::ThreadProc(LPVOID lpParam)
{
  CProcess* pProcess = (CProcess*)lpParam;

  if (!pProcess)
    return FALSE;

  pProcess->WaitForInputIdle();

  ::EnumWindows(pProcess->GetWindowHandle, (LPARAM)lpParam);

  if (!pProcess->m_hWnd)
    return FALSE;

  return TRUE;
}

BOOL CALLBACK CProcess::GetWindowHandle(HWND hWnd, LPARAM lParam)
{
  CProcess* pProcess = (CProcess*)lParam;

  if (!pProcess)
    return FALSE;

  if (::IsWindowVisible(hWnd))
  {
    HWND hOwner = ::GetWindow(hWnd, GW_OWNER);
    DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);

    if ((!hOwner || dwExStyle & WS_EX_APPWINDOW) && !(hOwner || dwExStyle & WS_EX_TOOLWINDOW))
    {
      DWORD dwProcessID = NULL;

      ::GetWindowThreadProcessId(hWnd, &dwProcessID);

      if (pProcess->m_ProcessInformation.dwProcessId == dwProcessID)
        pProcess->m_hWnd = hWnd;
    }
  }

  return TRUE;
}
