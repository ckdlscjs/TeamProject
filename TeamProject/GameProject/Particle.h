#pragma once
#include "TMath.h"

// https://copynull.tistory.com/290
// https://copynull.tistory.com/297
// 
// Sprite << �ִϸ��̼�
// �̹̽ú�? < �۷ο� / ����
// 
// ��ġ: 3D �������� ������ ��ġ�Դϴ�.
// �ӵ�: ���ڰ� �����̴� �ӵ��� ����.
// ���ӵ� : ������ �ӵ��� ���ϴ� �ӵ��Դϴ�.
// ���� : ���ڿ� ���Ե� ������ ��.
// ���� : ���ڰ� �ı��Ǳ� ���� �����ϴ� �ð��Դϴ�.
// ũ�� : ���� ������ �Ǵ� ��� ���ڷ� ǥ�õǴ� ������ ũ���Դϴ�.
// ���� : �ð��� ������ ���� �پ��� ȿ���� ���� ���� ���� �� �ִ� ������ �����Դϴ�.
// �ؽ�ó : ��ƼŬ�� �������ϴ� �� ���Ǵ� �̹��� �Ǵ� ��������, �ð��� ������ ���� ����� ���� �ֽ��ϴ�.
// 
// ���ϴ� ���� ���ۿ� ���� ���Ե� �� �ִ� �߰� ���� ��Ҵ� ������ �����ϴ�.
// ȸ�� : ��ü ���� �߽����� �ϴ� ������ ȸ���Դϴ�.
// 
// ȸ�� : ���ڰ� ȸ���ϴ� �ӵ��Դϴ�.
// ���� : ������ �ӵ��� �ð��� ������ ���� �����ϴ� �ӵ��Դϴ�.
// �� : �߷��̳� �ٶ��� ���� ������ �����ӿ� ������ ��ġ�� �ܺ� ��.
// �浹 ���� : ���ڰ� ����� �ٸ� ��ü�� �浹�ϴ� �ñ⸦ �����ϴ� ����Դϴ�.
// 

/*
function getPoint() {
	var u = Math.random();
	var v = Math.random();
	var theta = u * 2.0 * Math.PI;
	var phi = Math.acos(2.0 * v - 1.0);
	var r = Math.cbrt(Math.random());
	var sinTheta = Math.sin(theta);
	var cosTheta = Math.cos(theta);
	var sinPhi = Math.sin(phi);
	var cosPhi = Math.cos(phi);
	var x = r * sinPhi * cosTheta;
	var y = r * sinPhi * sinTheta;
	var z = r * cosPhi;
	return { x: x, y : y, z : z };
}

def random_three_vector() {
	"""
		Generates a random 3D unit vector(direction) with a uniform spherical distribution
		Algo from http ://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution
	: return :
		"""
		phi = np.random.uniform(0, np.pi * 2)
		costheta = np.random.uniform(-1, 1)

		theta = np.arccos(costheta)
		x = np.sin(theta) * np.cos(phi)
		y = np.sin(theta) * np.sin(phi)
		z = np.cos(theta)
		return (x, y, z)
}
*/

// distortion ���̴�
// ���? shape? Ÿ��?
// 
//

struct ParticleSRT
{
	TVector3		vFix;

	TVector3		vPVA;
	TVector3		vCurVelocity;
	TVector3		vVelocity;
	TVector3		vAccel;

	TVector3		vStart;
	TVector3		vEnd;
};
struct ParticleColor
{
	UINT		iColorType;

	TColor	vFix;

	TColor	vRandom;

	TColor	vStart;
	TColor	vEnd;
};
struct ParticleData
{
	UINT				iTransformType[3];
	ParticleSRT	particleSRT[3];

	ParticleColor	ColorData;

	bool				isDestroyLifeTime;
	float				fLifeTime;

	UINT				iBillBoardType;
};

class Particle
{
public:
	Particle();
	~Particle();

	void	Create(ParticleData data);

	bool	Frame(TMatrix* matView);
	void	ProcessColor();
	void	ProcessSRT();
	void	ProcessBillboard();

	// ��ƼŬ ����
	//z_rollȸ������
	//��ƼŬ��ġ�� ������ġ�� ����Ұ��ΰ�
	//�׳�Loop(����)�ΰ�, Ƚ�������ݺ��ΰ�
	//����Ʈ ��������Ʈ�ǰ�� Ư�� �ε����� �����Ұ��ΰ�
	//������ ���ʵں��� ����Ʈ�����۵ɰ��ΰ� delaytime
	//����Ʈ�� dx11����ɼ�, ���ó���Ұ��ΰ�
	// 
	
	TVector3		m_vPos;
	TVector3		m_vScale;
	TVector3		m_vRotation;

	TColor			m_vCurColor;

	UINT				m_iTransformType[3];
	ParticleSRT	m_ParticleSRT[3];
	ParticleColor	m_ParticleColor;

	bool				m_isDestroyLifeTime;
	bool				m_isDone;
	float				m_fCurLife;
	float				m_fLifeTime;

	UINT				m_iBillBoardType;

public:
	TMatrix			m_matWorld;
	TMatrix			m_matView;

	TMatrix			m_matRotation;
	TMatrix			m_matBillboard;
};

