#include "Camera.h"
#include "Input.h"


int Camera::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (((uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) && m_nRotateModelButtonMask & MOUSE_LEFT_BUTTON) ||
		((uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) && m_nRotateModelButtonMask & MOUSE_MIDDLE_BUTTON) ||
		((uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) && m_nRotateModelButtonMask & MOUSE_RIGHT_BUTTON))
	{
		int iMouseX = (short)LOWORD(lParam);
		int iMouseY = (short)HIWORD(lParam);
		m_WorldArcBall.OnBegin(iMouseX, iMouseY);
	}

	if (((uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) && m_nRotateCameraButtonMask & MOUSE_LEFT_BUTTON) ||
		((uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) && m_nRotateCameraButtonMask & MOUSE_MIDDLE_BUTTON) ||
		((uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) && m_nRotateCameraButtonMask & MOUSE_RIGHT_BUTTON))
	{
		int iMouseX = (short)LOWORD(lParam);
		int iMouseY = (short)HIWORD(lParam);
		m_ViewArcBall.OnBegin(iMouseX, iMouseY);
	}

	if (uMsg == WM_MOUSEMOVE)
	{
		int iMouseX = (short)LOWORD(lParam);
		int iMouseY = (short)HIWORD(lParam);
		m_WorldArcBall.OnMove(iMouseX, iMouseY);
		m_ViewArcBall.OnMove(iMouseX, iMouseY);
	}

	if ((uMsg == WM_LBUTTONUP && m_nRotateModelButtonMask & MOUSE_LEFT_BUTTON) ||
		(uMsg == WM_MBUTTONUP && m_nRotateModelButtonMask & MOUSE_MIDDLE_BUTTON) ||
		(uMsg == WM_RBUTTONUP && m_nRotateModelButtonMask & MOUSE_RIGHT_BUTTON))
	{
		m_WorldArcBall.OnEnd();
	}

	if ((uMsg == WM_LBUTTONUP && m_nRotateCameraButtonMask & MOUSE_LEFT_BUTTON) ||
		(uMsg == WM_MBUTTONUP && m_nRotateCameraButtonMask & MOUSE_MIDDLE_BUTTON) ||
		(uMsg == WM_RBUTTONUP && m_nRotateCameraButtonMask & MOUSE_RIGHT_BUTTON))
	{
		m_ViewArcBall.OnEnd();
	}
	// LBUTTONUP를 다른 이벤트(윈도우키 등) 때문에 받지 못할 때 호출된다.
	if (uMsg == WM_CAPTURECHANGED)
	{
		if ((HWND)lParam != hWnd)
		{
			if ((m_nRotateModelButtonMask & MOUSE_LEFT_BUTTON) ||
				(m_nRotateModelButtonMask & MOUSE_MIDDLE_BUTTON) ||
				(m_nRotateModelButtonMask & MOUSE_RIGHT_BUTTON))
			{
				m_WorldArcBall.OnEnd();
			}

			if ((m_nRotateCameraButtonMask & MOUSE_LEFT_BUTTON) ||
				(m_nRotateCameraButtonMask & MOUSE_MIDDLE_BUTTON) ||
				(m_nRotateCameraButtonMask & MOUSE_RIGHT_BUTTON))
			{
				m_ViewArcBall.OnEnd();
			}
		}
	}


	if (uMsg == WM_MOUSEWHEEL)
	{
		// Update member var state
		m_nMouseWheelDelta += (short)HIWORD(wParam);
	}
	return -1;
}
void Camera::SetButtonMasks(int nRotateModelButtonMask, int nZoomButtonMask, int nRotateCameraButtonMask)
{
	m_nRotateModelButtonMask = nRotateModelButtonMask,
		m_nZoomButtonMask = nZoomButtonMask;
	m_nRotateCameraButtonMask = nRotateCameraButtonMask;
}

