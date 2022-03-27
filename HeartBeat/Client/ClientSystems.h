#pragma once

#include "UploadBuffer.h"

class ClientSystems
{
public:
	static void MovePosition(Vector3* outPosition, const Vector3& velocity, float deltaTime, bool* outDirty);
	static void RotateY(Vector3* outRotation, float speed, float deltaTime, bool* outDirty);
	static void BindWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>* outBuffer, bool* outDirty);
	static void BindWorldMatrix(const Vector2& position, UploadBuffer<Matrix>* outBuffer, bool* outDirty);
	static void BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer);
	static void BindViewProjectionMatrixOrtho(UploadBuffer<Matrix>& buffer);
	static void BindBoneMatrix(const MatrixPalette& palette, UploadBuffer<MatrixPalette>& buffer);
	static void UpdateAnimation(AnimatorComponent* outAnimator, float deltaTime);
	static void PlayAnimation(AnimatorComponent* outAnimator, Animation* toAnim, float animPlayRate);
	static void UpdateBox(const AABB* const localBox, AABB* outWorldBox, const Vector3& position, float yaw, bool bDirty);
	static bool Intersects(const AABB& a, const AABB& b);
	static bool Intersects(const Vector2& position, int w, int h);
	static Vector3 ScreenToClip(const Vector2& coord);

private:
	static void computeMatrixPalette(const Animation* anim, const Skeleton* skel, float animTime, MatrixPalette* outPalette);
	static void computeBlendingMatrixPalette(const Animation* fromAnim, const Animation* toAnim, const Skeleton* skel, float fromAnimTime, float toAnimTime, float t, MatrixPalette* outPalette);
};