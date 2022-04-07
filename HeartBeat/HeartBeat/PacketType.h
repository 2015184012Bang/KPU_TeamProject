#pragma once

enum class CSPacket : uint8
{
	eLoginRequest,
	eImReady,
};

enum class SCPacket : uint8
{
	eLoginConfirmed,
	eUserConnected,
	eReadyPressed,
	eGameStart,
	eCreateCharacter,
};