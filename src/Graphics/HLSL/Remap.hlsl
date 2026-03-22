// Remap.hlsl
// https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine
float remap_target(float value, float originMin, float originMax, float targetMin, float targetMax)
{
    return targetMin + (value - originMin) * (targetMax - targetMin) / (originMax - originMin);
} // remap_target

float remap_new(float value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + (((value - original_min) / (original_max - original_min)) * (new_max - new_min));
} // remap_new

float remap_clamp(float value, float original_min, float original_max, float new_min, float new_max)
{
    return min(max(new_max, new_min), max(min(new_max, new_min), remap_new(value, original_min, original_max, new_min, new_max)));
} // RemapClamp

float remap_normalize(float value, float original_min, float original_max, float new_min, float new_max)
{
    return min(1.0, max(0.0, remap_new(value, original_min, original_max, new_min, new_max)));
} // remap_normalize