#include "URenderer.h"
#include <d3dcompiler.h>
#include "Core/Math/Transform.h"
#include "Object/Actor/Camera.h"
#include "Object/PrimitiveComponent/UPrimitiveComponent.h"
#include "Static/FEditorManager.h"
#include "Static/FLineBatchManager.h"
#include "Resource/DirectResource/Vertexbuffer.h"
#include "DirectXTK/WICTextureLoader.h"
#include "FDevice.h"
#include "Debug/DebugConsole.h"
#include "Object/Assets/SceneAsset.h"
#include "Resource/DirectResource/PixelShader.h"
#include "Resource/DirectResource/VertexShader.h"
#include "Resource/DirectResource/InputLayout.h"


void URenderer::Create(HWND hWindow)
{
	//CreateDeviceAndSwapChain(hWindow);
	//CreateFrameBuffer();
	CreateRasterizerState();
	//CreateDepthStencilBuffer();
	CreateDepthStencilState();
	CreateBlendState();
	CreatePickingTexture(hWindow);

	FLineBatchManager::Get().Create();

	InitMatrix();

	LoadTexture(L"font_atlas.png");
}

void URenderer::Release()
{
	ReleaseRasterizerState();

	// 렌더 타겟을 초기화
	FDevice::Get().GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);

	//ReleaseFrameBuffer();
	ReleaseDepthStencilBuffer();
	//ReleaseDeviceAndSwapChain();
}

void URenderer::CreateShader()
{
	// /**
	//      * 컴파일된 셰이더의 바이트코드를 저장할 변수 (ID3DBlob)
	//      *
	//      * 범용 메모리 버퍼를 나타내는 형식
	//      *   - 여기서는 shader object bytecode를 담기위해 쓰임
	//      * 다음 두 메서드를 제공한다.
	//      *   - LPVOID GetBufferPointer
	//      *     - 버퍼를 가리키는 void* 포인터를 돌려준다.
	//      *   - SIZE_T GetBufferSize
	//      *     - 버퍼의 크기(바이트 갯수)를 돌려준다
	//      */
	// ID3DBlob* VertexShaderCSO;
	// ID3DBlob* PixelShaderCSO;

	ID3DBlob* PickingShaderCSO;

	ID3DBlob* FontVertexShaderCSO;
	ID3DBlob* FontPixelShaderCSO;

	ID3DBlob* ErrorMsg = nullptr;


	D3DCompileFromFile(L"Shaders/ShaderW0.hlsl", nullptr, nullptr, "PickingPS", "ps_5_0", 0, 0, &PickingShaderCSO, nullptr);
	FDevice::Get().GetDevice()->CreatePixelShader(PickingShaderCSO->GetBufferPointer(), PickingShaderCSO->GetBufferSize(), nullptr, &PickingPixelShader);

	// Font Shaders
	D3DCompileFromFile(L"Shaders/Font_VS.hlsl", nullptr, nullptr, "Font_VS", "vs_5_0", 0, 0, &FontVertexShaderCSO, &ErrorMsg);
	FDevice::Get().GetDevice()->CreateVertexShader(FontVertexShaderCSO->GetBufferPointer(), FontVertexShaderCSO->GetBufferSize(), nullptr, &FontVertexShader);

	D3DCompileFromFile(L"Shaders/Font_PS.hlsl", nullptr, nullptr, "Font_PS", "ps_5_0", 0, 0, &FontPixelShaderCSO, nullptr);
	FDevice::Get().GetDevice()->CreatePixelShader(FontPixelShaderCSO->GetBufferPointer(), FontPixelShaderCSO->GetBufferSize(), nullptr, &FontPixelShader);

	if (ErrorMsg)
	{
		std::cout << (char*)ErrorMsg->GetBufferPointer() << std::endl;
		ErrorMsg->Release();
	}

	// // 입력 레이아웃 정의 및 생성
	// D3D11_INPUT_ELEMENT_DESC Layout[] =
	// {
	//     { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//     { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	// };
	//
	// FDevice::Get().GetDevice()->CreateInputLayout(Layout, ARRAYSIZE(Layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &SimpleInputLayout);

	PickingShaderCSO->Release();

	// 정점 하나의 크기를 설정 (바이트 단위)
	Stride = sizeof(FVertexSimple);
}

