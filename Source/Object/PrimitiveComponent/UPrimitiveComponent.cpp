#include "UPrimitiveComponent.h"
#include "Object/World/World.h"
#include "Object/Actor/Actor.h"
#include "Core/Engine.h"
#include "Object/Actor/Camera.h"
#include "Primitive/UGeometryGenerator.h"
#include "Resource/DirectResource/Vertexbuffer.h"
<<<<<<< Updated upstream
#include "Resource/DirectResource/VertexShader.h"
#include "Resource/DirectResource/PixelShader.h"
#include "Resource/DirectResource/InputLayout.h"
=======
#include "FontAtlas.h"
>>>>>>> Stashed changes

//#include ""

UPrimitiveComponent::UPrimitiveComponent()
{
	bCanBeRendered = true;
	VertexShader = FVertexShader::Find("Simple_VS");
	PixelShader = FPixelShader::Find("Simple_PS");

	// TODO: 이거는 나중에 매쉬같은데서  만들어야함
	InputLayout = FInputLayout::Find("Simple_VS");
	
}

UPrimitiveComponent::~UPrimitiveComponent()
{
	class std::shared_ptr<class FVertexBuffer> VertexBuffer = nullptr;
	class std::shared_ptr<class FIndexBuffer> IndexBuffer = nullptr;
}

void UPrimitiveComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPrimitiveComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 
}

void UPrimitiveComponent::UpdateConstantPicking(const URenderer& Renderer, const FVector4 UUIDColor)const
{
	Renderer.UpdateConstantPicking(UUIDColor);
}

void UPrimitiveComponent::UpdateConstantDepth(const URenderer& Renderer, const int Depth)const
{
	Renderer.UpdateConstantDepth(Depth);
}

void UPrimitiveComponent::Render()
{
	URenderer* Renderer = UEngine::Get().GetRenderer();
	if (Renderer == nullptr || !bCanBeRendered)
	{
		return;
	}
	if (GetOwner()->IsGizmoActor() == false)
	{
		if (bIsPicked)
		{
			/*bUseVertexColor = false;
			SetCustomColor(FVector4(1.0f, 0.647f, 0.0f, 1.0f));*/
		}
		else
		{
			bUseVertexColor = true;
		}
	}
	

	//FMatrix MVP = FMatrix::Transpose(
	//	scaleMatrix *
	//	result *
	//	positionMatrix *
	//	ViewMatrix *
	//	ProjectionMatrix
	//);

	FMatrix ModelMatrix;
	
	CalculateModelMatrix(ModelMatrix);

	Renderer->RenderPrimitive(*this, ModelMatrix);
}

