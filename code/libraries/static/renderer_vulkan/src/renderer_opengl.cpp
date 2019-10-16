//code from nanoglfw.cpp @zeux as reference, it's cool
//
//#include <windows.h>
//#include "dep/gl.h"
//#include <stdio.h>
//
//#pragma comment(lib, "opengl32.lib")

void glview()
{
  //HWND hWnd = CreateWindow(L"ListBox", L"Title", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,)
  //HWND hwnd;
  //HDC hDC = GetDC( hwnd );

  //PIXELFORMATDESCRIPTOR pfd = { sizeof( pfd ), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,0,32 };
  //SetPixelFormat( hDC, ChoosePixelFormat( hDC, &pfd ), &pfd );
  //wglMakeCurrent( hDC, wglCreateContext( hDC ) );

  //ShowWindow( hwnd, SW_SHOWNORMAL );

  //MSG msg;

  //while ( GetMessage( &msg, hwnd, 0, 0 ) && msg.message )
  //{
  //  TranslateMessage( &msg );
  //  DispatchMessage( &msg );
  //  RECT rect;
  //  GetClientRect( hwnd, &rect );

  //  glViewport( 0, 0, rect.right - rect.left, rect.bottom - rect.top );
  //  glClear( GL_COLOR_BUFFER_BIT );
  //  glBegin( GL_TRIANGLES );
  //  glColor3f( 1.f, 0.f, 0.f );
  //  glVertex2i( 0, 1 );
  //  glColor3f( 0.f, 1.f, 0.f );
  //  glVertex2i( -1, -1 );
  //  glColor3f( 0.f, 0.f, 1.f );
  //  glVertex2i( 1, -1 );
  //  glEnd();
  //  SwapBuffers( hDC );
  }