void URenderer::ReleaseShader()
{

}

void URenderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC ConstantBufferDesc = {};
	ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;                        // 매 프레임 CPU에서 업데이트 하기 위해
	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;             // 상수 버퍼로 설정
	ConstantBufferDesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0;  // 16byte의 배수로 올림
	ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;            // CPU에서 쓰기 접근이 가능하게 설정

	FDevice::Get().GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBuffer);

	D3D11_BUFFER_DESC ConstantBufferDescPicking = {};
	ConstantBufferDescPicking.Usage = D3D11_USAGE_DYNAMIC;                        // 매 프레임 CPU에서 업데이트 하기 위해
	ConstantBufferDescPicking.BindFlags = D3D11_BIND_CONSTANT_BUFFER;             // 상수 버퍼로 설정
	ConstantBufferDescPicking.ByteWidth = sizeof(FPickingConstants) + 0xf & 0xfffffff0;  // 16byte의 배수로 올림
	ConstantBufferDescPicking.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;            // CPU에서 쓰기 접근이 가능하게 설정

	FDevice::Get().GetDevice()->CreateBuffer(&ConstantBufferDescPicking, nullptr, &ConstantPickingBuffer);

	D3D11_BUFFER_DESC ConstantBufferDescDepth = {};
	ConstantBufferDescPicking.Usage = D3D11_USAGE_DYNAMIC;                        // 매 프레임 CPU에서 업데이트 하기 위해
	ConstantBufferDescPicking.BindFlags = D3D11_BIND_CONSTANT_BUFFER;             // 상수 버퍼로 설정
	ConstantBufferDescPicking.ByteWidth = sizeof(FDepthConstants) + 0xf & 0xfffffff0;  // 16byte의 배수로 올림
	ConstantBufferDescPicking.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;            // CPU에서 쓰기 접근이 가능하게 설정

	FDevice::Get().GetDevice()->CreateBuffer(&ConstantBufferDescPicking, nullptr, &ConstantsDepthBuffer);
}

void URenderer::ReleaseConstantBuffer()
{
	if (ConstantBuffer)
	{
		ConstantBuffer->Release();
		ConstantBuffer = nullptr;
	}

	if (ConstantPickingBuffer)
	{
		ConstantPickingBuffer->Release();
		ConstantPickingBuffer = nullptr;
	}

	if (ConstantsDepthBuffer)
	{
		ConstantsDepthBuffer->Release();
		ConstantsDepthBuffer = nullptr;
	}
}



