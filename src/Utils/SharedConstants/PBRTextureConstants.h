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
            Specular, Leaf,
            Unknown
        };

        struct PBRTextureKeyword {
            PBRTextureType           type;
            std::vector<std::string> keywords;
        };

        inline const std::vector<PBRTextureKeyword> PBRTEXTURE_KEYWORD_MAP = {
            { PBRTextureType::Albedo,     { "_basecolor", "_albedo", "_alb", "_diffuse", "_col", "_diff", "_BaseColor"}},
            { PBRTextureType::Normal,     { "_normal", "_nrm", "_norm", "_n" , "_Normal"}},
            { PBRTextureType::Metallic,   { "_metallic", "_metal", "_m" } },
            { PBRTextureType::Roughness,  { "_roughness", "_rough", "_r", "_Roughness"}},
            { PBRTextureType::AO,         { "_ao", "_occlusion" } },
            { PBRTextureType::Alpha,      { "_alpha", "_opacity" , "_Opacity_Map" } },
            { PBRTextureType::Specular,   { "_Specular", "_specular", "_spec" } },
			{ PBRTextureType::Leaf,       { "_leaf", "_Leaf", "_leaves"}}
        }; // PBRTEXTURE_KEYWORD_MAP

    } // PBRTextureConstants
}