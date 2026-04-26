#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "../../../../../../external/stb_image/stb_image.h"

struct ImageData
{
    int width, height, channels;
    unsigned char* data = nullptr;

    //@TODO: Make sure to clean this memory up (destructor)

};


inline ImageData load_image (const char* filename)
{
    ImageData returnData;
    returnData.data = stbi_load(filename, &returnData.width, &returnData.height, &returnData.channels, 4); // here we force the engine to load with 4 channels
    return returnData;
}
