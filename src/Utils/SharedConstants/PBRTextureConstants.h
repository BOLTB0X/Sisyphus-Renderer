#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace SharedConstants {
    
    namespace PBRTextureConstants {
        enum class PBRTextureType {
            Albedo, Normal, Metallic,
            Roughness, AO, Alpha, 
            Displacement, Emissive,
            Specular, Subsurface,
            Smoothnes, Unknown
        };

        struct PBRTextureKeyword {
            PBRTextureType           type;
            std::vector<std::string> keywords;
        };

        inline const std::vector<PBRTextureKeyword> PBRTEXTURE_KEYWORD_MAP = {
            { PBRTextureType::Albedo,     { "_basecolor", "_albedo", "_alb", "_diffuse", "_col", "_Col", "_diff", "_BaseColor", "_d"}},
            { PBRTextureType::Normal,     { "_normal", "_nrm", "_norm", "_n", "_Normal", "_Nor"}},
            { PBRTextureType::Metallic,   { "_metallic", "_metal", "_m" } },
            { PBRTextureType::Roughness,  { "_roughness", "_rough", "_r", "_Roughness", "_Rgh", "_Rgn"}},
            { PBRTextureType::AO,         { "_ao", "_occlusion", "_Occlusion", "_AO"}},
            { PBRTextureType::Alpha,      { "_alpha", "_opacity" , "_Opacity_Map" } },
            { PBRTextureType::Specular,   { "_Specular", "_specular", "_spec" } },
            { PBRTextureType::Subsurface, { "_ssss", "_SSSS", "_sss", "_Subsurface" } },
            { PBRTextureType::Smoothnes,  { "_Smoothness", "_smoothness", "_smoot" } }
        }; // PBRTEXTURE_KEYWORD_MAP

    } // PBRTextureConstants
}