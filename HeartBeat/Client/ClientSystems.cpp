#include "ClientPCH.h"
#include "ClientSystems.h"

#include "Application.h"

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

void ClientSystems::BindWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>& buffer, bool* outDirty)
{
	if (*outDirty)
	{
		Matrix mat = Matrix::CreateScale(scale);
		mat *= Matrix::CreateRotationY(XMConvertToRadians(rotation.y));
		mat *= Matrix::CreateTranslation(position);
		buffer.CopyData(0, mat);

		*outDirty = false;
	}

	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::WorldParam), buffer.GetVirtualAddress());
}

void ClientSystems::BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget,
	const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer)
{
	Matrix viewProjection = XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp);
	viewProjection *= XMMatrixPerspectiveFovLH(fov, Application::GetAspectRatio(), 0.1f, 10000.0f);
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
		float f = 1.0f - ((kAnimBlendTime - outAnimator->BlendingTime) / kAnimBlendTime);

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

		outAnimator->BlendingTime = kAnimBlendTime;
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

void ClientSystems::computeMatrixPalette(const Animation* anim, const Skeleton* skel, float animTime, MatrixPalette* outPalette)
{
	const vector<Matrix>& globalInvBindPoses = skel->GetGlobalInvBindPoses();
	vector<Matrix> currentPoses;
	anim->GetGlobalPoseAtTime(&currentPoses, skel, animTime);

	for (uint32 i = 0; i < skel->GetNumBones(); ++i)
	{
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
		outPalette->Entry[i] = globalInvBindPoses[i] * Matrix::Lerp(toPoses[i], fromPoses[i], t);
	}
}

