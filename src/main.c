/**
 * @file Demonstrates the difference between linear and CIE lightness PWM-based LED fading.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1));

const uint8_t CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[] = {
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    2,    2,    2,    2,    2,    2,    2,    2,    3,    3,    3,    3,    3,
    3,    3,    4,    4,    4,    4,    4,    5,    5,    5,    5,    5,    6,    6,    6,    6,
    6,    7,    7,    7,    7,    8,    8,    8,    8,    9,    9,    9,   10,   10,   10,   11,
   11,   11,   12,   12,   12,   13,   13,   13,   14,   14,   14,   15,   15,   16,   16,   16,
   17,   17,   18,   18,   19,   19,   20,   20,   21,   21,   22,   22,   23,   23,   24,   24,
   25,   25,   26,   26,   27,   28,   28,   29,   29,   30,   31,   31,   32,   33,   33,   34,
   35,   35,   36,   37,   37,   38,   39,   40,   40,   41,   42,   43,   44,   44,   45,   46,
   47,   48,   49,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,
   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   75,   76,   77,   78,
   79,   80,   82,   83,   84,   85,   87,   88,   89,   90,   92,   93,   94,   96,   97,   99,
  100,  101,  103,  104,  106,  107,  108,  110,  111,  113,  114,  116,  118,  119,  121,  122,
  124,  125,  127,  129,  130,  132,  134,  135,  137,  139,  141,  142,  144,  146,  148,  149,
  151,  153,  155,  157,  159,  161,  162,  164,  166,  168,  170,  172,  174,  176,  178,  180,
  182,  185,  187,  189,  191,  193,  195,  197,  200,  202,  204,  206,  208,  211,  213,  215,
  218,  220,  222,  225,  227,  230,  232,  234,  237,  239,  242,  244,  247,  249,  252,  255,
};

int main(void)
{
    uint32_t pwmPeriod_ns = 20*1000*1000; // 20ms/50Hz
    uint32_t numSteps = 256;
    uint32_t stepDuration_ms = 5;
    uint32_t stepIdx = 0;
    bool goingForward = true;

    printk("PWM-based LED fade\n");

    // Make sure PWM is ready
    if (!pwm_is_ready_dt(&pwm_led0)) {
        printk("Error: PWM device %s is not ready\n",
               pwm_led0.dev->name);
        return 0;
    }

    if (!pwm_is_ready_dt(&pwm_led1)) {
        printk("Error: PWM device %s is not ready\n",
               pwm_led1.dev->name);
        return 0;
    }

    while (1) {
        // Calculate the pulse width for liner LED
        uint32_t pulseWidthLinear_ns = (uint32_t)((float)stepIdx/(numSteps - 1) * pwmPeriod_ns);

        // Calculate the pulse width for CIE lightness LED
        uint8_t lutValue = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[stepIdx];
        uint32_t pulseWidthCieLightness_ns = (uint32_t)((float)lutValue/255 * pwmPeriod_ns);

        // Update linear LED
        int ret = pwm_set_dt(&pwm_led0, pwmPeriod_ns, pulseWidthLinear_ns);
        if (ret) {
            printk("Error %d: failed to set pulse width\n", ret);
            return 0;
        }

        // Update CIE lightness LED
        ret = pwm_set_dt(&pwm_led1, pwmPeriod_ns, pulseWidthCieLightness_ns);
        if (ret) {
            printk("Error %d: failed to set pulse width\n", ret);
            return 0;
        }

        // Update the step index
        if (goingForward) {
            stepIdx++;
            if (stepIdx == numSteps) {
                goingForward = false;
                stepIdx = numSteps - 1;
            }
        } else {
            stepIdx--;
            if (stepIdx == 0) {
                goingForward = true;
            }
        }

        k_sleep(K_MSEC(stepDuration_ms));
    }
    return 0;
}
