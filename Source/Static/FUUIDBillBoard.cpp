#include "FUUIDBillBoard.h"
#include "Core/Engine.h"
#include "Core/Rendering/URenderer.h"
#include "FontAtlas.h"
#include "Core/Engine.h"
#include "Object/World/World.h"
#include "Object/Actor/Actor.h"
#include "Object/Actor/Camera.h"
#include <d3dcompiler.h>

void FUUIDBillBoard::UpdateString(const std::wstring& String)
{
	Flush();

	float cursorX = 0.0f;
	uint32 StringLen = String.size();

	for (size_t i = 0; i < StringLen; ++i)
	{
		wchar_t c = String[i];
		const GlyphInfo& glyph = FFontAtlas::Get().GetGlyph(c);

		FVertexTexture vertices[4] =
		{
			{ 0.0f, -0.4375f + cursorX - (StringLen - 1) * 0.4375f, 1.0f, glyph.u, glyph.v },
			{ 0.0f, 0.4375f + cursorX - (StringLen - 1) * 0.4375f, 1.0f, glyph.u + glyph.width, glyph.v },
			{ 0.0f, 0.4375f + cursorX - (StringLen - 1) * 0.4375f, -1.0f, glyph.u + glyph.width, glyph.v + glyph.height },
			{ 0.0f, -0.4375f + cursorX - (StringLen - 1) * 0.4375f, -1.0f, glyph.u, glyph.v + glyph.height }
		};

		for (int j = 0; j < 4; ++j)
		{
			VertexBuffer.Add(vertices[j]);
		}

		uint32 baseIndex = static_cast<uint32>(VertexBuffer.Num()) - 4;
		IndexBuffer.Add(baseIndex + 0);
		IndexBuffer.Add(baseIndex + 1);
		IndexBuffer.Add(baseIndex + 2);
		IndexBuffer.Add(baseIndex + 0);
		IndexBuffer.Add(baseIndex + 2);
		IndexBuffer.Add(baseIndex + 3);

		cursorX += 0.875f;
	}
}

void FUUIDBillBoard::Flush()
{
	VertexBuffer = {};
	IndexBuffer = {};
}

void FUUIDBillBoard::CalculateModelMatrix(FMatrix& OutMatrix)
{
	ACamera* cam = UEngine::Get().GetWorld()->GetCamera();

	FVector cameraPosition = cam->GetActorTransform().GetPosition();

	FVector objectPosition = TargetObject->GetWorldTransform().GetPosition();
	FVector objectScale(0.1f, 0.1f, 0.1f);

	FVector lookDir = (objectPosition - cameraPosition).GetSafeNormal();
	FVector right = FVector(0, 0, 1).Cross(lookDir).GetSafeNormal();
	FVector up = lookDir.Cross(right).GetSafeNormal();

	FMatrix rotationMatrix;

	// 회전 행렬 구성
	rotationMatrix.X = FVector4(lookDir.X, lookDir.Y, lookDir.Z, 0);
	rotationMatrix.Y = FVector4(right.X, right.Y, right.Z, 0);
	rotationMatrix.Z = FVector4(up.X, up.Y, up.Z, 0);
	rotationMatrix.W = FVector4(0, 0, 0, 1);

	FMatrix positionMatrix = FMatrix::GetTranslateMatrix(objectPosition);
	FMatrix scaleMatrix = FMatrix::GetScaleMatrix(objectScale);

	OutMatrix = scaleMatrix * rotationMatrix * positionMatrix;
	return;
}

void FUUIDBillBoard::SetTarget(AActor* Target) 
{
	TargetObject = Target->GetRootComponent();
}

