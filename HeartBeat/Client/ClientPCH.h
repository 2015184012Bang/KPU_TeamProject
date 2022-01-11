#pragma once

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "DirectXTex")
#pragma comment(lib, "DirectXTK12")

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
#include "ClientComponents.h"

extern ComPtr<ID3D12Device> gDevice;
extern ComPtr<ID3D12GraphicsCommandList> gCmdList;
extern vector<ComPtr<ID3D12Resource>> gUsedUploadBuffers;
extern class TablsDescriptorHeap* gTexDescHeap;

#define RELEASE_UPLOAD_BUFFER(x) gUsedUploadBuffers.push_back(x)
