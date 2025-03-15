#include "Vertexbuffer.h"
#include "Core/Engine.h"
#include "Core/Rendering/URenderer.h"
#include "Debug/DebugConsole.h"
#include "d3d11.h"
#include "Core/Rendering/FDevice.h"



<<<<<<< Updated upstream
=======
//D3D11_INPUT_ELEMENT_DESC FVertexBuffer:: Layout[] = {
//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//};
>>>>>>> Stashed changes

void FVertexBuffer::Setting() const
{
	if (nullptr == Buffer)
	{
		UE_LOG("Error: Vertexbuffer Setting Failed");
	}
	
	FDevice::Get().GetDeviceContext()->IASetVertexBuffers(0, 1, &Buffer, &VertexSize, &Offset);
}

void FVertexBuffer::ResCreate(const void* _Data, size_t _VertexSize, size_t _VertexCount)
{
	
	VertexSize = static_cast<UINT>(_VertexSize);
	VertexCount = static_cast<UINT>(_VertexCount);

	D3D11_SUBRESOURCE_DATA Data;
	Data.pSysMem = _Data;

	BufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferInfo.ByteWidth = static_cast<UINT>(VertexSize * VertexCount);

	BufferInfo.CPUAccessFlags = 0;
	BufferInfo.Usage = D3D11_USAGE_DEFAULT;

	if (S_OK != FDevice::Get().GetDevice()->CreateBuffer(&BufferInfo, &Data, &Buffer))
	{
		UE_LOG("Error: Vertexbuffer Create Failed") ;
		return;
	}
}
