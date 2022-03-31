#pragma once

enum class CSPacket
{
	eLoginRequest,
	eImReady,
};

enum class SCPacket
{
	eLoginConfirmed,
	eUserConnected,
	eReadyPressed,
	eGameStart,
};