void URenderer::Prepare() const
{


	/**
	 * OutputMerger 설정
	 * 렌더링 파이프라인의 최종 단계로써, 어디에 그릴지(렌더 타겟)와 어떻게 그릴지(블렌딩)를 지정
	 */
	 //FDevice::Get().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	FDevice::Get().GetDeviceContext()->RSSetState(RasterizerState);
	FDevice::Get().GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareShader() const
{
	// 기본 셰이더랑 InputLayout을 설정
	//FDevice::Get().GetDeviceContext()->VSSetShader(SimpleVertexShader, nullptr, 0);
	//FDevice::Get().GetDeviceContext()->PSSetShader(SimplePixelShader, nullptr, 0);
	//FDevice::Get().GetDeviceContext()->IASetInputLayout(SimpleInputLayout);

	// 버텍스 쉐이더에 상수 버퍼를 설정
	if (ConstantBuffer)
	{
		FDevice::Get().GetDeviceContext()->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
	if (ConstantsDepthBuffer)
	{
		FDevice::Get().GetDeviceContext()->PSSetConstantBuffers(2, 1, &ConstantsDepthBuffer);
	}
}

void URenderer::RenderPrimitive(class UPrimitiveComponent& PrimitiveComp, const class FMatrix& ModelMatrix)
{



	FMatrix MVP = FMatrix::Transpose(
		ModelMatrix *
		ViewMatrix *
		ProjectionMatrix
	);

	FConstants UpdateInfo{
		MVP,
		PrimitiveComp.GetCustomColor(),
		PrimitiveComp.IsUseVertexColor()
	};

	UpdateConstant(UpdateInfo);




	RenderPrimitiveInternal(PrimitiveComp);

}

void URenderer::RenderPrimitiveInternal(class UPrimitiveComponent& PrimitiveComp) const
{
	UINT Offset = 0;

	//임시로 만듦 각 렌더러에 맞는 쉐이더를 넣어야함
	//FDevice::Get().GetDeviceContext()->VSSetShader(SimpleVertexShader, nullptr, 0);
	//FDevice::Get().GetDeviceContext()->PSSetShader(SimplePixelShader, nullptr, 0);

	if (PrimitiveComp.VertexShader == nullptr)
	{
		UE_LOG("Error: VertexShader has not been set.");
	}
	else
	{

	}

	if (PrimitiveComp.PixelShader == nullptr)
	{
		UE_LOG("Error: PixelShader has not been set.");
	}

	if (PrimitiveComp.VertexBuffer == nullptr)
	{
		UE_LOG("Error: VertexBuffer has not been set.");

	}

	if (PrimitiveComp.IndexBuffer == nullptr)
	{
		UE_LOG("Error: IndexBuffer has not been set.");
	}

	PrimitiveComp.VertexBuffer->Setting();
	PrimitiveComp.VertexShader->Setting();
	PrimitiveComp.PixelShader->Setting();
	PrimitiveComp.IndexBuffer->Setting();
	PrimitiveComp.InputLayout->Setting();

	FDevice::Get().GetDeviceContext()->IASetPrimitiveTopology(PrimitiveComp.Topology);

	FDevice::Get().GetDeviceContext()->RSSetState(RasterizerState);


	FDevice::Get().GetDeviceContext()->DrawIndexed(PrimitiveComp.IndexBuffer->GetIndexCount(), 0, 0);
}

ID3D11Buffer* URenderer::CreateVertexBuffer(const FVertexSimple* Vertices, UINT ByteWidth) const
{
	D3D11_BUFFER_DESC VertexBufferDesc = {};
	VertexBufferDesc.ByteWidth = ByteWidth;
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA VertexBufferSRD = {};
	VertexBufferSRD.pSysMem = Vertices;

	ID3D11Buffer* VertexBuffer;
	const HRESULT Result = FDevice::Get().GetDevice()->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, &VertexBuffer);
	if (FAILED(Result))
	{
		return nullptr;
	}
	return VertexBuffer;
}

void URenderer::LoadTexture(const wchar_t* texturePath)
{
	DirectX::CreateWICTextureFromFile(FDevice::Get().GetDevice(), FDevice::Get().GetDeviceContext(), texturePath, nullptr, &FontTextureSRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	FDevice::Get().GetDevice()->CreateSamplerState(&samplerDesc, &FontSamplerState);
	FDevice::Get().GetDeviceContext()->PSSetShaderResources(0, 1, &FontTextureSRV);
	FDevice::Get().GetDeviceContext()->PSSetSamplers(0, 1, &FontSamplerState);
}

ID3D11Buffer* URenderer::CreateIndexBuffer(const uint32* Indices, UINT ByteWidth) const
{
	D3D11_BUFFER_DESC IndexBufferDesc = {};
	IndexBufferDesc.ByteWidth = ByteWidth;
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA IndexBufferSRD = {};
	IndexBufferSRD.pSysMem = Indices;

	ID3D11Buffer* IndexBuffer;
	const HRESULT Result = FDevice::Get().GetDevice()->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, &IndexBuffer);
	if (FAILED(Result))
	{
		return nullptr;
	}
	return IndexBuffer;
}

void URenderer::ReleaseVertexBuffer(ID3D11Buffer* pBuffer) const
{
	pBuffer->Release();
}

void URenderer::UpdateConstant(const FConstants& UpdateInfo) const
{
	if (!ConstantBuffer) return;


	D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;
	// 상수 버퍼를 CPU 메모리에 매핑

	  // D3D11_MAP_WRITE_DISCARD는 이전 내용을 무시하고 새로운 데이터로 덮어쓰기 위해 사용
	FDevice::Get().GetDeviceContext()->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
	{
		// 매핑된 메모리를 FConstants 구조체로 캐스팅
		FConstants* Constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
		Constants->MVP = UpdateInfo.MVP;
		Constants->Color = UpdateInfo.Color;
		Constants->bUseVertexColor = UpdateInfo.bUseVertexColor ? 1 : 0;
	}
	FDevice::Get().GetDeviceContext()->Unmap(ConstantBuffer, 0);
}








void URenderer::CreateBlendState()
{
	// Blend
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	FDevice::Get().GetDevice()->CreateBlendState(&blendDesc, &BlendState);
}

void URenderer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = {};
	DepthStencilDesc.DepthEnable = TRUE;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;                     // 더 작은 깊이값이 왔을 때 픽셀을 갱신함
	// DepthStencilDesc.StencilEnable = FALSE;                                 // 스텐실 테스트는 하지 않는다.
	// DepthStencilDesc.StencilReadMask = 0xFF;
	// DepthStencilDesc.StencilWriteMask = 0xFF;
	// DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	// DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	// DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	// DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	// DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	// DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	// DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	FDevice::Get().GetDevice()->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);

	D3D11_DEPTH_STENCIL_DESC IgnoreDepthStencilDesc = {};
	DepthStencilDesc.DepthEnable = TRUE;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	FDevice::Get().GetDevice()->CreateDepthStencilState(&IgnoreDepthStencilDesc, &IgnoreDepthStencilState);
}


