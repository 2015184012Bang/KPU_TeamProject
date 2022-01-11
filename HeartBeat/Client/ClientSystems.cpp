#include "ClientPCH.h"
#include "ClientSystems.h"

void ClientSystems::Move(Vector3* outPosition, const Vector3& velocity, float deltaTime)
{
	*outPosition += velocity * deltaTime;
}

void ClientSystems::SetWorldMatrix(const Vector3& position, const Vector3& rotation, float scale, UploadBuffer<Matrix>& buffer)
{
	Matrix mat = Matrix::CreateScale(scale);
	mat *= Matrix::CreateRotationY(XMConvertToRadians(rotation.y));
	mat *= Matrix::CreateTranslation(position);

	buffer.CopyData(0, mat);

	gCmdList->SetGraphicsRootConstantBufferView(static_cast<uint32>(eRootParameter::WorldParam), buffer.GetVirtualAddress());
}