void FUUIDBillBoard::Render()
{
	if (VertexBuffer.Num() == 0 || !TargetObject)
		return;

	//Prepare
	ID3D11DeviceContext* DeviceContext = FDevice::Get().GetDeviceContext();

	// 버텍스 버퍼 업데이트
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	DeviceContext->Map(FontVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, VertexBuffer.GetData(), sizeof(FVertexTexture) * VertexBuffer.Num());
	DeviceContext->Unmap(FontVertexBuffer, 0);

	// 인덱스 버퍼 업데이트
	DeviceContext->Map(FontIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, IndexBuffer.GetData(), sizeof(uint32) * IndexBuffer.Num());
	DeviceContext->Unmap(FontIndexBuffer, 0);

	// 파이프라인 상태 설정
	UINT stride = sizeof(FVertexTexture);
	UINT offset = 0;

	// 기본 셰이더랑 InputLayout을 설정

	DeviceContext->IASetVertexBuffers(0, 1, &FontVertexBuffer, &stride, &offset);
	DeviceContext->IASetIndexBuffer(FontIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(PrimitiveTopology);

	DeviceContext->VSSetShader(FontVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(FontPixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(FontInputLayout);

	DeviceContext->OMSetBlendState(BlendState, BlendFactor, 0xffffffff);

	// Billboard
	FMatrix ModelMatrix;
	CalculateModelMatrix(ModelMatrix);

	// 버텍스 쉐이더에 상수 버퍼를 설정
	if (FontConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &FontConstantBuffer);

		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;
		FFontConstantInfo Constants;
		URenderer* Renderer = UEngine::Get().GetRenderer();

		Constants.ViewProjectionMatrix = FMatrix::Transpose( ModelMatrix * Renderer->GetViewMatrix() * Renderer->GetProjectionMatrix());

		// D3D11_MAP_WRITE_DISCARD는 이전 내용을 무시하고 새로운 데이터로 덮어쓰기 위해 사용
		DeviceContext->Map(FontConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		{
			memcpy(ConstantBufferMSR.pData, &Constants, sizeof(FFontConstantInfo));
		}
		DeviceContext->Unmap(FontConstantBuffer, 0);
	}

	DeviceContext->DrawIndexed((UINT)IndexBuffer.Num(), 0, 0);
}

void FUUIDBillBoard::Create()
{
	ID3D11Device* Device = FDevice::Get().GetDevice();
	ID3D11DeviceContext* DeviceContext = FDevice::Get().GetDeviceContext();

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(FVertexTexture) * MaxVerticesPerBatch;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// 버텍스 버퍼 생성
	HRESULT hr = Device->CreateBuffer(&vertexBufferDesc, nullptr, &FontVertexBuffer);

	if (FAILED(hr))
	{
		return;
	}
	// 인덱스 버퍼 설명
	D3D11_BUFFER_DESC IndexBufferDesc = {};
	IndexBufferDesc.ByteWidth = sizeof(uint32) * MaxIndicesPerBatch; // LineVertex가 아닌 UINT로 수정
	IndexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // IMMUTABLE에서 DYNAMIC으로 변경 (Add 함수에서 업데이트하기 위함)
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // 쓰기 접근 권한 추가

	// 인덱스 버퍼 생성
	hr = Device->CreateBuffer(&IndexBufferDesc, nullptr, &FontIndexBuffer);
	if (FAILED(hr))
	{
	}

	// 라인 쉐이더 컴파일 및 생성
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// 버텍스 쉐이더 컴파일
	hr = D3DCompileFromFile(L"Shaders/Font_VS.hlsl", nullptr, nullptr, "Font_VS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		// 컴파일 오류 처리
		if (errorBlob)
		{
		}
	}

	// 픽셀 쉐이더 컴파일
	hr = D3DCompileFromFile(L"Shaders/Font_PS.hlsl", nullptr, nullptr, "Font_PS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		// 컴파일 오류 처리
		if (errorBlob)
		{
		}
	}

	// 쉐이더 생성
	hr = Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &FontVertexShader);
	if (FAILED(hr))
	{
		// 오류 처리
	}

	hr = Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &FontPixelShader);
	if (FAILED(hr))
	{
		// 오류 처리
	}

	// 입력 레이아웃 설정
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);

	hr = Device->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &FontInputLayout);
	if (FAILED(hr))
	{
		// 오류 처리
	}

	// 버텍스/픽셀 쉐이더 블롭 해제
	if (vsBlob) vsBlob->Release();
	if (psBlob) psBlob->Release();

	// 라인 렌더링을 위한 래스터라이저 상태 설정
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE; // 라인은 양면을 볼 수 있도록 컬링 없음
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = TRUE; // 안티앨리어싱된 라인 활성화

	// 래스터라이저 상태 생성
	hr = Device->CreateRasterizerState(&rasterizerDesc, &RasterizerState);
	if (FAILED(hr))
	{
		// 오류 처리
	}

	// 상수 버퍼 생성
	D3D11_BUFFER_DESC ConstantBufferDesc = {};
	ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;                        // 매 프레임 CPU에서 업데이트 하기 위해
	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;             // 상수 버퍼로 설정
	ConstantBufferDesc.ByteWidth = (sizeof(FFontConstantInfo) + 0xf) & 0xfffffff0;  // 16byte의 배수로 올림
	ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;            // CPU에서 쓰기 접근이 가능하게 설정

	Device->CreateBuffer(&ConstantBufferDesc, nullptr, &FontConstantBuffer);

	// Blend State
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

	Device->CreateBlendState(&blendDesc, &BlendState);
}


