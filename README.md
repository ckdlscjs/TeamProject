# 🎮 LoLShinSouls 핵심 기능 및 소스코드 상세 분석

본 문서는 DirectX 11 API를 기반으로 개발된 액션 RPG 게임 클라이언트(LoLShinSouls)의 핵심 게임 플로우, 카메라, 맵 로드, 포그 셰이딩 및 UI 프레임워크 기능들을 실제 소스코드의 데이터 연산 흐름을 바탕으로 상세하게 분석한 기술 명세서임.

---

## 1. 전반적인 게임 플로우 메인프로그램구성
* **파일명**: [TeamProject/GameLib/GameCore.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameLib/GameCore.cpp) (Run 및 CoreFrame) / [TeamProject/GameProject/MyMain.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameProject/MyMain.cpp) (Frame 및 Render)
* **기능 개요**: Win32 메시지 펌핑과 디바이스 라이프사이클을 조율하여 게임 프레임 무한 루프를 기동하고, 입력, 타이머, 사운드 및 최상위 씬(Scene) 상태 업데이트와 그리기(Render) 흐름을 매 프레임 제어함.
* **코드 상세 분석**:
  - `GameCore::Run()`은 `CoreInit()`를 거쳐 `MyWindows::Run()` 윈도우 메시지 펌핑이 유지되는 동안 매 프레임 `CoreFrame()` 과 `CoreRender()` 를 가동하는 윈도우 루프를 기동함.
  - `CoreFrame()`은 키보드/마우스 입력(`I_Input.Frame()`), 시간 누적(`I_Timer.Frame()`), 텍스트 라이터(`g_pWriter->Frame()`)를 갱신한 뒤, 가상 함수 오버라이딩에 의해 `MyMain::Frame()`을 호출함.
  - `MyMain::Frame()`에서는 포스트 프로세싱 MRT 자원(`m_MRT.Frame()`), 화면 스크린(`m_screen.Frame()`), 그리고 핵심 **`I_Scene.Frame()`** 씬 매니저를 기동해 인게임 월드의 상태 머신을 업데이트함.
  - `MyMain::Render()`에서는 광원별 그림자 깊이 맵 연산(`light->PreRender()`)과 MRT 렌더를 순차 기동하고 최종적으로 현재 활성화된 씬(`I_Scene.Render()`)의 드로우콜을 화면에 렌더링함.

```cpp
// GameCore.cpp: Win32 메시지 펌핑 기반 무한 루프 및 매 프레임 업데이트 기동
bool	GameCore::Run()
{
	CoreInit();

	m_bGameRun = true;
	while (m_bGameRun)
	{
		if (MyWindows::Run())
		{
			if (!CoreFrame() || !CoreRender())
			{
				m_bGameRun = false;
			}
		}
		else
		{
			m_bGameRun = false;
		}
	}

	CoreRelease();

	return true;
}

bool	GameCore::CoreFrame()
{
	I_Input.Frame();
	I_Timer.Frame();
	I_Sound.Frame();
	g_pWriter->Frame();

	return Frame();
}
```
```cpp
// MyMain.cpp: 씬(Scene) 매니저 업데이트 및 MRT 그림자 깊이 렌더링 흐름 조율
bool MyMain::Frame()
{
    if (I_Input.GetKey(VK_ESCAPE) == KEY_PUSH)
        m_bGameRun = false;

    m_MRT.Frame(); 

    m_screen.Frame();
    
    I_Scene.Frame();
    return true;
}

bool MyMain::Render()
{
    {
        auto lights = SSB::I_Light.GetLightList();
        for (auto light : lights)
        {
            light->PreRender();
            I_Scene.PreRender();
        }
    }

    m_MRT.Render();
    I_Scene.Render();

    return true;
}
```

---

