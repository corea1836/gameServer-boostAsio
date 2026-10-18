# 맵 에셋 후보 조사 (T-1)

> 조사일: 2026-10-18. 기준(`docs/ADR.md` ADR-017): CC0, 평평한 바닥, 쿼터뷰 로우폴리, 캐릭터(`QuarterView 3D Action BE5`)와 어울리는 스타일. 최종 선택은 사용자가 한다.

## 1. 기준이 되는 현재 에셋

- **캐릭터 `QuarterView 3D Action BE5`**: Qubicle로 만든 **복셀 모델**(FBX)에 64×64 크기의 단색 팔레트 텍스처를 입혔다.
  - **CC0가 아니다.** `Readme.asset` 원문: "골드메탈 출저만 표기하는 전제 하에 자유롭게 사용 가능합니다. (상용 가능)", "Copyright ⓒ 2021 Goldmetal". 포트폴리오 크레딧에 골드메탈을 꼭 적어야 한다.
- **지금 맵 에셋 `@Resources/Materials/*.obj`(103개)**: OBJ 헤더가 `# Created by Kenney (www.kenney.nl)`다. 파일 이름 103개가 **Kenney Platformer Kit 4.1**의 OBJ 153개에 전부 들어 있어서 출처를 확정했다. 눈 블록, 캐릭터, 일부 소품(`spring`, `pipe`, `star` 등)은 가져오지 않았다.
- **보유 중인 `~/Downloads/KayKit_Mystery_Monthly_Series_4`**: 맵이 아니라 **캐릭터 팩**이다.
  - 라이선스 원문(`License.txt`): "License: (Creative Commons Zero, CC0) http://creativecommons.org/publicdomain/zero/1.0/", "This content is free to use in personal, educational and commercial projects.", 크레딧은 "this is not mandatory".
  - 캐릭터: Orc Raider, Driver, Monster Costume, Werewolf, Animatronic, Action Figure, Space Ranger, Ninja, Survivalist, Paladin, Clown, Robot(12종, 2023-07~2024-06). 소품(무기, 방패 등)과 대체 텍스처, Source(.blend)가 함께 들어 있다.
  - 리그: 전부 `Rig_Medium`(관절 23개)이라 애니메이션을 공유한다. 바인드 포즈 기준 키는 약 2.1~2.5다.
  - 들어 있는 애니메이션: `General`(Idle, Hit, Death, Spawn, Interact, PickUp, Throw, Use_Item)과 `MovementBasic`(Walking, Running, Jump). **공격 모션은 없다.** 근접·원거리 공격과 막기는 같은 리그용 무료 팩 [KayKit Character Animations](https://kaylousberg.itch.io/kaykit-character-animations)(CC0, 150개 이상)에 있다.
  - 스타일: 매끈한 2~3등신 로우폴리라서 **BE5의 복셀과는 결이 다르다.** 오히려 KayKit Dungeon, Forest와 작가와 단위가 같다.

## 2. 후보 요약

크기는 OBJ 정점을 직접 읽어 잰 값이다(단위 = Unity 1m). "—"는 직접 열어 보지 못한 항목이다.

| 팩 | 라이선스 확인 | 무료 분량 | 바닥·그리드 크기 | 텍스처 | 캐릭터와 어울림 |
|---|---|---|---|---|---|
| Kenney Platformer Kit 4.1 (현재) | 팩 안 `License.txt` | 153개 | `block-grass` 1.08×1.0×1.08 (가장자리가 0.04씩 튀어나옴) | `colormap.png` 그라데이션 아틀라스 1장 | 보통. 둥글고 귀여운 로우폴리 |
| Kenney Nature Kit 2.1 | 팩 안 `License.txt` | 330개 | `ground_grass` 1×1 평면, `cliff_block` 1×1×1 | 없음(머티리얼 단색) | 보통. 각진 플랫 셰이딩 로우폴리 |
| KayKit Dungeon Remastered | GitHub 원본의 `LICENSE.txt` | 약 200개 | 바닥 4×4 / 2×2, 벽 4(폭)×4(높이)×1(두께) | 그라데이션 아틀라스 1장(1024², 128²까지 축소 가능) | 보통. 매끈하고 통통한 스타일 |
| KayKit Forest Nature Pack | itch.io 페이지 표기 | 100개 이상(나무·바위·덤불·풀) | — (지형 타일은 유료 Extra에만 있음) | 그라데이션 아틀라스 1장 | 보통. Dungeon과 같은 스타일 |
| Kytric Voxel Path And Terrain | itch.io 페이지 표기 | 371개 | — | — | **가장 비슷함. 같은 복셀** |

## 3. 팩별 정리

### 3.1 Kenney Platformer Kit (현재 사용 중)
- 링크: https://kenney.nl/assets/platformer-kit
- 라이선스 원문(`License.txt`):
  > License: (Creative Commons Zero, CC0) http://creativecommons.org/publicdomain/zero/1.0/
  > You can use this content for personal, educational, and commercial purposes.
  > Support by crediting 'Kenney' or 'www.kenney.nl' (this is not a requirement)
- 구성: 잔디·눈 블록(높이, 경사, 오버행 변형), 코인, 상자, 문, 울타리, 함정, 캐릭터 5종. 포맷은 FBX, GLB, OBJ.
- 판단: 위로 쌓는 플랫폼 게임용 블록이라 높낮이와 오버행 변형이 대부분이다. 평평한 바닥으로 쓰려면 같은 높이로 깔면 되지만, 블록이 1.08 폭이라 셀 경계와 정확히 맞지 않는다.

### 3.2 Kenney Nature Kit
- 링크: https://kenney.nl/assets/nature-kit
- 라이선스 원문(`License.txt`):
  > License: (Creative Commons Zero, CC0) http://creativecommons.org/publicdomain/zero/1.0/
  > This content is free to use in personal, educational and commercial projects.
  > Support us by crediting Kenney or www.kenney.nl (this is not mandatory)
- 구성: 바닥 타일(잔디, 길 14종, 강 14종), 절벽 블록과 코너·계단·폭포, 나무 약 50종, 바위·돌, 울타리, 다리, 농작물, 버섯. 포맷은 DAE, FBX, GLTF, OBJ, STL.
- 판단: **바닥 1×1, 절벽 1×1×1이라 셀 1.0 그리드와 1:1로 맞는다.** 충돌 굽기(T-3)를 할 때 경계가 깔끔하다. 지금 쓰는 Kenney 에셋과 같은 계열이라 섞어 써도 이질감이 적다. 단점은 복셀이 아니라 플랫 셰이딩 로우폴리라는 점이다.

### 3.3 KayKit Dungeon Remastered (Kay Lousberg)
- 링크: https://kaylousberg.itch.io/kaykit-dungeon-remastered , GitHub 원본: https://github.com/KayKit-Game-Assets/KayKit-Dungeon-Remastered-1.0
- 라이선스 원문(`Assets/LICENSE.txt`):
  > License: (Creative Commons Zero, CC0) http://creativecommons.org/publicdomain/zero/1.0/
  > This content is free to use in personal, educational and commercial projects.
  > Support me by crediting Kay Lousberg, www.kaylousberg.com (this is not mandatory)
- itch.io 페이지의 부탁(라이선스 조건은 아님): "please don't resell unmodified copies or claim them as your own."
- 가격: 원하는 만큼 지불. 무료(약 200개) / Extra $7.95 이상(275개 이상 + 대체 텍스처 6종) / Source $11.95 이상(.blend 포함).
- 구성: 바닥(흙, 타일, 창살, 가시 함정), 벽, 계단, 문, 상자, 통, 테이블, 함정. 포맷은 FBX, GLTF, OBJ. itch.io 1.1 버전부터 Unity용 FBX(스케일·방향 조정)가 들어 있다(GitHub 원본은 1.0).
- 판단: 크기가 2·4의 배수이고 벽 두께가 정확히 1이라 그리드에 잘 맞는다. 다만 던전 전용이라 첫 필드 맵보다는 나중의 던전·보스방(R6~R8) 후보로 적합하다.

### 3.4 KayKit Forest Nature Pack (Kay Lousberg)
- 링크: https://kaylousberg.itch.io/kaykit-forest
- 라이선스(itch.io 페이지 표기): Creative Commons Zero v1.0 Universal. "Free for personal and commercial use, no attribution required." 부탁 문구는 Dungeon과 같다.
- 가격: 무료(나무, 바위, 덤불, 풀 100개 이상) / Extra $9.99 이상(나무·바위 추가, **모듈형 지형**, 모델마다 색상 변형 8종) / Source $14.99 이상.
- 판단: 무료판에는 지형 타일이 없다. 바닥은 평면이나 다른 팩으로 깔고, 이 팩은 장애물 소품으로만 쓰는 조합이 된다. KayKit Dungeon과 스타일이 같아서 두 팩을 같이 쓰면 통일감이 있다.

### 3.5 Kytric Voxel Path And Terrain
- 링크: https://kytric.itch.io/voxel-path-and-terrain
- 라이선스(itch.io 페이지 표기): Creative Commons Zero v1.0 Universal. "Free to use in private and commercial projects.", "You can modifiy the assets as you want.", "Credits are welcome but not necessary."
- 가격: 원하는 만큼 지불(0원 가능).
- 구성: 371개. 숲, 흙, 사막, 도시, 물, 도로·길, 산, 나무, 돌, 소품. 포맷은 .vox, OBJ, glTF(각 2~3MB).
- 판단: **캐릭터와 같은 복셀이라 스타일은 가장 잘 맞는다.** 다만 itch.io 다운로드라 직접 열어 보지 못해서 타일 크기, 모듈 구조, 팩 안 라이선스 파일 여부를 확인하지 못했다. 고른다면 import한 뒤 1칸 크기부터 재 봐야 한다.

## 4. 제외한 후보

- **Quaternius Medieval Village MegaKit** (CC0, 2025-01, 304개, 그리드 기반 모듈): 반사실적 텍스처 스타일이라 복셀 캐릭터와 어울리지 않는다. https://quaternius.com/packs/medievalvillagemegakit.html
- **KayKit Medieval Hexagon Pack** (CC0): 육각 타일이라 사각 셀 그리드와 맞지 않는다. https://kaylousberg.itch.io/kaykit-medieval-hexagon

## 5. 참고: 크레딧

- CC0 팩은 크레딧 의무가 없다. Kenney는 로고 사용만 금지한다("Do not use our logo", https://kenney.nl/support).
- BE5 캐릭터는 골드메탈 출처 표기가 **의무**다(1절).

## 6. 현재 상태와 추천 (2026-10-18)

- 당분간 Map01을 쓴다. 새 맵은 나중에 Claude가 스크립트로 자동 배치한다(`docs/PRD.md` T-4, `docs/ADR.md` ADR-018).
- **캐릭터를 KayKit Mystery Monthly로 바꿀지는 미정이다.** 이 결정에 따라 스크립트가 배치할 부품 팩이 정해진다(T-4 전까지 정하면 된다).
  - KayKit으로 바꾸는 경우: **KayKit Dungeon + Forest**. 작가와 단위가 같아서 크기를 따로 맞출 필요가 없고, 캐릭터·애니메이션·맵이 전부 CC0이 된다. Forest 무료판에는 지형이 없으니 바닥은 Dungeon의 흙 바닥 타일이나 평면으로 깐다. 공격 모션은 KayKit Character Animations에서 가져온다.
  - BE5를 유지하는 경우: **Kenney Nature Kit**(1순위) 또는 Kytric 복셀(스타일 우선).
