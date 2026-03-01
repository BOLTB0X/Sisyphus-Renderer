#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace SharedConstants {
    
    namespace PBRTextureConstants {
        enum class PBRTextureType {
            Albedo, Normal, Metallic,
            Roughness, AO, Alpha, 
            Displacement, Emissive, Unknown
        };

        struct PBRTextureKeyword {
            PBRTextureType type;
            std::vector<std::string> keywords;
        };

        inline const std::vector<PBRTextureKeyword> PBRTEXTURE_KEYWORD_MAP = {
            { PBRTextureType::Albedo,     { "_basecolor", "_albedo", "_alb", "_diffuse", "_col" } },
            { PBRTextureType::Normal,     { "_normal", "_nrm", "_norm", "_n" } },
            { PBRTextureType::Metallic,   { "_metallic", "_metal", "_m" } },
            { PBRTextureType::Roughness,  { "_roughness", "_rough", "_r" } },
            { PBRTextureType::AO,         { "_ao", "_occlusion" } },
            { PBRTextureType::Alpha,      { "_alpha", "_opacity" } }
        }; // PBRTEXTURE_KEYWORD_MAP

    } // PBRTextureConstants
}