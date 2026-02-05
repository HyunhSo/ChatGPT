# MyModularGame (UE5)

`ModularGameplay` + `GameFeatures`를 바로 사용할 수 있도록 최소 C++ 프로젝트 골격을 구성했습니다.

## 포함된 설정

- `MyModularGame.uproject`
  - `ModularGameplay`, `GameFeatures`, `EnhancedInput` 플러그인 활성화
- `Source/MyModularGame`
  - 기본 게임 모듈
  - `AModularGameModeBase`를 상속한 `AMyModularGameMode` 추가
- `Config/DefaultGame.ini`
  - 기본 GameMode를 `AMyModularGameMode`로 지정

## UE5에서 여는 방법

1. UE5(권장 5.3+)에서 `MyModularGame.uproject`를 엽니다.
2. "C++ 프로젝트 파일 생성/업데이트"를 실행합니다.
3. IDE에서 빌드 후 에디터를 재실행합니다.

## 다음 추천 작업

- `GameFeature` 플러그인 템플릿 생성 후, 컴포넌트 액션(`GameFeatureAction_AddComponents`)으로 Pawn/Controller에 기능 주입
- `ModularGameplayActors` 기반 `PlayerController`, `Character` 클래스 추가
- 입력은 `EnhancedInput`의 Mapping Context를 Game Feature 활성화 시점에 등록
