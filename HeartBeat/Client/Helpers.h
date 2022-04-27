#pragma once

#include "UploadBuffer.h"
#include "Bone.h"
#include "Entity.h"

using namespace std::string_view_literals;

struct AnimatorComponent;
struct TransformComponent;
struct AttachmentChildComponent;
class Animation;
class AABB;
class Skeleton;

class Helpers
{
public:
	static void UpdatePosition(Vector3* outPosition, const Vector3& to, bool* outDirty);
	static void UpdateYRotation(float* outYRotation, const float to, bool* outDirty);
	static void BindWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>* outBuffer, bool* outDirty);
	static void BindWorldMatrix(const Vector2& position, UploadBuffer<Matrix>* outBuffer, bool* outDirty);
	static void BindWorldMatrixAttached(TransformComponent* outTransform, const AttachmentChildComponent* attachment);
	static void BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer);
	static void BindViewProjectionMatrixOrtho(UploadBuffer<Matrix>& buffer);
	static void BindBoneMatrix(const MatrixPalette& palette, UploadBuffer<MatrixPalette>& buffer);
	static void UpdateAnimation(AnimatorComponent* outAnimator, float deltaTime);
	static void PlayAnimation(AnimatorComponent* outAnimator, Animation* toAnim, float animPlayRate = 1.0f);
	static void UpdateBox(const AABB* const localBox, AABB* outWorldBox, const Vector3& position, float yaw, bool bDirty);
	static bool Intersects(const AABB& a, const AABB& b);
	static bool Intersects(const Vector2& position, int w, int h);
	static Vector3 ScreenToClip(const Vector2& coord);
	static void AttachBone(Entity& parent, Entity& child, string_view boneName);

	// [사용법]
	// 1. 모든 자식 엔티티를 삭제하고 싶다
	// bAll : true, boneName : 디폴트
	// 2. 특정 자식 엔티티만 삭제하고 싶다
	// bAll : false, boneName: 삭제할 엔티티가 붙어있는 본의 이름
	static vector<entt::entity> GetEntityToDetach(Entity& parent, bool bAll = true, string_view boneName = ""sv);

private:
	static void computeMatrixPalette(const Animation* anim, const Skeleton* skel, float animTime, MatrixPalette* outPalette);
	static void computeBlendingMatrixPalette(const Animation* fromAnim, const Animation* toAnim, const Skeleton* skel, float fromAnimTime, float toAnimTime, float t, MatrixPalette* outPalette);
};