void UPrimitiveComponent::CalculateModelMatrix(FMatrix& OutMatrix)
{

	//빌보드 행렬계산
	if (bIsBillboard == true)
	{
		ACamera* cam = UEngine::Get().GetWorld()->GetCamera();

		FVector cameraPosition = cam->GetActorTransform().GetPosition();


		FVector objectPosition = GetWorldTransform().GetPosition();	
		FVector objectScale = GetWorldTransform().GetScale();

		FVector lookDir = (objectPosition - cameraPosition).GetSafeNormal();

		// 언리얼 좌표계에 맞춘 구형 빌보드
		// Z축이 상방 벡터

		// Y축(우측)은 상방 벡터와 시선 방향의 외적
		FVector right = FVector(0, 0, 1).Cross(lookDir).GetSafeNormal();

		FVector up = lookDir.Cross(right).GetSafeNormal();
	

		// X축(전방)은 우측 벡터와 상방 벡터의 외적
		// 언리얼에서는 X가 전방이므로 이렇게 계산
		//FVector forward = right.Cross(up).GetSafeNormal();


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
	OutMatrix = GetWorldTransform().GetMatrix();
	return;
}

void UPrimitiveComponent::RegisterComponentWithWorld(UWorld* World)
{
	World->AddRenderComponent(this);
}

UCubeComp::UCubeComp()
{
	VertexBuffer = FVertexBuffer::Find("Cube");
	IndexBuffer = FIndexBuffer::Find("Cube");
	
	//없으면 만든다.
	if (VertexBuffer == nullptr)
	{
		TArray<FVertexSimple> vertices;
		TArray<uint32> indices;
		float size = 1.f;

		UGeometryGenerator::CreateCube(size, &vertices, &indices);
		
		VertexBuffer = FVertexBuffer::Create(FString("Cube"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("Cube"), indices);
	}

		
	
	bCanBeRendered = true;
	
}

USphereComp::USphereComp()
{
	//없으면 만든다.
	VertexBuffer= FVertexBuffer::Find("Sphere");
	IndexBuffer = FIndexBuffer::Find("Sphere");
	if (VertexBuffer == nullptr)
	{
		TArray<FVertexSimple> vertices;
		TArray<uint32> indices;
		int slices = 16;
		int stacks = 16;
		int32 radius = 1.f;
		float height = 1.f;

		UGeometryGenerator::CreateSphere(radius, slices, stacks, &vertices, &indices);
		
		VertexBuffer = FVertexBuffer::Create(FString("Sphere"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("Sphere"), indices);
	}
	
	bCanBeRendered = true;

}

UTriangleComp::UTriangleComp()
{
	//없으면 만든다.
	VertexBuffer= FVertexBuffer::Find("Triangle");
	IndexBuffer = FIndexBuffer::Find("Triangle");
	if (VertexBuffer == nullptr)
	{
		FVertexSimple tempArray[] =
		{
			{  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f },
			{  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f },
			{  0.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f } 
		};
		//TArray<FVertexSimple> vertices(tempArray, 3);
		TArray<FVertexSimple> vertices;

		vertices.Add(tempArray[0]);
		vertices.Add(tempArray[1]);
		vertices.Add(tempArray[2]);
		
		uint32 TriangleIndices[3] =
		{
			0, 1, 2
		};


		
		TArray<uint32> indices;
		indices.Add(TriangleIndices[0]);
		indices.Add(TriangleIndices[1]);
		indices.Add(TriangleIndices[2]);
		
		//indices.Append(TriangleIndices, 3);
		
		VertexBuffer = FVertexBuffer::Create(FString("Triangle"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("Triangle"), indices);
	}
}

ULineComp::ULineComp()
{//없으면 만든다.
	VertexBuffer= FVertexBuffer::Find("Line");
	IndexBuffer = FIndexBuffer::Find("Line");

	Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	if (VertexBuffer == nullptr)
	{
		
		
		FVertexSimple tempArray[2] =
		{
			{ -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f }
		};
		//TArray<FVertexSimple> vertices(tempArray, 3);
		TArray<FVertexSimple> vertices;

		vertices.Add(tempArray[0]);
		vertices.Add(tempArray[1]);
		
		uint32 tempIndices[2] =
		{
			0, 1
		};
		
		
		TArray<uint32> indices;
		indices.Add(tempIndices[0]);
		indices.Add(tempIndices[1]);
		
		//indices.Append(TriangleIndices, 3);
		
		VertexBuffer = FVertexBuffer::Create(FString("Line"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("Line"), indices);
	}
}

//UQuadComp::UQuadComp()
//{
//	//없으면 만든다.
//	VertexBuffer = FVertexBuffer::Find("Quad");
//	IndexBuffer = FIndexBuffer::Find("Quad");
//	if (VertexBuffer == nullptr)
//	{
//		FVertexTexture tempArray[] =
//		{
//			{  0.0f, -1.0f, 1.0f, 0.0f, 0.0f },
//			{  0.0f, 1.0f, 1.0f, 1.0f, 0.0f, },
//			{  0.0f, 1.0f, -1.0f,  1.0f, 1.0f },
//			{  0.0f, -1.0f, -1.0f,  0.0f, 1.0f },
//		};
//
//		TArray<FVertexTexture> vertices;
//
//		vertices.Add(tempArray[0]);
//		vertices.Add(tempArray[1]);
//		vertices.Add(tempArray[2]);
//		vertices.Add(tempArray[3]);
//
//		uint32 tempIndices[6] =
//		{
//			0, 1, 2,
//			0, 2, 3,
//		};
//
//		TArray<uint32> indices;
//
//		indices.Add(tempIndices[0]);
//		indices.Add(tempIndices[1]);
//		indices.Add(tempIndices[2]);
//		indices.Add(tempIndices[3]);
//		indices.Add(tempIndices[4]);
//		indices.Add(tempIndices[5]);
//
//		VertexBuffer = FVertexBuffer::Create(FString("Quad"), vertices);
//		IndexBuffer = FIndexBuffer::Create(FString("Quad"), indices);
//	}
//}

UTextureQuadComp::UTextureQuadComp()
{
	//없으면 만든다.
	bIsBillboard = true;

	VertexBuffer = FVertexBuffer::Find("TexQuad");
	IndexBuffer = FIndexBuffer::Find("TexQuad");

	if (VertexBuffer == nullptr)
	{
		FFontAtlas Atlas;

		wchar_t c = L'L';

		const GlyphInfo& glyph = Atlas.GetGlyph(c);

		FVertexTexture tempArray[] =
		{
			/*{  0.0f, -1.0f, 1.0f, 0.0f, 0.0f },
			{  0.0f, 1.0f, 1.0f, 1.0f, 0.0f, },
			{  0.0f, 1.0f, -1.0f,  1.0f, 1.0f },
			{  0.0f, -1.0f, -1.0f,  0.0f, 1.0f },*/
			{  0.0f, -0.5f, 1.0f, glyph.u, glyph.v },
			{  0.0f, 0.5f, 1.0f, glyph.u + glyph.width, glyph.v },
			{  0.0f, 0.5f, -1.0f,  glyph.u + glyph.width, glyph.v + glyph.height },
			{  0.0f, -0.5f, -1.0f,  glyph.u, glyph.v + glyph.height },
		};

		TArray<FVertexTexture> vertices;

		vertices.Add(tempArray[0]);
		vertices.Add(tempArray[1]);
		vertices.Add(tempArray[2]);
		vertices.Add(tempArray[3]);

		uint32 tempIndices[6] =
		{
			0, 1, 2,
			0, 2, 3,
		};

		TArray<uint32> indices;

		indices.Add(tempIndices[0]);
		indices.Add(tempIndices[1]);
		indices.Add(tempIndices[2]);
		indices.Add(tempIndices[3]);
		indices.Add(tempIndices[4]);
		indices.Add(tempIndices[5]);

		VertexBuffer = FVertexBuffer::Create(FString("TexQuad"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("TexQuad"), indices);
	}
}

UCylinderComp::UCylinderComp()
{
	//없으면 만든다.
	VertexBuffer= FVertexBuffer::Find("Cylinder");
	IndexBuffer = FIndexBuffer::Find("Cylinder");
	if (VertexBuffer == nullptr)
	{
		TArray<FVertexSimple> vertices;
		TArray<uint32> indices;
		int slices = 36;
		int stacks = 36;
		float bRadius = .2f;
		float tRdius = .2f;
		float height = 1.f;

		UGeometryGenerator::CreateCylinder(bRadius, tRdius, height, slices, stacks, &vertices , &indices);
		
		VertexBuffer = FVertexBuffer::Create(FString("Cylinder"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("Cylinder"), indices);
	}
}

UConeComp::UConeComp()
{
	//없으면 만든다.
	VertexBuffer= FVertexBuffer::Find("Cone");
	IndexBuffer = FIndexBuffer::Find("Cone");
	if (VertexBuffer == nullptr)
	{
		TArray<FVertexSimple> vertices;
		TArray<uint32> indices;
		int slices = 36;
		int stacks = 6;
		float radius = 1.f;
		float height = 1.f;

		UGeometryGenerator::CreateCone(radius, height, slices, stacks, &vertices, &indices);
		VertexBuffer = FVertexBuffer::Create(FString("Cone"), vertices);
		IndexBuffer = FIndexBuffer::Create(FString("Cone"), indices);
	}
}
