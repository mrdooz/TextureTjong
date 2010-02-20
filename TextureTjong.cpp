// TextureTjong.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TextureLib/Texture.hpp"
#include "TextureLib/TextureLib.hpp"

namespace
{
  typedef std::vector<const Texture*> Textures;
  Textures textures;

  const Texture* selected_texture = NULL;

  HWND main_window = NULL;
  HWND texture_window = NULL;

  const DWORD main_thread_id = GetCurrentThreadId();

  bool is_on_main_thread()
  {
    return GetCurrentThreadId() == main_thread_id;
  }

#define WM_RELOAD (WM_APP + 1)

  std::string filename;
}

HRESULT UnicodeToAnsiToBuffer(LPCOLESTR pszW, char* buffer, const int buf_len)
{

  ULONG cbAnsi, cCharacters;
  DWORD dwError;

  // If input is null then just return the same.
  if (pszW == NULL)
  {
    return NOERROR;
  }

  cCharacters = wcslen(pszW)+1;
  // Determine number of bytes to be allocated for ANSI string. An
  // ANSI string can have at most 2 bytes per character (for Double
  // Byte Character Strings.)
  cbAnsi = cCharacters*2;

  if ((int)cbAnsi > buf_len) {
    return E_OUTOFMEMORY;
  }

  // Convert to ANSI.
  if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, buffer, cbAnsi, NULL, NULL))
  {
    dwError = GetLastError();
    return HRESULT_FROM_WIN32(dwError);
  }
  return NOERROR;
}


class ThumbNail
{
public:
  ThumbNail(const int x, const int y, const int size, const Texture* texture) : _x(x), _y(y), _size(size), _texture(texture) {}
  bool inside(const int px, const int py) const
  {
    return (px >= _x && px < _x + _size && py >= _y && py < _y + _size);
  }
  const Texture* texture() const { return _texture; }
private:
  int _x;
  int _y;
  int _size;
  const Texture* _texture;
};

typedef std::vector<ThumbNail> ThumbNails;
ThumbNails thumbnails;

namespace py = boost::python;

py::object main_module;
py::object main_namespace;

extern "C"
{
  void texture_create(const Texture* texture)
  {
    assert(is_on_main_thread());

    textures.push_back(texture);

  }

  void texture_delete(const Texture* texture)
  {
    assert(is_on_main_thread());

    if (texture == selected_texture) {
      selected_texture = NULL;
    }

    for (Textures::iterator i = textures.begin(), e = textures.end(); i != e; ++i) {
      const Texture* t = *i;
      if (t == texture) {
        textures.erase(i);
        return;
      }
    }

  }
}


void client_resize(HWND hWnd, const int width, const int height)
{
  RECT client_rect, window_rect;
  GetClientRect(hWnd, &client_rect);
  GetWindowRect(hWnd, &window_rect);
  const int diff_x = (window_rect.right - window_rect.left) - client_rect.right;
  const int diff_y = (window_rect.bottom - window_rect.top) - client_rect.bottom;
  MoveWindow(hWnd, window_rect.left, window_rect.top, width + diff_x, height + diff_y, TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
      info[0].FileName[info[0].FileNameLength/2+0] = 0;
      info[0].FileName[info[0].FileNameLength/2+1] = 0;
      char* name = new char[MAX_PATH];
      UnicodeToAnsiToBuffer(info[0].FileName, name, MAX_PATH);
      // change '\' to '/'
      char *ptr = name;
      while (*ptr) {
        if (*ptr == '\\') {
          *ptr = '/';
        }
        ++ptr;
      }

      // Only send event if the file we're watching has changed
      if (_stricmp(filename.c_str(), name) == 0) {
        PostMessage(main_window, WM_RELOAD, (WPARAM)name, 0);
      } else {
        delete [] name;
      }
    }

  }

  return 0;
}