## 2. 카메라 기능들
* **파일명**: [TeamProject/GameProject/SpringArmCamera.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameProject/SpringArmCamera.cpp) (CheckIntersectionWithMap) / [TeamProject/GameProject/CameraCinema.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameProject/CameraCinema.cpp) (MoveCameraBezierSpline) / [TeamProject/GameLib/Camera.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameLib/Camera.cpp) (PerlinNoise1D 및 UpdateCameraShake)
* **기능 개요**: 3인칭 뷰 스프링암의 지형 충돌 장애물 회피 검출과 영화적 연출을 위한 n차 베지어 무빙 보간, 타격감 구현을 위한 1D 펄린 노이즈 진동 쉐이크 효과를 담당함.
* **코드 상세 분석**:
  - `CheckIntersectionWithMap()`은 카메라 시선 벡터 레이를 쿼드트리의 가시 리프 노드 OBB 영역과 교차 연산(`OBBtoRay`)하여 1차 선별하고, 해당 노드의 삼각면 정점 좌표(`v0, v1, v2`)를 획득해 삼각형 피킹(`ChkPick`) 검사를 가동해 최단 충돌 거리를 갱신하여 장애물 뒤로 카메라가 가리는 현상을 예방함.
  - `MoveCameraBezierSpline()`은 카메라 컷신 연출을 위한 키프레임 무빙 정보 구조체 배열(`camMoveList`)의 크기로부터 번스타인 기저 함수 가중치를 연산하여 이동 및 회전 궤적을 부드럽게 보간 계산함.
  - `UpdateCameraShake()`는 해시 테이블과 Gradient Fade로 구현된 1D 펄린 노이즈를 기동하여 인과 연속성이 보장되는 화면 진동 오프셋(`noisePos`)을 X, Y축에 가해 화면 흔들림 및 프레임 감쇄를 셋업함.

```cpp
// SpringArmCamera.cpp: 지형 리프 노드 OBB 및 정밀 삼각면 레이 캐스팅 검출
bool SSB::SpringArmCamera::CheckIntersectionWithMap()
{
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(-m_fCameraPitchAngle, m_fCameraYawAngle, 0);
	XMVECTOR DefaultForward = { 0, 0, 1, 0 };
	XMVECTOR camPosition = XMVector3TransformNormal(DefaultForward, camRotationMatrix);
	camPosition = XMVector3Normalize(camPosition);

	m_Select.m_Ray.vOrigin = m_vTarget;
	XMStoreFloat3(&m_Select.m_Ray.vDirection, camPosition);
	m_Select.m_Ray.vDirection.Normalize();

	float distance = _kMaxDistance;
	if (Player::GetInstance().IsUltimateSkill())
	{
		distance = 30;
	}

	for (auto node : _map->m_pDrawLeafNodeList)
	{
		if (m_Select.OBBtoRay(&node->m_Box))
		{
			if (node->m_Box.vMin.x > m_vPos.x || node->m_Box.vMax.x < m_vPos.x || node->m_Box.vMin.z > m_vPos.z || node->m_Box.vMax.z < m_vPos.z)
				continue;
			UINT index = 0;
			UINT iNumFace = node->m_IndexList.size() / 3;
			for (UINT face = 0; face < iNumFace; face++)
			{
				UINT i0 = node->m_IndexList[index + 0];
				UINT i1 = node->m_IndexList[index + 1];
				UINT i2 = node->m_IndexList[index + 2];
				TVector3 v0 = _map->m_pMap->m_ListVertex[i0].pos;
				TVector3 v1 = _map->m_pMap->m_ListVertex[i1].pos;
				TVector3 v2 = _map->m_pMap->m_ListVertex[i2].pos;
				if (m_Select.ChkPick(v0, v1, v2, distance))
				{
					return true;
				}
				index += 3;
			}
		}
	}
	return false;
}
```
```cpp
// CameraCinema.cpp: n차 베지어 곡선 공식을 활용한 카메라 위치 및 시선방향 보간
void CameraCinema::MoveCameraBezierSpline(float time, float duration, std::vector<CameraMove>& camMoveList, XMFLOAT3& getPos, XMFLOAT3& getDir)
{
	float t = time / duration;
	int n = static_cast<int>(camMoveList.size()) - 1;
	float u = 1.0f - t;
	float un = pow(u, n);
	float tn = pow(t, n);
	XMFLOAT3 pos(0, 0, 0);
	XMFLOAT3 dir(0, 0, 0);
	for (int i = 0; i <= n; ++i)
	{
		float binomial = BinomialCoefficient(n, i);
		float basis = binomial * pow(u, n - i) * pow(t, i);
		pos.x += basis * camMoveList[i].camPos.x;
		pos.y += basis * camMoveList[i].camPos.y;
		pos.z += basis * camMoveList[i].camPos.z;

		dir.x += basis * camMoveList[i].fYaw;
		dir.y += basis * camMoveList[i].fPitch;
		dir.z += basis * camMoveList[i].fRoll;
	}

	getPos = pos;
	getDir = dir;
}
```
```cpp
// Camera.cpp: 1D 펄린 노이즈를 기동한 프레임 쉐이크 갱신
float Camera::PerlinNoise1D(float x)
{
	int X = (int)floor(x) & 255;
	x -= floor(x);
	float u = Fade(x);
	int A = hash[X];
	int B = hash[(X + 1) & 255];
	return Lerp(Gradient(hash[A], x), Gradient(hash[B], x - 1.0f), u);
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
```

