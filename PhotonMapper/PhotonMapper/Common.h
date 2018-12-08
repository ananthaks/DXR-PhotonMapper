#pragma once

namespace GlobalRootSignatureParams 
{
    enum Value 
    {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        //GeomIndexSlot,
        SceneConstantSlot,
        IndexBuffersSlot,
        VertexBuffersSlot,
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
