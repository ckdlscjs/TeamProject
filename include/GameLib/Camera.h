#pragma once
#include "Frustum.h"
#include "ArcBall.h"

#define MOUSE_LEFT_BUTTON   0x01
#define MOUSE_MIDDLE_BUTTON 0x02
#define MOUSE_RIGHT_BUTTON  0x04
#define MOUSE_WHEEL         0x08

class Camera
{
public:
	ArcBall			m_WorldArcBall;
	ArcBall			m_ViewArcBall;
	//--------------------------------------------------------------------------------------
	// ��� ����
	//--------------------------------------------------------------------------------------
	TVector3			m_vModelCenter;
	TMatrix			m_mModelLastRot;        // Last arcball rotation matrix for model 
	TMatrix			m_mModelRot;            // Rotation matrix of model
	TMatrix			m_mCameraRotLast;
public:
	Frustum m_vFrustum;

	//--------------------------------------------------------------------------------------
	// ī�޶� ���⺤�� �� ��ġ / Ÿ�� ����	
	//--------------------------------------------------------------------------------------
	TVector3		m_vPos;
	TVector3		m_vLook; // Z
	TVector3		m_vUp = { 0, 1, 0 }; // Y
	TVector3		m_vRight; // X
	TVector3		m_vTarget = { 0, 0, 0 };
	TVector3			m_vDefaultEye;          // Default camera eye position
	TVector3			m_vDefaultLookAt;       // Default LookAt position

	//--------------------------------------------------------------------------------------
	// �� ȸ���� ���� ���ʹϾ�
	//--------------------------------------------------------------------------------------
	TQuaternion		m_qRotation;
	//--------------------------------------------------------------------------------------
	// �� �࿡ ���� ������ w=������( �������κ����� �Ÿ� )
	//--------------------------------------------------------------------------------------
	TVector3			m_vPosDelta;
	//--------------------------------------------------------------------------------------
	// �� ��� �� ���� ���
	//--------------------------------------------------------------------------------------
	TMatrix		m_matWorld;
	TMatrix		m_matView;
	TMatrix		m_matProj;

	TVector3		m_vRotation = { 0, 0, 0 };
	float			m_fDistance = 0.0f;
	float			m_fSpeed = 10.0f;

	float	m_fNear;
	float	m_fFar;
	float	m_fFovY;
	float	m_fAspectRatio;

	// �߰� : ���Ϸ� �� ���� 	
	float				m_fCameraYawAngle;
	float				m_fCameraPitchAngle;
	float				m_fCameraRollAngle;
	// �߰� : ������( �ʱ� �������κ����� �Ÿ�) �� ���� ���� 	
	float				m_fRadius;
	float				m_fDefaultRadius;       // Distance from the camera to model 
	float				m_fMinRadius;           // Min radius
	float				m_fMaxRadius;           // Max radius
	//--------------------------------------------------------------------------------------
	// ���콺 �� ��ư�� ��뿩�θ� �Ǵ��ϴ� ����ũ ���� ���콺 �� ������
	//--------------------------------------------------------------------------------------	
	int					m_nMouseWheelDelta;     // Amount of middle wheel scroll (+/-) 
	int					m_nRotateModelButtonMask;
	int					m_nZoomButtonMask;
	int					m_nRotateCameraButtonMask;

public:
	virtual void CreateViewMatrix(TVector3 vEye, TVector3 vAt, TVector3 vUp);
	virtual void CreateProjMatrix(float fNear, float fFar, float fFovY, float fAspectRatio);
	virtual bool Frame();
	virtual bool Release();
	virtual void UpdateVector();

	virtual void Update(TVector4 vDirValue);

	void	SetRadius(float fDefaultRadius = 5.0f, float fMinRadius = 1.0f, float fMaxRadius = FLT_MAX);

	void	SetObjectView(TVector3 vMax, TVector3 vMin);


	virtual void				MoveLook(float fValue);
	virtual void				MoveSide(float fValue);
	virtual void				MoveUp(float fValue);


	// ȸ�� �� �̵��� ������ ��ư ���� 
	virtual void				SetButtonMasks(int nRotateModelButtonMask, int nZoomButtonMask, int nRotateCameraButtonMask);
	//--------------------------------------------------------------------------------------
	// ������ ����� ������ �ݿ��Ѵ�. 
	//--------------------------------------------------------------------------------------
	virtual void				SetWindow(int nWidth, int nHeight, float fArcballRadius = 0.9f);
	//--------------------------------------------------------------------------------------
	// ������ �޼����� ����Ͽ� ī�޶� ���� 
	//--------------------------------------------------------------------------------------
	virtual int					WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//--------------------------------------------------------------------------------------
	// �� ȸ�� �߽���ġ�� �����Ѵ�.
	//--------------------------------------------------------------------------------------
	void						SetModelCenter(TVector3 vModelCenter);

public:
	Camera();
	virtual ~Camera();
};

