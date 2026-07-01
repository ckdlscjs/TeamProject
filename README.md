# 🎮 LoLShinSouls 핵심 기능 및 소스코드 상세 분석

---

## 1. 전반적인 게임 플로우 메인프로그램구성
* **파일명**: `TeamProject/GameLib/GameCore.cpp` (`Run` 및 `CoreFrame`) / `TeamProject/GameProject/MyMain.cpp` (`Frame` 및 `Render`)
* **기능 개요**: Win32 메시지 펌핑과 디바이스 라이프사이클을 조율하여 게임 프레임 무한 루프를 기동하고, 입력, 타이머, 사운드 및 최상위 씬(Scene) 상태 업데이트와 그리기(Render) 흐름을 매 프레임 제어함.
* **코드 상세 분석**:
  - `GameCore::Run()`은 `CoreInit()`를 거쳐 `MyWindows::Run()` 윈도우 메시지 펌핑이 유지되는 동안 매 프레임 `CoreFrame()` 과 `CoreRender()` 를 가동하는 윈도우 루프를 기동함.
  - `CoreFrame()`은 키보드/마우스 입력(`I_Input.Frame()`), 시간 누적(`I_Timer.Frame()`), 텍스트 라이터(`g_pWriter->Frame()`)를 갱신한 뒤, 가상 함수 오버라이딩에 의해 `MyMain::Frame()`을 호출함.
  - `MyMain::Frame()`에서는 포스트 프로세싱 MRT 자원(`m_MRT.Frame()`), 화면 스크린(`m_screen.Frame()`)을 처리하고, 씬 매니저인 `I_Scene.Frame()`을 기동해 인게임 월드의 상태 머신을 업데이트함.
  - `MyMain::Render()`에서는 광원별 그림자 깊이 맵 연산(`light->PreRender()`)과 MRT 렌더링을 순차 기동하고 최종적으로 현재 활성화된 씬(`I_Scene.Render()`)의 드로우콜을 화면에 렌더링함.

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
bool    MyMain::Frame()
{
    if (I_Input.GetKey(VK_ESCAPE) == KEY_PUSH)
        m_bGameRun = false;

    m_MRT.Frame(); 

    m_screen.Frame();
    
    I_Scene.Frame();
    return true;
}

bool    MyMain::Render()
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
* **파일명**: `TeamProject/GameProject/SpringArmCamera.cpp` (`CheckIntersectionWithMap`) / `TeamProject/GameProject/CameraCinema.cpp` (`MoveCameraBezierSpline`) / `TeamProject/GameLib/Camera.cpp` (`PerlinNoise1D` 및 `UpdateCameraShake`)
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
* **파일명**: `TeamProject/GameProject/FQuadTree.cpp` (`OpenMap`)
* **기능 개요**: 맵 에디터에서 익스포트해둔 텍스트 형식의 씬 설정 파일을 런타임 게임 클라이언트가 시작 시 직접 줄 단위 독해하여 지형 메쉬 및 다중 스플래팅 디테일 텍스처 자원을 구성함.
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
* **파일명**: `TeamProject/GameProject/MyMain.cpp` (안개 변수 정의) / `data/shader/SSB/ScreenShader.hlsl` (안개 합성 포스트 프로세싱 픽셀 셰이더)
* **기능 개요**: 화면 전체 렌더 포스트 프로세싱(Post-processing) 단계에서 3차원 월드 공간의 깊이/위치 버퍼 정보를 복원하여, 카메라와 픽셀 간의 거리에 따른 선형 및 지수 안개(Fog) 효과를 실시간 혼합 연산함.
* **코드 상세 분석**:
  - `MyMain.cpp`에 안개의 시작 거리(`g_fFogStart = 30.0f`), 끝 거리(`g_fFogEnd = 200.0f`), 그리고 안개 밀도(`g_fFogDensity = 0.001f`)를 전역 상수 정의함.
  - 포스트 프로세싱 셰이더인 `ScreenShader.hlsl` 내부에서 깊이 맵(`PositionMap`)을 샘플링하여 픽셀의 3D 월드 좌표(`pos`)를 획득하고, 카메라와 픽셀 간의 실제 3D 거리(`fogDist = distance(currentCameraPos, pos)`)를 계산함.
  - **선형 안개 강도**: `saturate((fogDist - linearFogStart) / (linearFogEnd - linearFogStart))` 수식을 기동함.
  - **지수 안개 강도**: `exp(-fogDist * expFogDensity)` 수식을 기동하여 렌더링된 픽셀 색상(`ret`)과 안개 색상(`fogColor`)을 최종 가중치(`fogAmount`)로 보간 혼합(`lerp(ret, fogColor, fogAmount)`) 처리함.

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
* **파일명**: `TeamProject/GameLib/Interface.cpp` (`Interface::Frame`)
* **기능 개요**: 다중 인게임 UI의 좌표 알파 변조 및 수명 상태 조율을 트리 구조(Tree Structure) 계층으로 제어하고, 독립된 기능별 비동기 업데이트 루프를 지닌 UI 프레임워크 라이프사이클을 기동함.
* **코드 상세 분석**:
  - `Interface::Frame()`은 현재 등록된 비동기 UI 연출 작업(`m_pWorkList`) 목록을 전수 검사하며, 실행 완료된 상태(`work->m_isDone`)인 객체들을 에디터/클라이언트 런타임 메모리에서 제거(`delete`)함.
  - 활성화 상태인 작업들에 대해 `work->Frame(this)`을 기동하여 개별 연출을 먹이고, 자식 UI 리스트(`m_pChildList`)에 대해 순차적으로 업데이트 루프(`data->Frame()`)를 재귀 전파시켜 일괄 트랜스폼 정합을 처리함.

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
