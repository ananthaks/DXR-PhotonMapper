#ifndef MATERIALSHADER_H
#define MATERIALSHADER_H


inline float LambertShader(float3 worldPos, float3 cameraPos, float3 normal)
{
    float3 lightVec =  cameraPos - worldPos;
    float diffuseTerm = dot(normalize(normal), normalize(lightVec));
    diffuseTerm = clamp(diffuseTerm, 0, 1);
    return (diffuseTerm + AMBIENT_LIGHT);
}




#endif // ANALYTICPRIMITIVES_H