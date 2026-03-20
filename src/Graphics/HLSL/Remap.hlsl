// Remap.hlsl
// https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine
float remap(float value, float originMin, float originMax, float targetMin, float targetMax)
{
    return targetMin + (value - originMin) * (targetMax - targetMin) / (originMax - originMin);
} // remap

float Remap(float value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + (((value - original_min) / (original_max - original_min)) * (new_max - new_min));
} // Remap

float RemapClamp(float value, float original_min, float original_max, float new_min, float new_max)
{
    return min(max(new_max, new_min), max(min(new_max, new_min), Remap(value, original_min, original_max, new_min, new_max)));
} // RemapClamp

float RemapNormalize(float value, float original_min, float original_max, float new_min, float new_max)
{
    return min(1.0, max(0.0, Remap(value, original_min, original_max, new_min, new_max)));
} // RemapNormalize