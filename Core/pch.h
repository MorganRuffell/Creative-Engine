
#pragma once

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4239) // A non-const reference may only be bound to an lvalue; assignment operator takes a reference to non-const
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

//This is the amount of render targets there are -- We've set this to 3
#define RTCount = 3;


#include <wrl/client.h>
#include <wrl/event.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
#define MY_IID_PPV_ARGS                     IID_PPV_ARGS


#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstddef>
#include <vector>
#include <memory>
#include <string>
#include <cwctype>
#include <exception>

#include <d3dcommon.h>
#include <ppltasks.h>
#include <functional>
#include "resource.h"
#include "Display.h"

#include "CUtility.h"
#include "VectorMath.h"
#include "CEngineBespokeTypes.h"
#include "EngineProfiling.h"
#include "Util/CommandLineArg.h"

#include "WRL/Client.h"
#include "CDirectX12Core.h"
#include <wincodec.h>



#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif


using Microsoft::WRL::ComPtr;

