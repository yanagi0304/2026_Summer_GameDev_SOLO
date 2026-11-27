#include "HologramConstantBuffer.h"

#include "../../../pch.h"

namespace
{
	// HologramCommon.hlsli ‚Ì cbuffer ‚Æ•À‚Ñ‚ðˆê’v‚³‚¹‚é‚±‚Æ
	struct HologramConstant
	{
		float color[3];
		float time;

		float textureRate;
		float alpha;
		float scanPitch;
		float scanSpeed;

		float scanStrength;
		float rimPower;
		float rimStrength;
		float brightness;

		float bandInterval;
		float bandSpeed;
		float bandStrength;
		float flicker;

		float glitchWidth;
		float glitchRate;
		float depthOnly;
		float padding;
	};
}

void HologramConstantBuffer::Create(void)
{
	if (handle != -1) { return; }
	handle = CreateShaderConstantBuffer(sizeof(HologramConstant));
}

void HologramConstantBuffer::Set(const HologramParam& param, float time, float textureRate, bool depthOnly)
{
	HologramConstant* data = static_cast<HologramConstant*>(GetBufferShaderConstantBuffer(handle));
	if (data == nullptr) { return; }

	*data = {
		{ param.colorR, param.colorG, param.colorB }, time,
		textureRate, param.alpha, param.scanPitch, param.scanSpeed,
		param.scanStrength, param.rimPower, param.rimStrength, param.brightness,
		param.bandInterval, param.bandSpeed, param.bandStrength, param.flicker,
		param.glitchWidth, param.glitchRate, depthOnly ? 1.0f : 0.0f, 0.0f
	};

	UpdateShaderConstantBuffer(handle);
	SetShaderConstantBuffer(handle, DX_SHADERTYPE_VERTEX, REGISTER);
	SetShaderConstantBuffer(handle, DX_SHADERTYPE_PIXEL, REGISTER);
}

void HologramConstantBuffer::Reset(void)
{
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_VERTEX, REGISTER);
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_PIXEL, REGISTER);
}

void HologramConstantBuffer::Release(void)
{
	if (handle == -1) { return; }
	DeleteShaderConstantBuffer(handle);
	handle = -1;
}
