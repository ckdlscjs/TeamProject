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
- **MapTool, Map**(https://github.com/ckdlscjs/ToolEngine)
    - **LandScape**
    - **Frustum Culling**을 통한 Rendering최적화
<img width="912" height="471" alt="image" src="https://github.com/user-attachments/assets/a3a30846-dc8b-42d3-9b72-02f1e645cfe0" />
<img width="768" height="412" alt="image" src="https://github.com/user-attachments/assets/1b632213-a2b5-4b55-a27d-f9c81f39c37c" />

	- **Collider ObbBox, FbxMesh**의 **RayCast**를 이용한 충돌 구현
<img width="876" height="575" alt="image" src="https://github.com/user-attachments/assets/c58ad68a-fdd4-4395-86c5-1d4f555e8dd5" />
<img width="1206" height="512" alt="image" src="https://github.com/user-attachments/assets/83606d7a-d3e1-4c86-8376-9e114768580a" />

    - **FBXObject**
        - FBXSDK를 통해 리소스를 로드, **Vertex를 Index**로 구축하는 알고리즘을 구현해 최적화
        - 로드된 객체는 **Singleton Manager**에서 관리
<img width="933" height="576" alt="image" src="https://github.com/user-attachments/assets/e387a2f8-1853-431d-ad05-7b2036f8fa23" />

    - **SkyDome**
        - 입력받은 Slice, Stack 값을 통한 알고리즘을 이용해 SkyDome을 구현
<img width="707" height="576" alt="image" src="https://github.com/user-attachments/assets/5d7ca4ea-34af-46ef-b8ce-41e5f36582b5" />

    - **Camera**
    	- **n차 VezierSpline**을 통해 CinemaCamera를 구현
      	- 펄린노이즈값을 이용해 Amplitue, Frequency을 이용하는 CameraShake구현
<img width="1154" height="552" alt="image" src="https://github.com/user-attachments/assets/d217558e-5027-42cc-ac87-c3a91b575c5f" />
<img width="518" height="547" alt="image" src="https://github.com/user-attachments/assets/58dbba20-14fd-442b-94e5-bf060fd11a2e" />

    - **Fog**
        - 플레이어의 좌표를 기준으로 하는 거리Fog 구현, 이를 프로젝트 렌더링 엔진의 **MRT에 사용하여 최종 SRV**에 안개 적용
<img width="1095" height="283" alt="image" src="https://github.com/user-attachments/assets/e32c4ffa-cfbd-4384-965e-8f38a79b3706" />

    - **Foliage**
    	- 인스턴싱을 활용한 렌더링으로 맵전체에 잔디(폴리지)구현 
	 <img width="1299" height="559" alt="image" src="https://github.com/user-attachments/assets/0aaa797e-ffc4-4445-82da-6a602bda5acc" />

          
 ![Untitled2](https://github.com/ckdlscjs/TeamProject/assets/41976800/a7b14cf1-81f3-4bc0-afc0-592d8eab7d8e)
- **UI**
    - **Fade In, Out / Gage**
        - Frame당 **Vertex의 R,G,B,A** 값을 조절해 Shader에서 사용
<img width="773" height="573" alt="image" src="https://github.com/user-attachments/assets/1d3715e4-cfae-49ea-b032-03748d71d711" />

    - **Billboard**
        - 투영행렬을 사용하는 **UI를 빌보드화** 하여 Damage, Gage 등에 사용
<img width="666" height="419" alt="image" src="https://github.com/user-attachments/assets/b6e3515c-696d-4814-ba81-4f7a81bde745" />

    - **Text**
        - DXWrite를 이용, Scenario등 텍스트를 띄우고 Fade In, Out을 같이 사용
    - **ClockWiseFade**
        - PixelShader내에서 ConstantBuffer에서 넘어온 **Degree값을 기반으로 atan**를 이용해 계산하여 시계방향 으로 Fade되는 효과를 구현
<img width="865" height="416" alt="image" src="https://github.com/user-attachments/assets/f02a1e59-85b0-4f03-a2c0-ed7353b755c0" />

    - **MiniMap**
        - 추가 카메라를 통해 별도의 **RTT, ViewPort를 이용해 렌더링** 플레이어와 Enemy는 GeometryShader를 이용한 Box로 Render
<img width="560" height="568" alt="image" src="https://github.com/user-attachments/assets/ee03277f-ed16-403a-b293-fac853dde880" />

    - **AlphaMapBlur**
        - 가우시안값, AlphaMapWeight을 통해 **가중치에 따른 SRV의 Blur** 구현

## 동영상
https://youtu.be/4ECHTmTD6h4?si=qe3qtL75Kr7zCrz7 (LoLShinSouls) </br>
https://youtu.be/_ik09B4ptmo?si=l5gQW1pp_IqxO2Hu (MapTools)
## 다운로드
http://naver.me/55RIqKpf (게임)
