#include "ClientPCH.h"
#include "ClientSystems.h"

#include "AABB.h"
#include "Application.h"
#include "Animation.h"
#include "ClientComponents.h"
#include "Skeleton.h"

void ClientSystems::MovePosition(Vector3* outPosition, const Vector3& velocity, float deltaTime, bool* outDirty)
{
	*outPosition += velocity * deltaTime;
	*outDirty = true;
}

void ClientSystems::RotateY(Vector3* outRotation, float speed, float deltaTime, bool* outDirty)
{
	(*outRotation).y += speed * deltaTime;
	*outDirty = true;
}

void ClientSystems::BindWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>* outBuffer, bool* outDirty)
{
	if (*outDirty)
	{
		Matrix mat = Matrix::CreateScale(scale);
		mat *= Matrix::CreateRotationY(XMConvertToRadians(rotation.y));
		mat *= Matrix::CreateTranslation(position);
		outBuffer->CopyData(0, mat);

		*outDirty = false;
	}

	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::WorldParam), outBuffer->GetVirtualAddress());
}

void ClientSystems::BindWorldMatrix(const Vector2& position, UploadBuffer<Matrix>* outBuffer, bool* outDirty)
{
	Vector3 converted = ScreenToClip(position);

	BindWorldMatrix(converted, Vector3::Zero, 1.0f, outBuffer, outDirty);
}

void ClientSystems::BindWorldMatrixAttached(TransformComponent* outTransform, const AttachmentChildComponent* attachment)
{
	Matrix mat = attachment->ParentPalette->CurrentPoses[attachment->BoneIndex]; 
	mat *= Matrix::CreateScale(outTransform->Scale);
	mat *= Matrix::CreateRotationY(XMConvertToRadians(outTransform->Rotation.y));
	mat *= Matrix::CreateTranslation(outTransform->Position);
	outTransform->Buffer.CopyData(0, mat);

	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::WorldParam), outTransform->Buffer.GetVirtualAddress());
}

void ClientSystems::BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget,
	const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer)
{
	Matrix viewProjection = XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp);
	viewProjection *= XMMatrixPerspectiveFovLH(fov, Application::GetAspectRatio(), 0.1f, 10000.0f);
	buffer.CopyData(0, viewProjection);
	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::ViewProjParam), buffer.GetVirtualAddress());
}

void ClientSystems::BindViewProjectionMatrixOrtho(UploadBuffer<Matrix>& buffer)
{
	Matrix viewProjection = Matrix::Identity;
	viewProjection *= XMMatrixOrthographicLH(static_cast<float>(Application::GetScreenWidth()), static_cast<float>(Application::GetScreenHeight()), 0.0f, 1.0f);
	buffer.CopyData(0, viewProjection);
	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::ViewProjParam), buffer.GetVirtualAddress());
}

void ClientSystems::BindBoneMatrix(const MatrixPalette& palette, UploadBuffer<MatrixPalette>& buffer)
{
	buffer.CopyData(0, palette);
	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::BoneParam), buffer.GetVirtualAddress());
}

void ClientSystems::UpdateAnimation(AnimatorComponent* outAnimator, float deltaTime)
{
	if (!outAnimator->CurAnim || !outAnimator->Skel)
	{
		return;
	}

	outAnimator->CurAnimTime += deltaTime * outAnimator->AnimPlayRate;

	if (outAnimator->CurAnimTime > outAnimator->CurAnim->GetDuration())
	{
		if (outAnimator->CurAnim->IsLoop())
		{
			outAnimator->CurAnimTime -= outAnimator->CurAnim->GetDuration();
		}
		else
		{
			outAnimator->SetTrigger("WhenEnd");
		}
	}

	if (outAnimator->BlendingTime > 0.0f)
	{
		float f = 1.0f - ((ANIM_BLEND_TIME - outAnimator->BlendingTime) / ANIM_BLEND_TIME);

		computeBlendingMatrixPalette(outAnimator->PrevAnim, outAnimator->CurAnim, outAnimator->Skel, outAnimator->PrevAnimTime, outAnimator->CurAnimTime,
			f, &outAnimator->Palette);

		outAnimator->BlendingTime -= deltaTime * outAnimator->AnimPlayRate;
	}
	else
	{
		computeMatrixPalette(outAnimator->CurAnim, outAnimator->Skel, outAnimator->CurAnimTime, &outAnimator->Palette);
	}
}

