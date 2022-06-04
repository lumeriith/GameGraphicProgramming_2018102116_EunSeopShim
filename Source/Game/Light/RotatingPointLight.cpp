#include "Light/RotatingPointLight.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   RotatingPointLight::RotatingPointLight

  Summary:  Constructor

  Args:     const XMFLOAT4& position
			  Position of the light
			const XMFLOAT4& color
			  Position of the color
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
RotatingPointLight::RotatingPointLight(_In_ const XMFLOAT4& position, _In_ const XMFLOAT4& color, _In_ FLOAT attenuationDistance) :
	PointLight(position, color, attenuationDistance)
{ }

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   RotatingPointLight::Update

  Summary:  Rotates the light every frame

  Args:     FLOAT deltaTime
			  Elapsed time
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void RotatingPointLight::Update(_In_ FLOAT deltaTime)
{
	XMMATRIX rotate = XMMatrixRotationY(-2.0f * deltaTime);
	XMVECTOR position = XMLoadFloat4(&m_position);
	position = XMVector3Transform(position, rotate);
	XMStoreFloat4(&m_position, position);

	m_eye = position;
	m_at = XMVectorSet(0, 0, 0, 1);
	m_up = DEFAULT_UP;

	m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
}
