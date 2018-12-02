#pragma once

namespace GlobalRootSignatureParams 
{
    enum Value 
    {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        SceneConstantSlot,
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
		Count
	};
}
