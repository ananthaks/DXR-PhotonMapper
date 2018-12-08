#pragma once

namespace GlobalRootSignatureParamsWithPrimitives 
{
    enum Value 
    {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        SceneConstantSlot,
        IndexBuffersSlot,
        VertexBuffersSlot,
        GeomIndexSlot,
        MaterialSlot,
        LightSlot,
        Count 
    };
}

namespace LocalRootSignatureParams 
{
    enum Value 
    {
        CubeConstantSlot = 0,
        Count 
    };
}

namespace ComputeRootSignatureParams
{
	enum Value
	{
		OutputViewSlot = 0,
		ParamConstantBuffer,
		Count
	};
}
