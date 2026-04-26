//
// Created by namusanga on 4/18/26.
//

#include "../image_loaders/img_loader.h"
#include "catch2/catch_test_macros.hpp"


TEST_CASE("img_loader: loads PNG", "[img_loader]")
{
    // path to the image
    const char* image_path = "assets/japan.jpg";
    ImageData loaded_image = load_image(image_path);

    REQUIRE(loaded_image.data != nullptr);
    printf("Image Location: %p \n", loaded_image.data);

    REQUIRE(loaded_image.height > 0);
    printf("Loaded Image height: %i \n", loaded_image.height);

    REQUIRE(loaded_image.width > 0);
    printf("Loaded Image width: %i \n", loaded_image.width);


    REQUIRE(loaded_image.channels > 0);
    printf("Loaded image with channels: %i \n", loaded_image.channels);


    //This text is expected to run on the specific japan image
    REQUIRE(loaded_image.height == 1080);
    REQUIRE(loaded_image.width == 1920);
}
