DXR-PhotonMapper
======================

**Anantha Srinivas**
[LinkedIn](https://www.linkedin.com/in/anantha-srinivas-00198958/), [Twitter](https://twitter.com/an2tha)  |  **Connie Chang**
[LinkedIn](https://www.linkedin.com/in/conniechang44), [Demo Reel](https://www.vimeo.com/ConChang/DemoReel)

# Progressive Photon Mapping using DXR

![](images/dxr-seed.jpg)

## Inspiration

Real-time rendering has become a reality thanks to NVIDIA's RTX GPU and Microsoft's DXR API. However, there is still a long way to go until these become prevalent across games. One of the possible application of this technology could be in Virtual Reality. Rendering in VR is a lot more complicated and hard considering that we have to render multi-view (which can be done through multi-pass or other efficient techniques such as multi-view rendering from Microsoft). This is where Photon Mapping can fully utilize our hardware. Since photon generation and light calculation is camera independent, we can reuse the same photon data for both the eye, possibly improving performance in VR. While this project solely focuses on implementing Photon Mapping using DirectX RayTracing, its use in VR could be a future application.

## Specific Goals of the project

* Have an understanding and working of DirectX Ray tracing running with D3D12.
* Build Custom spatial data structures (k-d trees) using DXR framework to store photons
* Construct a framework that has the following stages:
1) Photon Generation and Propogation.
2) Ray tracing for color retrieval.

## Proposed Pipeline
![](images/flow.png)

## Milestones

|Date|Milestone goals|
|----| -----|
|11/19| Have an Initial DXR setup working|
|11/26| Implement K-d trees for storing Photons and basic photon generation|
|12/03| Implementation of Progressive photon mapping|
|12/10| Final Renders and fine tuning|

# References

1) [Real time ray tracing](https://arstechnica.com/gadgets/2018/08/microsoft-announces-the-next-step-in-gaming-graphics-directx-raytracing/)
2) [DXR Tutorials](https://github.com/NVIDIAGameWorks/DxrTutorials)
3) [DXR Introduction](http://intro-to-dxr.cwyman.org)
4) NVIDIA DXR Documentation [Part1](https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial-Part-1), [Part2](https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial-Part-2)
