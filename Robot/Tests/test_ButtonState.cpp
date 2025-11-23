#include <gtest/gtest.h>

#include "ButtonState.hpp"


TEST(ButtonState, button_never_pressed){
    bool button_state = false;
    ButtonState(&button_state);
    EXPECT_FALSE(button_state_obj.get_button_state());
    // button_state = true;
    // EXPECT_TRUE(button_state_obj.get_button_state());
    // EXPECT_FALSE(button_state_obj.get_button_state());
    // delay(27);
    // EXPECT_TRUE(button_state_obj.get_button_state());
    // EXPECT_FALSE(button_state_obj.get_button_state());
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}