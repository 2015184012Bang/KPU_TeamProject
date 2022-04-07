#pragma once

enum class CSPacket : uint8
{
	eLoginRequest,
	eImReady,
	eUserInput,
};

enum class SCPacket : uint8
{
	eLoginConfirmed,
	eUserConnected,
	eReadyPressed,
	eGameStart,
	eCreateCharacter,
	eUpdateTransform,
};