#ifndef TESTS_INPUTS_H_
#define TESTS_INPUTS_H_

#include <stdbool.h>

bool test_lookat_from_inputs__by_default_look_at_negative_z_axis(void);
bool test_lookat_from_inputs__yaw_is_counter_clockwise__90_deg(void);
bool test_lookat_from_inputs__yaw_is_counter_clockwise__60_deg(void);
bool test_inputs_from_lookat__yaw_is_counter_clockwise__90_deg(void);
bool test_inputs_from_lookat__yaw_is_counter_clockwise__60_deg(void);
bool test_inputs_from_lookat__by_default_look_at_negative_z_axis(void);
bool test_inputs_from_lookat__id(void);
bool test_lookat_from_inputs__id(void);
bool test_inputs_from_lookat__real_fail(void);

bool all_inputs_tests(void);


#endif // TESTS_INPUTS_H_
