// TextureTjong.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

namespace py = boost::python;

py::object main_module;
py::object main_namespace;

LPCTSTR ClsName = L"BasicApp";
LPCTSTR WndName = L"A Simple Window";

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

enum {
  COMPLETION_KEY_NONE         =   0,
  COMPLETION_KEY_SHUTDOWN     =   1,
  COMPLETION_KEY_IO           =   2
};

struct WatcherThreadParams
{
  WatcherThreadParams() : dir_handle(INVALID_HANDLE_VALUE), watcher_completion_port(INVALID_HANDLE_VALUE) {}
  HANDLE dir_handle;
  HANDLE watcher_completion_port;
};

DWORD WINAPI WatcherThread(void *param);

/*
  Directory watcher thread. Uses completion ports to block until it detects a change in the directory tree
  or until a shutdown event is posted
*/
DWORD WINAPI WatcherThread(void* param)
{
  const int32_t NUM_ENTRIES = 128;
  FILE_NOTIFY_INFORMATION info[NUM_ENTRIES];

  WatcherThreadParams *params = (WatcherThreadParams*)param;


  while (true) {

    params->dir_handle = CreateFile(_T("./"), FILE_LIST_DIRECTORY, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

    if (params->dir_handle == INVALID_HANDLE_VALUE) {
      return 1;
    }

    params->watcher_completion_port = CreateIoCompletionPort(params->dir_handle, NULL, COMPLETION_KEY_IO, 0);

    if (params->watcher_completion_port == INVALID_HANDLE_VALUE) {
      return 1;
    }

    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    const BOOL res = ReadDirectoryChangesW(
      params->dir_handle, info, sizeof(info), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &overlapped, NULL);
    if (!res) {
      return 1;
    }

    DWORD bytes;
    ULONG key = COMPLETION_KEY_NONE;
    OVERLAPPED *overlapped_ptr = NULL;
    bool done = false;
    GetQueuedCompletionStatus(params->watcher_completion_port, &bytes, &key, &overlapped_ptr, INFINITE);
    switch (key) {
    case COMPLETION_KEY_NONE: 
      done = true;
      break;
    case COMPLETION_KEY_SHUTDOWN: 
      return 1;

    case COMPLETION_KEY_IO: 
      break;
    }
    CloseHandle(params->dir_handle);
    CloseHandle(params->watcher_completion_port);

    if (done) {
      break;
    } else {
      int a = 10;
/*
      FileChangedEventArgs *event = new FileChangedEventArgs();
      info[0].FileName[info[0].FileNameLength/2+0] = 0;
      info[0].FileName[info[0].FileNameLength/2+1] = 0;
      UnicodeToAnsiToBuffer(info[0].FileName, event->filename, MAX_PATH);
      // change '\' to '/'
      char *ptr = event->filename;
      while (*ptr) {
        if (*ptr == '\\') {
          *ptr = '/';
        }
        ++ptr;
      }
      g_system->send_event(EventId::FileChanged, event);
*/
    }

  }

  return 0;
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Create the application window
  WNDCLASSEX WndClsEx;
  WndClsEx.cbSize        = sizeof(WNDCLASSEX);
  WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
  WndClsEx.lpfnWndProc   = WndProcedure;
  WndClsEx.cbClsExtra    = 0;
  WndClsEx.cbWndExtra    = 0;
  WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
  WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  WndClsEx.lpszMenuName  = NULL;
  WndClsEx.lpszClassName = ClsName;
  WndClsEx.hInstance     = hInstance;
  WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  // Register the application
  RegisterClassEx(&WndClsEx);

  // Create the window object
  HWND hWnd = CreateWindow(ClsName,
    WndName,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    NULL,
    NULL,
    hInstance,
    NULL);

  if( !hWnd ) {
    return 1;
  }

  // Display the window to the user
  ShowWindow(hWnd, SW_SHOWNORMAL);
  UpdateWindow(hWnd);


  Py_SetProgramName("mr texture generator");
  Py_InitializeEx(0);
  main_module = py::import("__main__");
  main_namespace = main_module.attr("__dict__");

  const UINT timer_id = SetTimer(hWnd, 1, 1000, NULL);

  MSG msg;
  while( GetMessage(&msg, NULL, 0, 0) )
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  KillTimer(hWnd, timer_id);

  return msg.wParam;
}

void paint(HWND hwnd)
{
  HDC hDC, MemDCExercising;
  PAINTSTRUCT Ps;
  HBITMAP bmpExercising;

  hDC = BeginPaint(hwnd, &Ps);

  // Load the bitmap from the resource
  bmpExercising = (HBITMAP)LoadImageA(0, "tjong.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

  BITMAP bm;
  ::GetObject (bmpExercising, sizeof (bm), & bm);

  // Create a memory device compatible with the above DC variable
  MemDCExercising = CreateCompatibleDC(hDC);
  // Select the new bitmap
  SelectObject(MemDCExercising, bmpExercising);

  // Copy the bits from the memory DC into the current dc
  BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, MemDCExercising, 0, 0, SRCCOPY);



  // Restore the old bitmap
  DeleteDC(MemDCExercising);
  DeleteObject(bmpExercising);
  EndPaint(hwnd, &Ps);

  PAINTSTRUCT psPaint;
  HDC hdc = BeginPaint( hwnd, &psPaint );
  // Do painting here
  EndPaint (hwnd, &psPaint);

}

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
  case WM_DESTROY:
    PostQuitMessage(WM_QUIT);
    break;

  case WM_PAINT:
    paint(hWnd);
    break;

  case WM_TIMER:
    {
      try {
        py::object ignored = py::exec_file("1.py", main_namespace, main_namespace);
      } catch (py::error_already_set&) {
        PyErr_Print();
        PyErr_Clear();
      }

      InvalidateRect(hWnd, NULL, FALSE);
    }
    break;
  default:
    return DefWindowProc(hWnd, Msg, wParam, lParam);
  }
  return 0;
}
