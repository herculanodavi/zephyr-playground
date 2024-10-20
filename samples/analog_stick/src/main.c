/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                         DT_SPEC_AND_COMMA)};

#define STICK_BUTTON_NODE	DT_ALIAS(stick_button)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(STICK_BUTTON_NODE, gpios);

int main(void)
{
    int err;
    uint32_t count = 0;
    uint16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
    };

    printk("Starting device...");

    /* Configure channels individually prior to sampling. */
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
    {
        if (!adc_is_ready_dt(&adc_channels[i]))
        {
            printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return 0;
        }

        err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0)
        {
            printk("Could not setup channel #%d (%d)\n", i, err);
            return 0;
        }
    }

	err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (err != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       err, button.port->name, button.pin);
		return 0;
	}

    int32_t axes[2];
    int button_press;
    while (1)
    {
        printk("ADC reading[%u]:\n", count++);

        for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
        {
            (void)adc_sequence_init_dt(&adc_channels[i], &sequence);
            err = adc_read_dt(&adc_channels[i], &sequence);
            if (err < 0)
            {
                printk("Could not read (%d)\n", err);
                continue;
            }
            axes[i] = ((int32_t) buf * 100) / 4096;
        }

        button_press = gpio_pin_get_dt(&button);

        printk("X: %d%%\n", axes[0]);
        printk("Y: %d%%\n", axes[1]);
        printk("Button: %d\n", button_press);

        k_sleep(K_MSEC(200));
    }
    return 0;
}