void URenderer::ReleaseDepthStencilBuffer()
{

	if (DepthStencilState)
	{
		DepthStencilState->Release();
		DepthStencilState = nullptr;
	}
	if (IgnoreDepthStencilState)
	{
		IgnoreDepthStencilState->Release();
		IgnoreDepthStencilState = nullptr;
	}
}

void URenderer::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC RasterizerDesc = {};
	RasterizerDesc.FillMode = D3D11_FILL_SOLID; // 채우기 모드
	RasterizerDesc.CullMode = D3D11_CULL_BACK;  // 백 페이스 컬링
	RasterizerDesc.FrontCounterClockwise = FALSE;

	FDevice::Get().GetDevice()->CreateRasterizerState(&RasterizerDesc, &RasterizerState);
}

void URenderer::ReleaseRasterizerState()
{
	if (RasterizerState)
	{
		RasterizerState->Release();
		RasterizerState = nullptr;
	}
}


void URenderer::InitMatrix()
{
	WorldMatrix = FMatrix::Identity();
	ViewMatrix = FMatrix::Identity();
	ProjectionMatrix = FMatrix::Identity();
}

void URenderer::ReleasePickingFrameBuffer()
{
	if (PickingFrameBuffer)
	{
		PickingFrameBuffer->Release();
		PickingFrameBuffer = nullptr;
	}
	if (PickingFrameBufferRTV)
	{
		PickingFrameBufferRTV->Release();
		PickingFrameBufferRTV = nullptr;
	}
}

