#include "ClientPCH.h"
#include "ClientSystems.h"

#include "Application.h"

void ClientSystems::Move(Vector3* outPosition, const Vector3& velocity, float deltaTime)
{
	*outPosition += velocity * deltaTime;
}

void ClientSystems::BindWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>& buffer)
{
	Matrix mat = Matrix::CreateScale(scale);
	mat *= Matrix::CreateRotationY(XMConvertToRadians(rotation.y));
	mat *= Matrix::CreateTranslation(position);

	buffer.CopyData(0, mat);

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