void ClientSystems::PlayAnimation(AnimatorComponent* outAnimator, Animation* toAnim, float animPlayRate)
{
	if (outAnimator->CurAnim == toAnim)
	{
		HB_LOG("Self transition is not allowed!");
		return;
	}

	if (outAnimator->CurAnim == nullptr)
	{
		outAnimator->PrevAnim = toAnim;
		outAnimator->PrevAnimTime = 0.0f;
		outAnimator->BlendingTime = 0.0f;
	}
	else
	{
		outAnimator->PrevAnim = outAnimator->CurAnim;
		outAnimator->PrevAnimTime = outAnimator->CurAnimTime;

		if (outAnimator->PrevAnimTime > outAnimator->PrevAnim->GetDuration())
		{
			outAnimator->PrevAnimTime = outAnimator->PrevAnim->GetDuration();
		}

		outAnimator->BlendingTime = ANIM_BLEND_TIME;
	}

	outAnimator->CurAnim = toAnim;
	outAnimator->AnimPlayRate = animPlayRate;
	outAnimator->CurAnimTime = 0.0f;
}

void ClientSystems::UpdateBox(const AABB* const localBox, AABB* outWorldBox, const Vector3& position, float yaw, bool bDirty)
{
	if (!bDirty)
	{
		return;
	}

	*outWorldBox = *localBox;

	outWorldBox->UpdateWorldBox(position, yaw);
}

bool ClientSystems::Intersects(const AABB& a, const AABB& b)
{
	const Vector3& aMin = a.GetMin();
	const Vector3& aMax = a.GetMax();

	const Vector3& bMin = b.GetMin();
	const Vector3& bMax = b.GetMax();

	bool no = aMax.x < bMin.x ||
		aMax.y < bMin.y ||
		aMax.z < bMin.z ||
		bMax.x < aMin.x ||
		bMax.y < aMin.y ||
		bMax.z < aMin.z;

	return !no;
}

bool ClientSystems::Intersects(const Vector2& position, int w, int h)
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(Application::GetHwnd(), &p);

	bool contains = (p.x >= position.x) && (p.x <= position.x + w)
		&& (p.y >= position.y) && (p.y <= position.y + h);

	return contains;
}

void ClientSystems::computeMatrixPalette(const Animation* anim, const Skeleton* skel, float animTime, MatrixPalette* outPalette)
{
	const vector<Matrix>& globalInvBindPoses = skel->GetGlobalInvBindPoses();
	vector<Matrix> currentPoses;
	anim->GetGlobalPoseAtTime(&currentPoses, skel, animTime);

	for (uint32 i = 0; i < skel->GetNumBones(); ++i)
	{
		outPalette->CurrentPoses[i] = currentPoses[i];
		outPalette->Entry[i] = globalInvBindPoses[i] * currentPoses[i];
	}
}

void ClientSystems::computeBlendingMatrixPalette(const Animation* fromAnim, const Animation* toAnim, const Skeleton* skel, float fromAnimTime, float toAnimTime, float t, MatrixPalette* outPalette)
{
	const vector<Matrix>& globalInvBindPoses = skel->GetGlobalInvBindPoses();

	vector<Matrix> fromPoses;
	fromAnim->GetGlobalPoseAtTime(&fromPoses, skel, fromAnimTime);

	vector<Matrix> toPoses;
	toAnim->GetGlobalPoseAtTime(&toPoses, skel, toAnimTime);

	for (uint32 i = 0; i < skel->GetNumBones(); ++i)
	{
		outPalette->CurrentPoses[i] = Matrix::Lerp(toPoses[i], fromPoses[i], t);
		outPalette->Entry[i] = globalInvBindPoses[i] * outPalette->CurrentPoses[i];
	}
}

Vector3 ClientSystems::ScreenToClip(const Vector2& coord)
{
	Vector3 v;
	v.x = -(Application::GetScreenWidth() / 2) + coord.x;
	v.y = (Application::GetScreenHeight() / 2) - coord.y;
	v.z = 0.0f;

	return v;
}

void ClientSystems::SetBoneAttachment(Entity& parent, Entity& child, const string& boneName)
{
	// Add AttachmentComponent to child
	auto& parentAnimator = parent.GetComponent<AnimatorComponent>();
	uint32 boneIndex = parentAnimator.Skel->GetBoneIndexByName(boneName);
	child.AddComponent<AttachmentChildComponent>(&parentAnimator.Palette, boneIndex);

	// Add ChildComponent to parent
	auto& childId = child.GetComponent<IDComponent>();
	parent.AddComponent<AttachmentParentComponent>(childId.ID);
}
