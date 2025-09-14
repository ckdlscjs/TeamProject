# DirectX 3D Team Project : LolShinSouls
![dx_4](https://github.com/ckdlscjs/TeamProject/assets/41976800/ffcd1bc5-122f-40b8-a9c3-d4287dbf065f)
## 개요
- 장르
    - 3D RPG
- 제작 인원
    - 3명
- 제작 기간
    - 2달

## 기술 스택
- C++
- DirectX
- FBX SDK
- FMOD

## 역할
![Untitled](https://github.com/ckdlscjs/TeamProject/assets/41976800/a4ed9c9e-fc8b-4c57-b4db-3d61b6a8cb14)
- **MapTool, Map**
    - **LandScape**
        - **Frustum Culling**을 통한 Rendering최적화
```c++
F_POSITION Frustum::ClassifyPoint(XMVECTOR v)
{
	for (int iPlane = 0; iPlane < 6; iPlane++)
	{
		float fDistance =
			m_Plane[iPlane].a * XMVectorGetX(v) +
			m_Plane[iPlane].b * XMVectorGetY(v) +
			m_Plane[iPlane].c * XMVectorGetZ(v) +
			m_Plane[iPlane].d;
		if (fDistance == 0) return F_ONPLANE;
		if (fDistance < 0) return F_FRONT;
	}
	return F_BACK;
}

F_POSITION Frustum::ClassifyBox(T_BOX v)
{
	float		fPlaneToCenter = 0.0;
	float		fDistance = 0.0f;
	XMFLOAT3	vDir;
	F_POSITION  t_Position;

	t_Position = F_SPANNING;
	for (int iPlane = 0; iPlane < 6; iPlane++)
	{
		vDir = v.vAxis[0] * v.fExtent[0];
		fDistance = fabs(m_Plane[iPlane].a * vDir.x + m_Plane[iPlane].b * vDir.y + m_Plane[iPlane].c * vDir.z);
		vDir = v.vAxis[1] * v.fExtent[1];
		fDistance += fabs(m_Plane[iPlane].a * vDir.x + m_Plane[iPlane].b * vDir.y + m_Plane[iPlane].c * vDir.z);
		vDir = v.vAxis[2] * v.fExtent[2];
		fDistance += fabs(m_Plane[iPlane].a * vDir.x + m_Plane[iPlane].b * vDir.y + m_Plane[iPlane].c * vDir.z);

		fPlaneToCenter = m_Plane[iPlane].a * v.vCenter.x + m_Plane[iPlane].b * v.vCenter.y +
			m_Plane[iPlane].c * v.vCenter.z + m_Plane[iPlane].d;

		if (fPlaneToCenter <= -fDistance) return F_BACK;
	}
	return t_Position;
}

- **Collider ObbBox, FbxMesh**의 **RayCast**를 이용한 충돌 구현
 ```c++
bool TCollision::IntersectRayToOBB(const XMVECTOR& rayOrigin, const XMVECTOR& rayDirection, const T_BOX& obb, float& dist)
{
    // Calculate the center and extent of the OBB
    XMVECTOR vCenter = XMLoadFloat3(&obb.vCenter);
    XMVECTOR vExtent = XMVectorSet(obb.fExtent[0], obb.fExtent[1], obb.fExtent[2], 0.0f);

    // Calculate the ray origin in local space of the OBB
    XMVECTOR vRayOriginLocal = rayOrigin - vCenter;

    // Calculate the inverse of the OBB's world matrix
    XMMATRIX matWorld = XMMatrixIdentity();
    matWorld.r[0] = XMLoadFloat3(&obb.vAxis[0]);
    matWorld.r[1] = XMLoadFloat3(&obb.vAxis[1]);
    matWorld.r[2] = XMLoadFloat3(&obb.vAxis[2]);
    matWorld.r[3] = vCenter;
    XMMATRIX matWorldInverse = XMMatrixInverse(nullptr, matWorld);

    // Transform the ray to local space of the OBB
    XMVECTOR vRayDirectionLocal = XMVector3Normalize(XMVector3TransformNormal(rayDirection, matWorldInverse));
    XMVECTOR vRayOriginLocalTransformed = XMVector3TransformCoord(vRayOriginLocal, matWorldInverse);

    // Perform ray-OBB intersection test
    float tmin = -XMVectorGetX(vExtent);
    float tmax = XMVectorGetX(vExtent);
    for (int i = 0; i < 3; ++i) {
        float e = XMVectorGetByIndex(vExtent, i);
        float d = XMVectorGetByIndex(vRayDirectionLocal, i);
        float o = XMVectorGetByIndex(vRayOriginLocalTransformed, i);

        if (fabsf(d) > FLT_EPSILON) {
            float t1 = (tmin - o) / d;
            float t2 = (tmax - o) / d;
            if (t1 > t2) std::swap(t1, t2);
            if (t1 > -e) tmin = t1;
            if (t2 < e) tmax = t2;
            if (tmin > tmax) return false;
        }
        else if (-o > e || o > e) {
            return false;
        }
    }

    // Store the intersection distance
    dist = tmin;

    return true;
}
 ```

    - **FBXObject**
        - FBXSDK를 통해 리소스를 로드, **Vertex를 Index**로 구축하는 알고리즘을 구현해 최적화
        - 로드된 객체는 **Singleton Manager**에서 관리
    - **SkyDome**
        - 입력받은 Slice, Stack 값을 통한 알고리즘을 이용해 SkyDome을 구현
    - **Camera**
        - **n차 VezierSpline**을 통해 CinemaCamera를 구현
        - 펄린노이즈값을 이용해 Amplitue, Frequency을 이용하는 CameraShake구현
    - **Fog**
        - 플레이어의 좌표를 기준으로 하는 거리Fog 구현, 이를 프로젝트 렌더링 엔진의
        **MRT에 사용하여 최종 SRV**에 안개 적용
          
 ![Untitled2](https://github.com/ckdlscjs/TeamProject/assets/41976800/a7b14cf1-81f3-4bc0-afc0-592d8eab7d8e)
- **UI**
    - **Fade In, Out / Gage**
        - Frame당 **Vertex의 R,G,B,A** 값을 조절해 Shader에서 사용
    - **Billboard**
        - 투영행렬을 사용하는 **UI를 빌보드화** 하여 Damage, Gage 등에 사용
    - **Text**
        - DXWrite를 이용, Scenario등 텍스트를 띄우고 Fade In, Out을 같이 사용
    - **ClockWiseFade**
        - PixelShader내에서 ConstantBuffer에서 넘어온 **Degree값을 기반으로 atan**를 
        이용해 계산하여 시계방향 으로 Fade되는 효과를 구현
    - **MiniMap**
        - 추가 카메라를 통해 별도의 **RTT, ViewPort를 이용해 렌더링** 플레이어와 Enemy는
        GeometryShader를 이용한 Box로 Render
    - **AlphaMapBlur**
        - 가우시안값, AlphaMapWeight을 통해 **가중치에 따른 SRV의 Blur** 구현

## 동영상
https://youtu.be/4ECHTmTD6h4?si=qe3qtL75Kr7zCrz7 (LoLShinSouls) </br>
https://youtu.be/_ik09B4ptmo?si=l5gQW1pp_IqxO2Hu (MapTools)
## 다운로드
http://naver.me/55RIqKpf (게임)