uint8_t* downsample_and_flip(const Texture* t, const int32_t width, const int32_t height)
{
  uint32_t* data = new uint32_t[width*height];
  uint8_t* ptr = (uint8_t*)data;

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      *data++ = t->at((float)j/(width-1), (float)(height - 1 - i)/(height-1));
    }
  }

  return (uint8_t*)ptr;
}

uint8_t* flip(const Texture* t)
{
  const int height = t->height();
  const int width = t->width();

  uint32_t* src = (uint32_t*)(t->data() + (height - 1) * width * 4);
  uint32_t* dst = new uint32_t[width * height];
  uint32_t* p = dst;

  for (int i = 0; i < height; ++i) {
    memcpy(dst, src, width * 4);
    dst += width;
    src -= width;
  }

  return (uint8_t*)p;
}

void fill_bitmap_info(const Texture* t, BITMAPINFO* info)
{
  ZeroMemory(info, sizeof(*info));
  info->bmiHeader.biSize = sizeof(info->bmiHeader);
  info->bmiHeader.biWidth = t->width();
  info->bmiHeader.biHeight = t->height();
  info->bmiHeader.biPlanes = 1;
  info->bmiHeader.biBitCount = 32;
  info->bmiHeader.biCompression = BI_RGB;

}

void paint_single_texture(HWND hwnd)
{
  PAINTSTRUCT ps;
  HDC dc = BeginPaint(hwnd, &ps);

  if (selected_texture) {
    BITMAPINFO bm_info;
    fill_bitmap_info(selected_texture, &bm_info);

    uint8_t* flipped = flip(selected_texture);
    SetDIBitsToDevice(dc, 0, 0, selected_texture->width(), selected_texture->height(), 0, 0, 0, 
      selected_texture->height(), flipped, &bm_info, DIB_RGB_COLORS);
    delete[] (xchg_null(flipped));

    RECT client_rect;
    GetClientRect(hwnd, &client_rect);

    DrawTextA(dc, selected_texture->name().c_str(), -1, &client_rect, DT_LEFT);
  }

  EndPaint(hwnd, &ps);
}

void paint_thumbnails(HWND hwnd)
{
  PAINTSTRUCT Ps;
  HDC hDC = BeginPaint(hwnd, &Ps);

  const int thumbnail_size = 128;

  BITMAPINFO bm_info;
  ZeroMemory(&bm_info, sizeof(bm_info));
  bm_info.bmiHeader.biSize = sizeof(bm_info.bmiHeader);
  bm_info.bmiHeader.biWidth = thumbnail_size;
  bm_info.bmiHeader.biHeight = thumbnail_size;
  bm_info.bmiHeader.biPlanes = 1;
  bm_info.bmiHeader.biBitCount = 32;
  bm_info.bmiHeader.biCompression = BI_RGB;

  thumbnails.clear();

  const int spacing = 10;
  int x_pos = spacing, y_pos = spacing;
  for (size_t i = 0; i < textures.size(); ++i) {
    if (x_pos + thumbnail_size > Ps.rcPaint.right) {
      x_pos = spacing;
      y_pos += thumbnail_size + spacing;
    }

    const Texture* t = textures[i];
    uint8_t* downsampled = downsample_and_flip(t, thumbnail_size, thumbnail_size);
    SetDIBitsToDevice(hDC, x_pos, y_pos, thumbnail_size, thumbnail_size, 0, 0, 0, thumbnail_size, downsampled, &bm_info, DIB_RGB_COLORS);
    thumbnails.push_back(ThumbNail(x_pos, y_pos, thumbnail_size, t));
    delete [] downsampled;

    x_pos += thumbnail_size + spacing;

  }

  EndPaint(hwnd, &Ps);
}

LRESULT CALLBACK WndProc2(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
  case WM_PAINT:
    paint_single_texture(texture_window);
    break;

  default:
    return DefWindowProc(hWnd, Msg, wParam, lParam);
  }
  return 0;
}

