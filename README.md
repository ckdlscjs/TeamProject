# DirectX 3D Team Project : LolShinSouls

## Keyword : Deffered Rendering, FSM, Collision Process, Edge Process

![234498154-ffaa7cd6-9bbb-4f14-b423-c42b302dc000.PNG](https://postfiles.pstatic.net/MjAyNDAyMDlfMTkx/MDAxNzA3NDczODQ0Mzg3.R-OnPPhk45OirD-AtXJuGcIA9oFXtQXoYxDBV6-xBKcg.68hipY759BXn9Z5ya-fWH7sv6OXCKpR8jlahN_cuqHMg.PNG.sinsin63/234498154-ffaa7cd6-9bbb-4f14-b423-c42b302dc000.png?type=w580)

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
![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/2c037e11-5d69-451f-8557-93550e6a2b52/d5b82d7e-a766-4e29-959d-a01094485dc8/Untitled.png)
- **MapTool, Map**
    - **LandScape**
        - **Frustum Culling**을 통한 Rendering최적화
        - **Collider ObbBox, FbxMesh**의 **RayCast**를 이용한 충돌 구현
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
          
 ![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/2c037e11-5d69-451f-8557-93550e6a2b52/b4085bb7-0061-40c1-ac4a-3217cde50c1e/Untitled.png)
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

[https://youtu.be/X2dgJuERe9M](https://youtu.be/X2dgJuERe9M)

## 세부사항

[LolShinSoul](https://www.notion.so/LolShinSoul-1cb2b2b24e2d4a3abfb6a0d506a08c39?pvs=21)
