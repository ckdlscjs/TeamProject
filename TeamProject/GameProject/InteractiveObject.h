#pragma once
#include "TestObj.h"
class InteractiveObject
{
	// �浹ü
	// �浹 + Ű �Է½� �߻� �̺�Ʈ - string, enum, 
	// ��ȣ�ۿ�� ��ȭ�� ���� ������Ʈ�� ����Ʈ�� �־���ҵ�
	// 
	// ��ȣ�ۿ� ������Ʈ���� �������� ���ƾ��ϳ�? - (������Ʈ�� �������� �����Ǹ鼭 �����δٸ� �ʿ��ҵ�)
	// �������� ���ƾ��Ѵٸ� ����Ʈ�� ��𼱰� ��� �־����
	// �ݺ� ��ȣ�ۿ��� �����Ѱ� / �ܹ߼����� ������°�
	// 
	// ������Ʈ�� ������ ����̳� �Լ�
	// �����δٸ� ���ʵ����� ��������
	// 
	// ��� 1)
public:
	void	SetSwitch(bool* b);
	void	Switching();

public:
	bool* m_bSwitch;

	// ��� 2)
public:
	void	SetObject(TestObj* obj);
	void	Interacted();

public:
	TestObj* m_pObj;
};

