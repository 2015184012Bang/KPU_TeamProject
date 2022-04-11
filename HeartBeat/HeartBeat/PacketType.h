#pragma once

enum class CSPacket : uint8
{
	eLoginRequest,
	eImReady,
	eUserKeyboardInput,
	eUserMouseInput,
};

enum class SCPacket : uint8
{
	eLoginConfirmed,
	eUserConnected,
	eReadyPressed,
	eGameStart,
	eCreateCharacter,
	eUpdateTransform,
	eUpdateCollision,
	eCreateEnemy,
	eDeleteEntity,
	eCreateTank,
};