#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#define SLEEP_TIME_MS 1000U
#define LED_NODE      DT_CHOSEN(app_led)
#define IMU_NODE      DT_ALIAS(imu_i2c)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct device* imu_i2c_dev = DEVICE_DT_GET(IMU_NODE);

int main(void)
{
    int err = 0;

    if (!gpio_is_ready_dt(&led)) { return -1; }

    err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (err < 0) { return -1; }

    while (1)
    {
        (void) gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);
    }
 
    return 0;
}