void	Camera::UpdateVector()
{
	m_vRight.x = m_matView._11;
	m_vRight.y = m_matView._21;
	m_vRight.z = m_matView._31;

	m_vUp.x = m_matView._12;
	m_vUp.y = m_matView._22;
	m_vUp.z = m_matView._32;

	m_vLook.x = m_matView._13;
	m_vLook.y = m_matView._23;
	m_vLook.z = m_matView._33;

	m_vRight.Normalize();
	m_vUp.Normalize();
	m_vLook.Normalize();

	m_vFrustum.CreateFrustum(&m_matView, &m_matProj);
}

void	Camera::SetObjectView(TVector3 vMax, TVector3 vMin)
{
	TMatrix	matView;
	TVector3 vCenter = (vMax + vMin) * 0.5f;
	TVector3 vDis(vMax - vMin);
	float fRadius = D3DXVec3Length(&vDis) * 0.5f;

	TVector3 vTarget = TVector3(vCenter.x, vCenter.y, vCenter.z);
	TVector3 vPos = vTarget + (-m_vLook * (fRadius * 2));
	TVector3 vUp = { 0.0f, 1.0f, 0.0f };
	D3DXMatrixLookAtLH(&m_matView, &vPos, &vTarget, &vUp);

	m_vPos = vPos;
	m_vTarget = vTarget;
}

void Camera::GetCalcYawPitchRoll(float& fYaw, float& fPitch, float& fRoll)
{
	//TVector3 vScale;
	//TVector3 vPos;
	//TQuaternion vRotation;
	//D3DXMatrixDecompose(&vScale, &vRotation, &vPos, &m_matView);
	///*TVector3 vDir(m_matView._13, m_matView._23, m_matView._33);*/
	//fYaw = XMConvertToDegrees(atan2f(vRotation.z, vRotation.x));
	//float fLength = sqrtf(vRotation.x * vRotation.x + vRotation.z * vRotation.z);
	//fPitch = XMConvertToDegrees(atan2f(-vRotation.y, fLength));
	//fRoll = 0.0f;
	TVector3 vScale;
	TVector3 vPos;
	TQuaternion vRotation;
	D3DXMatrixDecompose(&vScale, &vRotation, &vPos, &m_matView);

	// 회전행렬에서 yaw, pitch, roll 값을 계산
	// 회전행렬에서 각 축 벡터를 추출하여 각도를 계산할 수 있다.
	TVector3 xAxis(m_matView._11, m_matView._12, m_matView._13);
	TVector3 yAxis(m_matView._21, m_matView._22, m_matView._23);
	TVector3 zAxis(m_matView._31, m_matView._32, m_matView._33);

	// Yaw (y 축 기준 회전 각도)
	fYaw = XMConvertToDegrees(atan2f(xAxis.z, xAxis.x));

	// Pitch (x 축 기준 회전 각도)
	fPitch = XMConvertToDegrees(atan2f(-yAxis.z, sqrtf(yAxis.x * yAxis.x + yAxis.y * yAxis.y)));

	// Roll (z 축 기준 회전 각도)
	//fRoll = XMConvertToDegrees(atan2f(zAxis.y, yAxis.y));
	fRoll = 0.0f;
}

void Camera::CreateViewMatrix(TVector3 vEye, TVector3 vAt, TVector3 vUp)
{
	//m_vDefaultEye = m_vCameraPos = vPos;
	//m_vDefaultLookAt = m_vTargetPos = vTarget;
	m_vPos = vEye;
	m_vTarget = vAt;
	D3DXMatrixLookAtLH(&m_matView, &m_vPos, &m_vTarget, &vUp);

	TQuaternion qRotation;
	TVector3 vTrans, vScale, vAxis;
	TMatrix mInvView;
	TQuaternion q;
	float fAngle;

	if (SUCCEEDED(D3DXMatrixDecompose(&vScale, &qRotation, &vTrans, &m_matView)))
	{
		D3DXQuaternionNormalize(&qRotation, &qRotation);
		D3DXQuaternionToAxisAngle(&qRotation, &vAxis, &fAngle);
		// 반드시 정규화 해야 한다.
		D3DXVec3Normalize(&vAxis, &vAxis);

		q.x = sin(fAngle / 2) * vAxis.x;
		q.y = sin(fAngle / 2) * vAxis.y;
		q.z = sin(fAngle / 2) * vAxis.z;
		q.w = cos(fAngle / 2);
	}
	D3DXMatrixInverse(&mInvView, NULL, &m_matView);
	TVector3* pZBasis = (TVector3*)&mInvView._31;

	m_fCameraYawAngle = atan2f(pZBasis->x, pZBasis->z);
	float fLen = sqrtf(pZBasis->z * pZBasis->z + pZBasis->x * pZBasis->x);
	m_fCameraPitchAngle = -atan2f(pZBasis->y, fLen);

	UpdateVector();
}

