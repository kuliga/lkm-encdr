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

static int irqnum[2];

static irq_handler_t encdr_isr(unsigned irqnum, void *dev_id,
					struct pt_regs *regs);

/*
 * Set up the gpio and irq instance.
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

	printk(KERN_INFO "ENCDR: siema!\n");
	return 0;
}

static void __exit encdr_exit(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(encdr); i++) 
		free_irq(irqnum[i], NULL);

	gpio_free_array(encdr, ARRAY_SIZE(encdr));
	gpio_free_array(leds, ARRAY_SIZE(leds));
	printk(KERN_INFO "ENCDR: nara!\n");
}

static irq_handler_t encdr_isr(unsigned irqnum, void *dev_id,
					struct pt_regs *regs)
{
	const u16 valid_states = 0b0110100110010110;
	static u8 ab_match_dir, ab_state, rotary_counter; 
	u8 ab_curr = 0;
	for (size_t i = 0; i < ARRAY_SIZE(encdr); i++)
		ab_curr |= gpio_get_value(encdr[i].gpio) << i;
		
	ab_state <<= 2;
	ab_state |= ab_curr;
	if (valid_states & (1 << (ab_state & 0x0f))) {
		ab_match_dir <<= 4;
		ab_match_dir |= ab_state;
		switch (ab_match_dir) {
		case 0x2b:
			rotary_counter++;
			goto isr_exit;
		case 0x17:
			rotary_counter--;
			goto isr_exit;
		default:
			goto isr_exit;
		}
	}
isr_exit:
	printk(KERN_INFO "ENCDR: %d\n", rotary_counter);
	return 0;
}

module_init(encdr_init);
module_exit(encdr_exit);