---

## 3. 맵 로드 및 구성
* **파일명**: [TeamProject/GameProject/FQuadTree.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameProject/FQuadTree.cpp) (OpenMap)
* **기능 개요**: 맵 에디터에서 익스포트해둔 텍스트 형식의 씬 배치 설정 파일을 런타임 게임 클라이언트가 시작 시 직접 줄 단위 독해하여 지형 메쉬 및 다중 스플래팅 디테일 텍스처 자원을 구성함.
* **코드 상세 분석**:
  - `std::ifstream` 스트림을 개방해 맵 데이터 파일(`szFullPath`)을 읽고 `std::getline` 및 문자열 구분 파싱을 실행함.
  - 베이스 지형 텍스처 명세(`m_pTexture`)와 쉼표(`,`)로 구분된 4중 스플래팅 텍스처 명세(`m_ListTextureSplatting`)를 색출하고, 텍스처 매니저(`I_Tex.Load`)를 기동하여 랜드스케이프 지형의 렌더링 텍스처 목록을 동적으로 복구 셋업함.

```cpp
// FQuadTree.cpp: 직렬화 씬 파일 로드를 통한 랜드스케이프 및 텍스처 자원 복원
FQuadTree* OpenMap(std::wstring szFullPath, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
	Texture* pTexture = nullptr;
	std::vector<Texture*> pTexList;
	Transform mapTransform = {};
	UINT iMaxDepth = 0;
	std::wstring szVSPath;
	std::wstring szPSPath;
	std::vector<Cinema> CinemaList;
	std::vector<InstanceData> InstanceList;
	MeshMap* pMapMesh = new MeshMap();
	pMapMesh->SetDevice(pd3dDevice, pContext);
	std::vector<std::pair<std::string, Transform>> spawnList;
	std::map<std::wstring, T_BOX> triggerList;
	std::unordered_set<Object*> allObjectList;
	std::unordered_set<Object*> allInstnaceObjectList;
	BYTE* fAlphaData = nullptr;
	std::ifstream is(szFullPath);
	std::string line;

	BaseObject* pSphereObject = nullptr;

	while (std::getline(is, line))
	{
		std::istringstream iss(line);
		std::string fieldName;
		if (std::getline(iss, fieldName, ':'))
		{
			if (fieldName == "m_pTexture")
			{
				std::string textureName;
				iss >> textureName;
				PathChanger(textureName);
				I_Tex.Load(to_mw(textureName), &pTexture);
			}
			else if (fieldName == "m_ListTextureSplatting")
			{
				std::string texturesStr;
				std::getline(iss, texturesStr);
				std::stringstream texturesStream(texturesStr);
				std::string texturePath;
				while (std::getline(texturesStream, texturePath, ','))
				{
					if (texturePath.size() > 1)
					{
						texturePath.erase(std::remove(texturePath.begin(), texturePath.end(), ' '), texturePath.end());
						Texture* texture;
						PathChanger(texturePath);
						I_Tex.Load(to_mw(texturePath), &texture);
						pTexList.push_back(texture);
					}
				}
			}
			else if (fieldName == "m_Transform")
			{
				iss >> mapTransform;
			}
			else if (fieldName == "m_iMaxDepth")
			{
				iss >> iMaxDepth;
			}
            // ... 생략 ...
		}
	}
}
```

