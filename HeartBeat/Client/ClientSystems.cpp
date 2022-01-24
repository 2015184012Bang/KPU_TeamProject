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

void ClientSystems::UpdateAnimation(const Animation* anim, const Skeleton* skel, 
	float* outAnimTime, float animPlayRate, bool bLoop, MatrixPalette* outPalette, float deltaTime)
{
	if (!anim || !skel)
	{
		return;
	}

	*outAnimTime += deltaTime * animPlayRate;

	if (*outAnimTime > anim->GetDuration())
	{
		if (bLoop)
		{
			*outAnimTime -= anim->GetDuration();
		}
	}

	computeMatrixPalette(anim, skel, *outAnimTime, outPalette);
}

void ClientSystems::PlayAnimation(AnimatorComponent* outAnimator, Animation* anim, float animPlayRate, bool bLoop)
{
	if (!anim)
	{
		HB_ASSERT(false, "Invalid animation. ASSERTION FAILED");
	}

	outAnimator->Anim = anim;
	outAnimator->AnimPlayRate = animPlayRate;
	outAnimator->AnimTime = 0.0f;
	outAnimator->bLoop = bLoop;
	
	computeMatrixPalette(anim, outAnimator->Skel, outAnimator->AnimTime, &outAnimator->Palette);
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

