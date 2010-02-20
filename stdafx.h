// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

// Undefine _DEBUG when including python to avoid it wanting to link against python26_d.lib, which
// i don't have..
#ifdef _DEBUG 
#undef _DEBUG 
#include <Python.h> 
#define _DEBUG 
#else 
#include <Python.h> 
#endif 

#include <boost/python.hpp>
#include <stdint.h>

#include <vector>
#include <cassert>