---

## 4. 안개 (Fog)
* **파일명**: [TeamProject/GameProject/MyMain.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameProject/MyMain.cpp) (안개 변수 정의) / [data/shader/SSB/ScreenShader.hlsl](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/data/shader/SSB/ScreenShader.hlsl) (안개 합성 포스트 프로세싱 픽셀 셰이더)
* **기능 개요**: 화면 전체 렌더 포스트 프로세싱(Post-processing) 단계에서 3차원 월드 공간의 깊이/위치 버퍼 정보를 복원하여, 카메라와 픽셀 간의 거리에 따른 선형 및 지수 안개(Fog) 효과를 실시간 혼합 연산함.
* **코드 상세 분석**:
  - `MyMain.cpp`에 안개의 시작 거리(`g_fFogStart = 30.0f`), 끝 거리(`g_fFogEnd = 200.0f`), 그리고 안개 밀도(`g_fFogDensity = 0.001f`)를 전역 상수 정의함.
  - 포스트 프로세싱 셰이더인 `ScreenShader.hlsl` 내부에서 깊이 맵(`PositionMap`)을 샘플링하여 픽셀의 3D 월드 좌표(`pos`)를 획득하고, 카메라와 픽셀 간의 실제 3D 거리(`fogDist = distance(currentCameraPos, pos)`)를 계산함.
  - **선형 안개 강도**: `saturate((fogDist - linearFogStart) / (linearFogEnd - linearFogStart))`를 구함.
  - **지수 안개 강도**: `exp(-fogDist * expFogDensity)`를 산출하여 안개 가중치(`fogAmount`)로 픽셀 색상과 회색 안개색을 보간 합성(`lerp(ret, fogColor, fogAmount)`) 렌더링함.

```hlsl
// ScreenShader.hlsl: 깊이/위치 맵을 이용한 선형 및 지수 안개(Fog) 실시간 혼합 셰이딩
cbuffer constant : register(b11)
{
	float4 currentCameraPos;
	float linearFogStart;
	float linearFogEnd;
	float expFogDensity;
};

// 픽셀 셰이더 내부 안개 연산 구절
float4 pos = PositionMap.Sample(Sampler, input.TextureUV);
float fogDist = distance(currentCameraPos, pos);
float linearFogAmount = saturate((fogDist - linearFogStart) / (linearFogEnd - linearFogStart));

float expFogAmount = exp(-fogDist * expFogDensity);

float4 fogColor = float4(0.5f, 0.5f, 0.5f, 0.2f);

float fogAmount = lerp(linearFogAmount, expFogAmount, 0.0f);

float4 diffuseColor = ColorMap.Sample(Sampler, input.TextureUV);

float4 ret = diffuseColor * GetAmbient();
ret += diffuseColor * GetShadowRatio(input.TextureUV) * (GetDiffuse(input.TextureUV) + GetSpecular(input.TextureUV));

ret = lerp(ret, fogColor, fogAmount);
```

---

