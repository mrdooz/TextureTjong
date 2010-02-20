#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>


#include <stdint.h>
#pragma warning(push)
#pragma warning(disable: 4100 4121 4244 4512)
#include <boost/python.hpp>
#pragma warning(pop)

#include "scoped_ptr.h"

#include <d3dx10.h>
#include <strstream>
