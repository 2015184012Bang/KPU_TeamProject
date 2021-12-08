#pragma once

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

#include "HeartBeat.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXTK12/SimpleMath.h>
#include <DirectXTex.h>
#include <DirectXTex.inl>
#include "d3dx12.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace DirectX::PackedVector;
using namespace DirectX::SimpleMath;

#include "d3dx12.h"
#include "d3dHelper.h"

static ComPtr<ID3D12Device> gDevice;