void Camera::CreateProjMatrix(float fNear, float fFar, float fFovY, float fAspectRatio)
{
	m_fFovY = fFovY;
	m_fAspectRatio = fAspectRatio;
	m_fNear = fNear;
	m_fFar = fFar;

	D3DXMatrixPerspectiveFovLH(&m_matProj, m_fFovY, m_fAspectRatio, m_fNear, m_fFar);
}

void Camera::Update(TVector4 vDirValue)
{
	TMatrix matRotation;
	D3DXQuaternionRotationYawPitchRoll(&m_qRotation, vDirValue.y, vDirValue.x, vDirValue.z);

	m_vPos += m_vLook * vDirValue.w;
	m_fRadius += vDirValue.w;

	D3DXMatrixAffineTransformation(&matRotation, 1.0f, NULL, &m_qRotation, &m_vPos);
	D3DXMatrixInverse(&m_matView, NULL, &matRotation);

	UpdateVector();
}

bool Camera::Frame()
{
	// 카메라 이동 거리 = 속도 + ( 경과시간 * 마우스 휠 변위값 )
	float fDistance = m_fSpeed * g_fSecondPerFrame;

	if (I_Input.GetKey(VK_SPACE) == KEY_HOLD)	m_fSpeed += g_fSecondPerFrame * 10.0f;
	else						m_fSpeed -= g_fSecondPerFrame * 10.0f;
	// 최소값으로 고정
	if (m_fSpeed < 1.0f) m_fSpeed = 1.0f;

	if (I_Input.GetKey('W') == KEY_HOLD) 		
		MoveLook(g_fSecondPerFrame * 5.0f * m_fSpeed);
	if (I_Input.GetKey('S') == KEY_HOLD)		
		MoveLook(-g_fSecondPerFrame * 5.0f * m_fSpeed);
	if (I_Input.GetKey('D') == KEY_HOLD)		
		MoveSide(g_fSecondPerFrame * 5.0f * m_fSpeed);
	if (I_Input.GetKey('A') == KEY_HOLD)		
		MoveSide(-g_fSecondPerFrame * 5.0f * m_fSpeed);
	if (I_Input.GetKey('Q') == KEY_HOLD)		
		MoveUp(g_fSecondPerFrame * 5.0f * m_fSpeed);
	if (I_Input.GetKey('E') == KEY_HOLD)		
		MoveUp(-g_fSecondPerFrame * 5.0f * m_fSpeed);

	Update(TVector4(m_fCameraPitchAngle, m_fCameraYawAngle, m_fCameraRollAngle, fDistance));
	UpdateVector();
	return true;
}

bool Camera::Release()
{
	return true;
}

Camera::Camera()
{
	// 추가
	m_vPosDelta = TVector3(0.0f, 0.0f, 0.0f);

	m_fDefaultRadius = 0.0f;
	m_fMinRadius = 1.0f;
	m_fMaxRadius = FLT_MAX;
	m_fRadius = 0.0f;

	m_fCameraYawAngle = 0.0f;
	m_fCameraPitchAngle = 0.0f;
	m_fCameraRollAngle = 0.0f;

	m_fSpeed = 0.0f;

	// 추가
	m_nMouseWheelDelta = 0;

	m_vModelCenter = TVector3(0, 0, 0);

	D3DXMatrixIdentity(&m_matWorld);
	D3DXMatrixIdentity(&m_mModelRot);
	D3DXMatrixIdentity(&m_mModelLastRot);
	D3DXMatrixIdentity(&m_mCameraRotLast);

	m_nRotateModelButtonMask = MOUSE_LEFT_BUTTON;
	m_nZoomButtonMask = MOUSE_WHEEL;
	m_nRotateCameraButtonMask = MOUSE_RIGHT_BUTTON;


	D3DXMatrixIdentity(&m_matWorld);
	//CreateProjMatrix(XM_PI / 4.0f, 1.0f, 0.1f, 1000.0f);
	CreateViewMatrix(TVector3(0.0f, 0.0f, 10.0f), TVector3(0.0f, 0.0f, 0.0f), TVector3(0.0f, 1.0f, 0.0f));

	InitHash(100);
}

