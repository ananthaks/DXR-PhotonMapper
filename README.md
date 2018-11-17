# DXR-PhotonMapper

## Goal
An implementation of Progressive Photon Mapping using DXR

## Acknowledgement
The Titan V used for this project was donated by the NVIDIA Corporation.
We gratefully acknowledge the support of NVIDIA Corporation with the donation of the Titan V GPU used for this project

The base code for this project was inspired from DirectX Ray Tracing samples from Microsoft. The repository can be found here: https://github.com/Microsoft/DirectX-Graphics-Samples

## Build Instructions

1) Ensure you have Windows 10 (atleast build 1803)
2) Enable Developer Mode : Settings > Updates and Security > For Developers > Select Developer mode
3) Visual Studio 2017 version 15.8.7
4) Windows 10 SDK. [Version 10.0.17763.132](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk)
5) Open the PhotonMapper\PhotonMapper.sln and perform Build

**Note** DXR is currently being run on 'Fallback Layer'. Some GPUs may not supported this feature.
Currently tested GPU is NVIDIA GTX 1080, running driver version 399.07. I am speculating that any GTX 10x0 should run, but please ensure that its driver is up to date.
