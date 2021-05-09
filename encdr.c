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
MODULE_DESCRIPTION("A simple encoder driver");
MODULE_VERSION("0.0.1");

static struct gpio leds[] = {
	{50, GPIOF_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED, "PIN_A led"},
	{51, GPIOF_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED, "PIN_B led"},
};

static struct gpio encdr[] = {
	{70, GPIOF_DIR_IN | GPIOF_EXPORT_DIR_FIXED, "PIN_A"},
	{71, GPIOF_DIR_IN | GPIOF_EXPORT_DIR_FIXED, "PIN_B"},
};

static irqnum[2];

static irq_handler_t encdr_isr(unsigned irqnum, void *dev_id,
					struct pt_regs *regs);

/*
 * Set up teh gpio and irq instance.
 */
static int __init encdr_init(void)
{
	gpio_request_array(leds, ARRAY_SIZE(leds));
	gpio_request_array(encdr, ARRAY_SIZE(encdr));
	
	for (size_t i = 0; i < ARRAY_SIZE(encdr); i++) {
		irqnum[i] = gpio_to_irq(encdr[i].gpio);
		request_irq(irqnum[i], (irq_handler_t) encdr_isr, 
			IRQF_TRIGGER_RISING, "encdr handler", NULL);
	}

	return 0;
}


