Camera::~Camera()
{

}

void Camera::MoveLook(float fValue)
{
	m_vPos += m_vLook * fValue;
}
void Camera::MoveSide(float fValue)
{
	m_vPos += m_vRight * fValue;
}
void Camera::MoveUp(float fValue)
{
	m_vPos += m_vUp * fValue;
}

void Camera::SetRadius(float fDefaultRadius, float fMinRadius, float fMaxRadius)
{
	m_fDefaultRadius = m_fRadius = fDefaultRadius;
	m_fMinRadius = fMinRadius;
	m_fMaxRadius = fMaxRadius;
}
void Camera::SetWindow(int nWidth, int nHeight, float fArcballRadius)
{
	m_WorldArcBall.SetWindow(nWidth, nHeight, fArcballRadius);
	m_ViewArcBall.SetWindow(nWidth, nHeight, fArcballRadius);
}

void Camera::SetModelCenter(TVector3 vModelCenter)
{
	m_vModelCenter = vModelCenter;
}

float Camera::Fade(float t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Camera::Lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

float Camera::Gradient(int hash, float x)
{
	int h = hash & 15;
	float u = h < 8 ? x : -x;
	return u * (h < 4 ? 1.0f : -1.0f);
}

float Camera::PerlinNoise1D(float x)
{
	int X = (int)floor(x) & 255;
	x -= floor(x);
	float u = Fade(x);
	int A = hash[X];
	int B = hash[(X + 1) & 255];
	return Lerp(Gradient(hash[A], x), Gradient(hash[B], x - 1.0f), u);
}
void Camera::CameraShake()
{
	m_fShakeCurrent = 0.0f;
	m_vShakeOriginPos = m_vPos;
}

#include <random>
void Camera::InitHash(int seed)
{
	std::mt19937 gen(seed);
	std::uniform_int_distribution<int> dist(0, 255);
	for (int i = 0; i < 256; ++i)
	{
		hash[i] = dist(gen);
	}
}

void Camera::UpdateCameraShake()
{
	if (m_fShakeCurrent < m_fShakeDuration)
	{
		float shakeFactor = 1.0f - (m_fShakeCurrent / m_fShakeDuration);
		float offsetX = PerlinNoise1D(m_fShakeCurrent * m_fShakeFrequency) * m_fShakeAmplitude * shakeFactor;
		float offsetY = PerlinNoise1D((m_fShakeCurrent + 1000.0f) * m_fShakeFrequency) * m_fShakeAmplitude * shakeFactor;
		TVector3 noisePos(offsetX, offsetY, 0.0f);
		m_vPos += noisePos;
		m_fShakeCurrent += g_fSecondPerFrame;
	}
}

void Camera::CameraClosing(float fDuration)
{
	m_fClosingCurrent = 0.0f;
	m_fClosingDuration = fDuration;
}

void Camera::UpdateCameraClosing()
{
	if (m_fClosingCurrent < m_fClosingDuration)
	{
		TVector3 posOrigin = m_vPos;
		TVector3 vLook = m_vTarget - posOrigin;
		vLook.Normalize();
		m_vPos += vLook * 2.0f;
		vLook = posOrigin - m_vPos;
		m_vPos += vLook * (m_fClosingCurrent / m_fClosingDuration);
		m_fClosingCurrent += g_fSecondPerFrame;
	}
	else
	{
		m_fClosingDuration = 0.5f;
	}
}
