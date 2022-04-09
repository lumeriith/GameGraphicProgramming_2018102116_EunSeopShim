#include "Camera/Camera.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::Camera

	  Summary:  Constructor

	  Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
				 m_moveUpDown, m_travelSpeed, m_rotationSpeed,
				 m_padding, m_cameraForward, m_cameraRight, m_cameraUp,
				 m_eye, m_at, m_up, m_rotation, m_view].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Camera::Camera(_In_ const XMVECTOR& position) :
		m_yaw(0.0f),
		m_pitch(0.0f),
		m_moveLeftRight(),
		m_moveBackForward(),
		m_moveUpDown(),
		m_travelSpeed(15.0f),
		m_rotationSpeed(10.0f),
		m_padding(),
		m_cameraForward(DEFAULT_FORWARD),
		m_cameraRight(DEFAULT_RIGHT),
		m_cameraUp(DEFAULT_UP),
		m_eye(position),
		m_at(),
		m_up(),
		m_rotation(),
		m_view()
	{}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::GetEye

	  Summary:  Returns the eye vector

	  Returns:  const XMVECTOR&
				  The eye vector
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMVECTOR& Camera::GetEye() const
	{
		return m_eye;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::GetAt

	  Summary:  Returns the at vector

	  Returns:  const XMVECTOR&
				  The at vector
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMVECTOR& Camera::GetAt() const
	{
		return m_at;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::GetUp

	  Summary:  Returns the up vector

	  Returns:  const XMVECTOR&
				  The up vector
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMVECTOR& Camera::GetUp() const
	{
		return m_up;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::GetView

	  Summary:  Returns the view matrix

	  Returns:  const XMMATRIX&
				  The view matrix
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMMATRIX& Camera::GetView() const
	{
		return m_view;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::HandleInput

	  Summary:  Sets the camera state according to the given input

	  Args:     const DirectionsInput& directions
				  Keyboard directional input
				const MouseRelativeMovement& mouseRelativeMovement
				  Mouse relative movement input
				FLOAT deltaTime
				  Time difference of a frame

	  Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
				 m_moveUpDown].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Camera::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
	{
		// Handle Traversal
		int xMov = 0, yMov = 0, zMov = 0;

		if (directions.bLeft) xMov = -1;
		else if (directions.bRight) xMov = 1;

		if (directions.bDown) yMov = -1;
		else if (directions.bUp) yMov = 1;

		if (directions.bBack) zMov = -1;
		else if (directions.bFront) zMov = 1;

		if (xMov != 0 || yMov != 0 || zMov != 0)
		{
			XMVECTOR xmvMove = XMVectorSet(xMov, yMov, zMov, 0.0f);
			xmvMove = XMVector3Normalize(xmvMove);
			xmvMove *= m_travelSpeed * deltaTime;
			XMFLOAT3 vMove;
			XMStoreFloat3(&vMove, xmvMove);
			m_moveLeftRight += vMove.x;
			m_moveUpDown += vMove.y;
			m_moveBackForward += vMove.z;
		}

		// Handle Aim
		int xRot = 0, yRot = 0;
		xRot = mouseRelativeMovement.X;
		yRot = mouseRelativeMovement.Y;

		if (xRot != 0 || yRot != 0)
		{
			XMVECTOR xmvRot = XMVectorSet(xRot, yRot, 0.0f, 0.0f);
			xmvRot = XMVector2Normalize(xmvRot);
			xmvRot *= m_rotationSpeed * deltaTime;
			XMFLOAT2 vRot;
			XMStoreFloat2(&vRot, xmvRot);
			m_yaw += vRot.x;
			m_pitch += vRot.y;

			if (m_pitch > XM_PIDIV2) m_pitch = XM_PIDIV2;
			else if (m_pitch < -XM_PIDIV2) m_pitch = -XM_PIDIV2;
		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Camera::Update

	  Summary:  Updates the camera based on its state

	  Args:     FLOAT deltaTime
				  Time difference of a frame

	  Modifies: [m_rotation, m_at, m_cameraRight, m_cameraUp,
				 m_cameraForward, m_eye, m_moveLeftRight,
				 m_moveBackForward, m_moveUpDown, m_up, m_view].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Camera::Update(_In_ FLOAT deltaTime)
	{
		m_rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

		m_cameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, m_rotation);
		m_cameraUp = XMVector3TransformCoord(DEFAULT_UP, m_rotation);
		m_cameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, m_rotation);

		m_eye += m_moveLeftRight * m_cameraRight;
		m_eye += m_moveUpDown * m_cameraUp;
		m_eye += m_moveBackForward * m_cameraForward;

		m_at = m_eye + m_cameraForward;
		m_up = m_cameraUp;

		m_moveLeftRight = 0.0f;
		m_moveUpDown = 0.0f;
		m_moveBackForward = 0.0f;

		m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
	}
}