## 5. UI 전반
* **파일명**: [TeamProject/GameLib/Interface.cpp](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameLib/Interface.cpp) (Interface::Frame 및 파생 클래스들의 Frame / Render) / [TeamProject/GameLib/Interface.h](https://github.com/ckdlscjs/TeamProject/blob/master/TeamProject/GameLib/Interface.h)
* **기능 개요**: 계층적 트리 구조 배치 기반의 UI 프레임워크 상에서, 각각 독립된 런타임 수명 주기를 갖는 애니메이션/트윈 컴포넌트(`InterfaceWork`)를 가동하여 클릭 변동 스케일 모션, 페이딩, 게이지바 보간, 플로팅 텍스트 렌더링, 시계 방향 스킬 쿨타임 마스크 연출 등을 일괄 동기화 처리함.
* **코드 상세 분석**:
  - **UI 구조 트리 전파 (`Interface::Frame`)**:
    부모-자식 트리 구조 UI를 전수 순회하며 비동기 작업 수명이 끝난 완료 객체(`work->m_isDone`)를 런타임 메모리에서 안전하게 파괴(`delete`) 해제하고, 하위 자식 목록(`m_pChildList`)에 대해 갱신 프레임(`data->Frame()`)을 재귀 전파시켜 위치 좌표 및 알파를 일괄 조율함.
  - **마우스 호버/클릭 상태 동적 스케일 (`InterfaceClick`)**:
    마우스 포인터 좌표(`I_Input.m_ptPos`)와 UI 사각형 경계(`TRectangle rect`)의 충돌 교차 판정(`rect.IntersectPoint`)을 기동하여, Hover/Push 시 UI 스케일 크기(`m_vScale`)를 1.1배 혹은 0.9배로 부드럽게 증감 변동 연출함.
  - **비동기 HP/게이지바 보간 (`InterfaceSetGage`)**:
    데미지 핏 등의 소요율 `m_fGage` 값에 맞춰 매 프레임 경과 시간 비율 `t = 1.0f - m_fRemain / m_fDuration` 만큼 정점 텍스처 좌표(`t.x`)를 선형 보간하여 HP/Gage바를 매끄럽게 갱신함.
  - **데미지 플로팅 텍스트 드로잉 (`InterfaceDamageFloating`)**:
    타격한 몹의 Damage 수치값을 텍스트 변환하여 매칭 폰트 리스트(`m_Damage`)를 로드하고, 경과 시간에 비례해 위쪽으로 띄워 올리는 변위(`m_fFloatLength * (1.0f - m_fRemain / m_fDuration)`)를 버텍스 리스트 좌표에 누적 적용하여 실시간 동적 플로팅 데미지를 월드에 렌더링함.
  - **시계방향 스킬 쿨타임 각도 파라미터 연동 (`InterfaceFadeClockwise`)**:
    스킬 시전 시 쿨타임 마스크의 각도를 `pInter->m_cbData.fTimer = 360.0f * m_fCurrent / m_fTime` 수식으로 계산하여 상수 버퍼에 탑재함으로써 픽셀 셰이더가 쿨타임 각도를 discard 처리할 수 있도록 변조 각도를 동적 산출함.

```cpp
// Interface.cpp: 등록된 InterfaceWork 목록을 매 프레임 업데이트 및 트리 재귀 처리
bool Interface::Frame()
{
	for (auto iter = m_pWorkList.begin(); iter != m_pWorkList.end();)
	{
		InterfaceWork* work = (*iter);
		if (work->m_isDone)
		{
			iter = m_pWorkList.erase(iter);
			if (work)
			{
				delete work;
				work = nullptr;
			}
			continue;
		}
		else
		{
			work->Frame(this);
			iter++;
		}
	}
	for (auto data : m_pChildList)
	{
		data->Frame();
	}

	return true;
}
```
```cpp
// Interface.h: InterfaceClick, InterfaceDamageFloating 등의 비동기 트윈 기작 C++ 실제 소스코드
class InterfaceClick : public InterfaceWork
{
public:
	bool Frame(Interface* pInter)
	{
		POINT ptMouse = I_Input.m_ptPos;
		pInter->m_vScale = TVector3(m_fScale);

		TVector3 pos = pInter->m_vPos;
		long width = pInter->m_pTexture->m_Desc.Width;
		long height = pInter->m_pTexture->m_Desc.Height;

		TRectangle rect = { (long)pos.x, (long)pos.y, width, height };
		if (rect.IntersectPoint(ptMouse))
		{
			pInter->m_CurrentState = UI_HOVER;
			pInter->m_vScale = TVector3(m_fScale * 1.1f);
			if (I_Input.GetKey(VK_LBUTTON) == KEY_PUSH ||
				I_Input.GetKey(VK_LBUTTON) == KEY_HOLD)
			{
				pInter->m_CurrentState = UI_PUSH;
				pInter->m_vScale = TVector3(m_fScale * 0.9f);
			}
			if (I_Input.GetKey(VK_LBUTTON) == KEY_UP)
			{
				pInter->m_CurrentState = UI_SELECT;
			}
		}
		else
		{
			pInter->m_CurrentState = UI_NORMAL;
		}
		pInter->SetFrame(pInter->m_CurrentState);
		return true;
	}

public:
	InterfaceClick(float fScale)
	{
		m_fScale = fScale;
	}
public:
	float	m_fScale;
};

class InterfaceDamageFloating : public InterfaceWork
{
public:
	bool	Frame(Interface* pInter) override
	{
		m_fRemain -= g_fSecondPerFrame;
		m_fAlpha = m_fRemain / m_fDuration;
		if (m_fRemain <= 0.0f)
		{
			m_isDone = true;
		}
		for (int i = 0; i < pInter->m_VertexList.size(); i++)
		{
			pInter->m_VertexList[i].c.w = m_fAlpha;
		}
		return true;
	}

	bool	Render(Interface* pInter) override
	{
		pInter->m_VertexList[0].p.x = -m_fDamageSize / 2.0f;
		pInter->m_VertexList[0].p.y = m_Damage[0].m_v1 / 2.0f + (m_fFloatLength * (1.0f - m_fRemain / m_fDuration));
		for (int idx = 0; idx < m_Damage.size(); idx++)
		{
			float width = m_Damage[idx].m_u1;
			float height = m_Damage[idx].m_v1;

			pInter->m_VertexList[1].p.x = pInter->m_VertexList[0].p.x + width;
			pInter->m_VertexList[1].p.y = pInter->m_VertexList[0].p.y;

			pInter->m_VertexList[2].p.x = pInter->m_VertexList[0].p.x;
			pInter->m_VertexList[2].p.y = pInter->m_VertexList[0].p.y - height;

			pInter->m_VertexList[3].p.x = pInter->m_VertexList[1].p.x;
			pInter->m_VertexList[3].p.y = pInter->m_VertexList[2].p.y;

			float descWidth = (float)pInter->m_pTexture->m_Desc.Width;
			float descHeight = (float)pInter->m_pTexture->m_Desc.Height;

			pInter->m_VertexList[0].t.x = m_Damage[idx].m_u0 / descWidth;
			pInter->m_VertexList[0].t.y = m_Damage[idx].m_v0 / descHeight;

			pInter->m_VertexList[1].t.x = (m_Damage[idx].m_u0 + m_Damage[idx].m_u1) / descWidth;
			pInter->m_VertexList[1].t.y = pInter->m_VertexList[0].t.y;

			pInter->m_VertexList[2].t.x = pInter->m_VertexList[0].t.x;
			pInter->m_VertexList[2].t.y = (m_Damage[idx].m_v0 + m_Damage[idx].m_v1) / descHeight;

			pInter->m_VertexList[3].t.x = pInter->m_VertexList[1].t.x;
			pInter->m_VertexList[3].t.y = pInter->m_VertexList[2].t.y;

			pInter->UpdateVertexBuffer();
			pInter->UpdateConstantBuffer();
			pInter->Interface::Render();
			pInter->m_VertexList[0].p.x = pInter->m_VertexList[1].p.x;
		}

		return true;
	}
    // ... 생략 ...
};
```
