/*
 * A simple kernel module for controlling a rotary encoder.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jtk");
MODULE_DESCRIPTION("A button/led driver");
MODULE_VERSION("0.0.1");

static unsigned gpio_led = 50;
static unsigned encdr_b = 71;
static unsigned encdr_a = 70;
static unsigned encdr[] = {70, 71};
static unsigned irqnum[2];
static int rotary_counter = 0;
static bool led_on = false; //default state: off

enum encoder_pinout {
	_A_ = 0,
	_B_ = 1,
};

static irq_handler_t butdev_isr(unsigned int irq, void *dev_id,
                                        struct pt_regs *reg);

/*
 * Set up the gpio and irq instace.
 */
static int __init butdev_init(void)
{
        int res = 0;
        if (!gpio_is_valid(gpio_led)) {
                printk(KERN_INFO "invalid gpio number");
                return -ENODEV;
        }

        led_on = true;
        gpio_request(gpio_led, "led");
        gpio_direction_output(gpio_led, led_on);
        gpio_export(gpio_led, false);
	
        gpio_request(encdr_a, "pin_a");
        gpio_direction_input(encdr_a);
        gpio_set_debounce(encdr_a, 200);
        gpio_export(encdr_a, false);	
	
        gpio_request(encdr_b, "pin_b");
        gpio_direction_input(encdr_b);
        gpio_set_debounce(encdr_b, 200);
        gpio_export(encdr_b, false);	
	
	gpio_request_array(encdr, ARRAY_SIZE(encdr));
	for (size_t i = 0; i < ARRAY_SIZE(encdr); i++)
		gpio_direction_input(encdr[i]);
	
        printk(KERN_INFO "ENCDR_TEST: The encoder state is currently: %d, %d\n",
			gpio_get_value(encdr[_A_]), gpio_get_value(encdr[_B_]);

	for (size_t i = 0; i < ARRAY_SIZE(encdr); i++)
		irqnum[i] = gpio_to_irq(encdr[i]);

        printk(KERN_INFO "ENCDR_TEST: The encoder is mapped to IRQ: %d, %d\n", 
						irqnum[_A_], irqnum[_B_]);

        res = request_irq(irqnum, (irq_handler_t) butdev_isr, 
                IRQF_TRIGGER_RISING, "button_handler", NULL);
        printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", res);
        return res;
}

static void __exit butdev_exit(void)
{       
        printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", 
                                        gpio_get_value(gpio_button));
        printk(KERN_INFO "GPIO_TEST: The button was pressed %d times\n",
                                                        press_count);
        gpio_set_value(gpio_led, 0);
        gpio_unexport(gpio_led);
        free_irq(irqnum, NULL);
        gpio_unexport(gpio_button);
        gpio_free(gpio_led);
        gpio_free(gpio_button);
        printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

static irq_handler_t butdev_isr(unsigned int irq, void *dev_id,
                                        struct pt_regs *regs)
{
        led_on = !led_on;
        gpio_set_value(gpio_led, led_on);
        printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", 
                                        gpio_get_value(gpio_button));
        press_count++;
        return (irq_handler_t) IRQ_HANDLED;
}

module_init(butdev_init);
module_exit(butdev_exit);