void URenderer::CreatePickingTexture(HWND hWnd)
{
	RECT Rect;
	int Width, Height;

	Width = FDevice::Get().GetViewPortInfo().Width;
	Height = FDevice::Get().GetViewPortInfo().Height;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Width;
	textureDesc.Height = Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	FDevice::Get().GetDevice()->CreateTexture2D(&textureDesc, nullptr, &PickingFrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC PickingFrameBufferRTVDesc = {};
	PickingFrameBufferRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // 색상 포맷
	PickingFrameBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

	FDevice::Get().GetDevice()->CreateRenderTargetView(PickingFrameBuffer, &PickingFrameBufferRTVDesc, &PickingFrameBufferRTV);
}

void URenderer::PrepareZIgnore()
{
	FDevice::Get().GetDeviceContext()->OMSetDepthStencilState(IgnoreDepthStencilState, 0);
}

void URenderer::PreparePicking()
{
	// 렌더 타겟 바인딩
	FDevice::Get().GetDeviceContext()->OMSetRenderTargets(1, &PickingFrameBufferRTV, FDevice::Get().GetDepthStencilView());
	FDevice::Get().GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	FDevice::Get().GetDeviceContext()->OMSetDepthStencilState(DepthStencilState, 0);                // DepthStencil 상태 설정. StencilRef: 스텐실 테스트 결과의 레퍼런스

	FDevice::Get().GetDeviceContext()->ClearRenderTargetView(PickingFrameBufferRTV, PickingClearColor);
}

void URenderer::PreparePickingShader() const
{
	FDevice::Get().GetDeviceContext()->PSSetShader(PickingPixelShader, nullptr, 0);

	if (ConstantPickingBuffer)
	{
		FDevice::Get().GetDeviceContext()->PSSetConstantBuffers(1, 1, &ConstantPickingBuffer);
	}
}

void URenderer::UpdateConstantPicking(FVector4 UUIDColor) const
{
	if (!ConstantPickingBuffer) return;

	D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

	UUIDColor = FVector4(UUIDColor.X / 255.0f, UUIDColor.Y / 255.0f, UUIDColor.Z / 255.0f, UUIDColor.W / 255.0f);

	FDevice::Get().GetDeviceContext()->Map(ConstantPickingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
	{
		FPickingConstants* Constants = static_cast<FPickingConstants*>(ConstantBufferMSR.pData);
		Constants->UUIDColor = UUIDColor;
	}
	FDevice::Get().GetDeviceContext()->Unmap(ConstantPickingBuffer, 0);
}

void URenderer::UpdateConstantDepth(int Depth) const
{
	if (!ConstantsDepthBuffer) return;

	ACamera* Cam = FEditorManager::Get().GetCamera();

	D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

	FDevice::Get().GetDeviceContext()->Map(ConstantsDepthBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
	{
		FDepthConstants* Constants = static_cast<FDepthConstants*>(ConstantBufferMSR.pData);
		Constants->DepthOffset = Depth;
		Constants->nearPlane = Cam->GetNear();
		Constants->farPlane = Cam->GetFar();
	}
	FDevice::Get().GetDeviceContext()->Unmap(ConstantsDepthBuffer, 0);
}

void URenderer::PrepareMain()
{
	FDevice::Get().GetDeviceContext()->OMSetDepthStencilState(DepthStencilState, 0);                // DepthStencil 상태 설정. StencilRef: 스텐실 테스트 결과의 레퍼런스
	//FDevice::Get().GetDeviceContext()->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
	FDevice::Get().GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

// void URenderer::PrepareMainShader()
// {
//     FDevice::Get().GetDeviceContext()->PSSetShader(SimplePixelShader, nullptr, 0);
// }

FVector4 URenderer::GetPixel(FVector MPos)
{
	MPos.X = FMath::Clamp(MPos.X, 0.0f, FDevice::Get().GetViewPortInfo().Width);
	MPos.Y = FMath::Clamp(MPos.Y, 0.0f, FDevice::Get().GetViewPortInfo().Height);
	// 1. Staging 텍스처 생성 (1x1 픽셀)
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	stagingDesc.Width = 1; // 픽셀 1개만 복사
	stagingDesc.Height = 1;
	stagingDesc.MipLevels = 1;
	stagingDesc.ArraySize = 1;
	stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 원본 텍스처 포맷과 동일
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	ID3D11Texture2D* stagingTexture = nullptr;
	FDevice::Get().GetDevice()->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);

	// 2. 복사할 영역 정의 (D3D11_BOX)
	D3D11_BOX srcBox = {};
	srcBox.left = static_cast<UINT>(MPos.X);
	srcBox.right = srcBox.left + 1; // 1픽셀 너비
	srcBox.top = static_cast<UINT>(MPos.Y);
	srcBox.bottom = srcBox.top + 1; // 1픽셀 높이
	srcBox.front = 0;
	srcBox.back = 1;
	FVector4 color{ 1, 1, 1, 1 };

	if (stagingTexture == nullptr)
		return color;

	// 3. 특정 좌표만 복사
	FDevice::Get().GetDeviceContext()->CopySubresourceRegion(
		stagingTexture, // 대상 텍스처
		0,              // 대상 서브리소스
		0, 0, 0,        // 대상 좌표 (x, y, z)
		PickingFrameBuffer, // 원본 텍스처
		0,              // 원본 서브리소스
		&srcBox         // 복사 영역
	);

	// 4. 데이터 매핑
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	FDevice::Get().GetDeviceContext()->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);

	// 5. 픽셀 데이터 추출 (1x1 텍스처이므로 offset = 0)
	const BYTE* pixelData = static_cast<const BYTE*>(mapped.pData);

	if (pixelData)
	{
		color.X = static_cast<float>(pixelData[0]); // R
		color.Y = static_cast<float>(pixelData[1]); // G
		color.Z = static_cast<float>(pixelData[2]); // B
		color.W = static_cast<float>(pixelData[3]); // A
	}

	std::cout << "X: " << (int)color.X << " Y: " << (int)color.Y
		<< " Z: " << color.Z << " A: " << color.W << "\n";

	// 6. 매핑 해제 및 정리
	FDevice::Get().GetDeviceContext()->Unmap(stagingTexture, 0);
	stagingTexture->Release();

	return color;
}

void URenderer::UpdateViewMatrix(const FTransform& CameraTransform)
{
	ViewMatrix = CameraTransform.GetViewMatrix();
}

void URenderer::UpdateProjectionMatrix(ACamera* Camera)
{
	float AspectRatio = UEngine::Get().GetScreenRatio();

	float FOV = FMath::DegreesToRadians(Camera->GetFieldOfView());
	float Near = Camera->GetNear();
	float Far = Camera->GetFar();

	if (Camera->ProjectionMode == ECameraProjectionMode::Perspective)
	{
		ProjectionMatrix = FMatrix::PerspectiveFovLH(FOV, AspectRatio, Near, Far);
	}
	else if (Camera->ProjectionMode == ECameraProjectionMode::Perspective)
	{
		ProjectionMatrix = FMatrix::PerspectiveFovLH(FOV, AspectRatio, Near, Far);

		// TODO: 추가 필요.
		// ProjectionMatrix = FMatrix::OrthoForLH(FOV, AspectRatio, Near, Far);
	}
}

void URenderer::OnUpdateWindowSize(int Width, int Height)
{

	ReleasePickingFrameBuffer();
}

void URenderer::OnResizeComplete()
{
	CreatePickingTexture(UEngine::Get().GetWindowHandle());

	// 깊이 스텐실 버퍼를 재생성
}

void URenderer::RenderPickingTexture()
{
	// 백버퍼로 복사
	ID3D11Texture2D* backBuffer;
	FDevice::Get().GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	FDevice::Get().GetDeviceContext()->CopyResource(backBuffer, PickingFrameBuffer);
	backBuffer->Release();
}