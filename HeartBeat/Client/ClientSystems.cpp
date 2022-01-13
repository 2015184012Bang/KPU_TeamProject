#include "ClientPCH.h"
#include "ClientSystems.h"

#include "Application.h"

void ClientSystems::MovePosition(Vector3* outPosition, const Vector3& velocity, float deltaTime, bool* outDirty)
{
	*outPosition += velocity * deltaTime;
	*outDirty = true;
}

void ClientSystems::BindWorldMatrix(const Vector3& position, const Vector3& rotaion, float scale, UploadBuffer<Matrix>& buffer, bool* outDirty)
{
	if (*outDirty)
	{
		Matrix mat = Matrix::CreateScale(scale);
		mat *= Matrix::CreateRotationY(XMConvertToRadians(rotaion.y));
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