bool update_selected_texture(const int x, const int y)
{
  for (int i = 0, e = thumbnails.size(); i < e; ++i) {
    const ThumbNail& cur = thumbnails[i];
    if (cur.inside(x, y)) {
      if (cur.texture() != selected_texture) {
        selected_texture = cur.texture();
        return true;
      }
      return false;
    }
  }
  return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
  case WM_DESTROY:
    PostQuitMessage(WM_QUIT);
    break;

  case WM_PAINT:
    paint_thumbnails(hWnd);
    break;

  case WM_LBUTTONUP:
    if (update_selected_texture(LOWORD(lParam), HIWORD(lParam))) {
      client_resize(texture_window, selected_texture->width(), selected_texture->height());
      InvalidateRect(texture_window, NULL, FALSE);
    }
    break;

  case WM_RELOAD:
    {
      char* name = (char*)wParam;

      try {
        py::object ignored = py::exec_file(name, main_namespace, main_namespace);
      } catch (py::error_already_set&) {
        PyErr_Print();
        PyErr_Clear();
      }

      if (!textures.empty()) {
        InvalidateRect(main_window, NULL, FALSE);
        InvalidateRect(texture_window, NULL, FALSE);
      }

      delete [] name;
    }


  default:
    return DefWindowProc(hWnd, Msg, wParam, lParam);
  }
  return 0;
}



HWND create_window(HINSTANCE hInstance, const int width, const int height, LPCTSTR class_name, LPCTSTR window_name, WNDPROC wnd_proc)
{
  // Create the application window
  WNDCLASSEX WndClsEx;
  WndClsEx.cbSize        = sizeof(WNDCLASSEX);
  WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
  WndClsEx.lpfnWndProc   = wnd_proc;
  WndClsEx.cbClsExtra    = 0;
  WndClsEx.cbWndExtra    = 0;
  WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
  WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  WndClsEx.lpszMenuName  = NULL;
  WndClsEx.lpszClassName = class_name;
  WndClsEx.hInstance     = hInstance;
  WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  // Register the application
  RegisterClassEx(&WndClsEx);

  // Create the window object
  HWND hWnd = CreateWindow(class_name,
    window_name,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width,
    height,
    NULL,
    NULL,
    hInstance,
    NULL);

  if (hWnd) {
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
  }

  client_resize(hWnd, width, height);

  return hWnd;

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  main_window = create_window(hInstance, 800, 600, _T("TexGenMainWndClass"), _T("Main Window"), WndProc);
  texture_window = create_window(hInstance, 512, 512, _T("TexGenTextureWndClass"), _T("Texture"), WndProc2);

  if (main_window == NULL || texture_window == NULL) {
    return 1;
  }

  Py_SetProgramName("mr texture generator");
  Py_InitializeEx(0);
  main_module = py::import("__main__");
  main_namespace = main_module.attr("__dict__");


  // load the texture module, and set the texture create/delete callbacks
  HMODULE texture_ext = LoadLibrary(_T("texture_ext.pyd"));

  pfnSetTextureInit set_texture_init = (pfnSetTextureInit)GetProcAddress(texture_ext, "setTextureInit");
  pfnSetTextureClose set_texture_close = (pfnSetTextureClose)GetProcAddress(texture_ext, "setTextureClose");

  if (set_texture_init == 0 || set_texture_close == 0) {
    return 1;
  }

  set_texture_init(texture_create);
  set_texture_close(texture_delete);

  filename = "1.py";
  WatcherThreadParams params;

  DWORD thread_id;
  HANDLE watcher_thread = CreateThread(0, 0, WatcherThread, (void*)&params, 0, &thread_id);

  // post a message to get things rolling
  char* p = new char[MAX_PATH];
  strcpy(p, filename.c_str());
  PostMessage(main_window, WM_RELOAD, (WPARAM)p, 0);


  MSG msg;
  while( GetMessage(&msg, NULL, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  PostQueuedCompletionStatus(params.watcher_completion_port, 0, COMPLETION_KEY_SHUTDOWN, 0);
  WaitForSingleObject(watcher_thread, INFINITE);

  FreeLibrary(texture_ext);

  return msg.wParam;
}
