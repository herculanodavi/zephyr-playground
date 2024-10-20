#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/kernel.h>

#define SLEEP_TIME_MS 1000U
#define LED_NODE DT_CHOSEN(app_led)
#define RTC_NODE DT_ALIAS(rtc_node)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct device *rtc_device = DEVICE_DT_GET(RTC_NODE);

int main(void)
{
    int err = 0;

    if (!gpio_is_ready_dt(&led))
    {
        return -1;
    }
    err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (err < 0)
    {
        return -1;
    }

    if (!device_is_ready(rtc_device))
    {
        printk("Device is not ready!");
    }
    err = device_init(rtc_device);
    if (err < 0)
    {
        printk("Failed to init rtc device\n");
    }

    struct rtc_time time;
    memset(&time, 0, sizeof(time));

    err = rtc_set_time(rtc_device, &time);
    if (err < 0)
    {
        printk("Failed to set time\n");
    }

    while (1)
    {
        (void)gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);
        err = rtc_get_time(rtc_device, &time);
        printk("Got current time is: %02d:%02d:%02d, with code %d\n", time.tm_hour, time.tm_min, time.tm_sec, err);
    }

    return 0;
}