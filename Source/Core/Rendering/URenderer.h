#pragma once

#define _TCHAR_DEFINED  // TCHAR 재정의 에러 때문
#include <d3d11.h>

#include "UI.h"
#include "Core/Math/Vector.h"
// #include "Object/Actor/Camera.h"
#include "Core/Math/Matrix.h"
#include "Core/Engine.h"
#include "Primitive/PrimitiveVertices.h"


struct FVertexSimple;
struct FVector4;

class ACamera;

class URenderer
{
public:
	friend class FLineBatchManager;
	
private:
    struct alignas(16) FConstants
    {
        FMatrix MVP;
        FVector4 Color;
		// true인 경우 Vertex Color를 사용하고, false인 경우 Color를 사용합니다.
        uint32 bUseVertexColor;
        FVector Padding;
    };
	
	struct alignas(16) FPickingConstants
	{
		FVector4 UUIDColor;
	};

	struct alignas(16) FDepthConstants{
		unsigned int DepthOffset;
		int nearPlane;
		int farPlane;
	};
	
  //  struct ConstantUpdateInfo
  //  {
  //      const FTransform& Transform;
		//const FVector4& Color;
  //      bool bUseVertexColor;
  //  };

public:
	
    /** Renderer를 초기화 합니다. */
    void Create(HWND hWindow);

    /** Renderer에 사용된 모든 리소스를 해제합니다. */
    void Release();

    void CreateShader();

    void ReleaseShader();

    void CreateConstantBuffer();

    void ReleaseConstantBuffer();



    /** 렌더링 파이프라인을 준비 합니다. */
    void Prepare() const;

    /** 셰이더를 준비 합니다. */
    void PrepareShader() const;

	void RenderPrimitive(class UPrimitiveComponent& PrimitiveComp , const class FMatrix& ModelMatrix);

    /**
     * Buffer에 있는 Vertex를 그립니다.
     * @param pBuffer 렌더링에 사용할 버텍스 버퍼에 대한 포인터
     * @param numVertices 버텍스 버퍼에 저장된 버텍스의 총 개수
     */
    void RenderPrimitiveInternal(class UPrimitiveComponent& PrimitiveComp) const;

	void LoadTexture(const wchar_t* texturePath);
	ID3D11ShaderResourceView* FontTextureSRV = nullptr;
	ID3D11SamplerState* FontSamplerState = nullptr;
    /**
     * 정점 데이터로 Vertex Buffer를 생성합니다.
     * @param Vertices 버퍼로 변환할 정점 데이터 배열의 포인터
     * @param ByteWidth 버퍼의 총 크기 (바이트 단위)
     * @return 생성된 버텍스 버퍼에 대한 ID3D11Buffer 포인터, 실패 시 nullptr
     *
     * @note 이 함수는 D3D11_USAGE_IMMUTABLE 사용법으로 버퍼를 생성합니다.
     */
    ID3D11Buffer* CreateVertexBuffer(const FVertexSimple* Vertices, UINT ByteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(const uint32* Indices, UINT ByteWidth) const;

    /** Buffer를 해제합니다. */
    void ReleaseVertexBuffer(ID3D11Buffer* pBuffer) const;

    /** Constant Data를 업데이트 합니다. */
    void UpdateConstant(const FConstants& UpdateInfo) const;


    /** View 변환 Matrix를 업데이트 합니다. */
    void UpdateViewMatrix(const FTransform& CameraTransform);

    /** Projection 변환 Matrix를 업데이트 합니다. */
    void UpdateProjectionMatrix(ACamera* Camera);

	//픽킹용으로 남겨둠
	void OnUpdateWindowSize(int Width, int Height); 
	void OnResizeComplete();

protected:





    /** 뎁스 스텐실 상태를 생성합니다. */
	void CreateDepthStencilState();
	
	/** 블렌드 상태를 생성합니다. */
	void CreateBlendState();


	/** 뎁스 스텐실 리소스는 디바이스 뎁스 설정은 여기서 날린다. */
	void ReleaseDepthStencilBuffer();
	
    /** 레스터라이즈 상태를 생성합니다. */
    void CreateRasterizerState();

    /** 레스터라이저 상태를 해제합니다. */
    void ReleaseRasterizerState();

    void InitMatrix();

protected:




	
    ID3D11RasterizerState* RasterizerState = nullptr;       // 래스터라이저 상태(컬링, 채우기 모드 등 정의)
    ID3D11Buffer* ConstantBuffer = nullptr;                 // 쉐이더에 데이터를 전달하기 위한 상수 버퍼


    // Shader를 렌더링할 때 사용되는 변수들
	
	ID3D11VertexShader* FontVertexShader = nullptr;       // Vertex 데이터를 처리하는 Vertex 셰이더
	ID3D11PixelShader* FontPixelShader = nullptr;         // Pixel의 색상을 결정하는 Pixel 셰이더

<<<<<<< Updated upstream
    //ID3D11InputLayout* SimpleInputLayout = nullptr;         // Vertex 셰이더 입력 레이아웃 정의
=======
    ID3D11InputLayout* SimpleInputLayout = nullptr;         // Vertex 셰이더 입력 레이아웃 정의
	ID3D11InputLayout* TextureInputLayout = nullptr;
>>>>>>> Stashed changes
    unsigned int Stride = 0;                                // Vertex 버퍼의 각 요소 크기

	ID3D11DepthStencilState* DepthStencilState = nullptr;   // DepthStencil 상태(깊이 테스트, 스텐실 테스트 등 정의)
    ID3D11DepthStencilState* GizmoDepthStencilState = nullptr; // 기즈모용 스텐실 스테이트. Z버퍼 테스트 하지않고 항상 앞에렌더
	
	// Blend state
	ID3D11BlendState* BlendState = nullptr;
	float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	FMatrix WorldMatrix;
    FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;

	D3D_PRIMITIVE_TOPOLOGY CurrentTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	
#pragma region picking
protected:
	// 피킹용 버퍼들
	ID3D11Texture2D* PickingFrameBuffer = nullptr;                 // 화면 출력용 텍스처
	ID3D11RenderTargetView* PickingFrameBufferRTV = nullptr;       // 텍스처를 렌더 타겟으로 사용하는 뷰
	ID3D11Buffer* ConstantPickingBuffer = nullptr;                 // 뷰 상수 버퍼
	FLOAT PickingClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; //
	ID3D11PixelShader* PickingPixelShader = nullptr;         // Pixel의 색상을 결정하는 Pixel 셰이더
	ID3D11Buffer* ConstantsDepthBuffer = nullptr;

	ID3D11DepthStencilState* IgnoreDepthStencilState = nullptr;   // DepthStencil 상태(깊이 테스트, 스텐실 테스트 등 정의)

public:
	//피킹용 함수들	
    void ReleasePickingFrameBuffer();
    void CreatePickingTexture(HWND hWnd);
    void PrepareZIgnore();
    void PreparePicking();
	void PreparePickingShader() const;
	void UpdateConstantPicking(FVector4 UUIDColor) const;
    void UpdateConstantDepth(int Depth) const;

    void PrepareMain();
	//void PrepareMainShader();

	FVector4 GetPixel(FVector MPos);

	void RenderPickingTexture();
	FMatrix GetProjectionMatrix() const { return ProjectionMatrix; }
	FMatrix GetViewMatrix() const { return ViewMatrix; }
#pragma endregion picking
};
