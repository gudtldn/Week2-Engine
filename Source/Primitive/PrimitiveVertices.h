#pragma once
#include "Core/HAL/PlatformType.h"
#include "Core/Container/Array.h"
#include "Core/Math/Vector.h"

struct FVertexSimple
{
    float X, Y, Z;    // Position
    float R, G, B, A; // Color
};

struct FVertexTexture
{
	float X, Y, Z;    // Position
	float U, V; // UV
};

struct FLineVertexSimple {
	FVector position;
	FVector4 color;

	FLineVertexSimple(const FVector& pos, const FVector4& col) : position(pos), color(col) {}
};

struct FGeometryData
{
	TArray<FVertexSimple> Vertices;
	TArray<uint32> Indices;
};

enum class EPrimitiveType : uint8
{
	EPT_None,
	EPT_Line,
	EPT_Triangle,
	EPT_Quad,
	EPT_Cube,
	EPT_Sphere,
	EPT_Cylinder,
	EPT_Cone,
	EPT_Max,
};

extern FVertexSimple LineVertices[2];
extern FVertexSimple CubeVertices[36];
extern FVertexSimple SphereVertices[2400];
extern FVertexSimple TriangleVertices[3];

struct GlyphInfo
{
	float u;      
	float v;      
	float width;  
	float height; 
	float offsetX;
	float offsetY;
};