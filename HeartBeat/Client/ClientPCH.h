#pragma once

#include "HeartBeat.h"

#include "d3dHelper.h"
#include "Types.h"

extern ComPtr<ID3D12Device> gDevice;
extern ComPtr<ID3D12GraphicsCommandList> gCmdList;
extern vector<ComPtr<ID3D12Resource>> gUsedUploadBuffers;
extern class TableDescriptorHeap* gTexDescHeap;

#define RELEASE_UPLOAD_BUFFER(x) gUsedUploadBuffers.push_back(x)
