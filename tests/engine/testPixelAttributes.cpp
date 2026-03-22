#include <gtest/gtest.h>

#include <engine/pixels/pixelEnum.hpp>

TEST(PixelAttributesTests, DefaultPropertyBehavior) {
    Pixel::DefaultPixelProperties props;
    EXPECT_EQ(props.color, glm::vec3(1.0f, 0.0f, 0.0f));
    EXPECT_FALSE(props.isSolid);

    props.isSolid = true;
    props.solidAttributes = Pixel::Solid();
    EXPECT_TRUE(props.isSolid);
}
