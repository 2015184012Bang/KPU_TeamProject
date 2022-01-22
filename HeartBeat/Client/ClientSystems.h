#pragma once

#include "UploadBuffer.h"

class ClientSystems
{
public:
	static void MovePosition(Vector3* outPosition, const Vector3& velocity, float deltaTime, bool* outDirty);
	static void RotateY(Vector3* outRotation, float speed, float deltaTime, bool* outDirty);
	static void BindWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>& buffer, bool* outDirty);
	static void BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer);
	static void BindBoneMatrix(const MatrixPalette& palette, UploadBuffer<MatrixPalette>& buffer);
	static void UpdateAnimation(Animation* anim, Skeleton* skel, float* outAnimTime, float animPlayRate, bool bLoop, MatrixPalette* outPalette, float deltaTime);
	static void PlayAnimation(AnimatorComponent* outAnimator, Animation* anim, float animPlayRate, bool bLoop);

private:
	static void computeMatrixPalette(Animation* anim, Skeleton* skel, float animTime, MatrixPalette* outPalette);
};