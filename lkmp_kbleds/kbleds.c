/*
 * driver example to play with keyboard leds
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/vt_kern.h>
#include <linux/console_struct.h>

struct timer_list my_timer;
struct tty_driver *my_driver;
char my_led_status = 0;

#define BLINK_DELAY  HZ/5
#define ALL_LEDS_ON  0x07
#define RESTORE_LEDS 0xff

static void my_timer_func(unsigned long ptr)
{
	int *pst = (int *)ptr;

	if(*pst == ALL_LEDS_ON)
		*pst = RESTORE_LEDS;
	else
		*pst = ALL_LEDS_ON;

	(my_driver->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED,
			*pst);

	my_timer.expires = jiffies + BLINK_DELAY;
	add_timer(&my_timer);
}

static int __init kbleds_init(void)
{
	int i;

	printk (KERN_INFO "kbleds: loading\n");
	printk (KERN_INFO "kbleds: fgconsole is %x\n", fg_console);

	for (i = 0; i < MAX_NR_CONSOLES; ++i)
	{
		if(!vc_cons[i].d)
			break;
	}

	printk (KERN_INFO "kbleds: finished scanning: %d\n", fg_console);

	my_driver = vc_cons[fg_console].d->vc_tty->driver;
	printk (KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);

	init_timer(&my_timer);
	my_timer.function = my_timer_func;
	my_timer.data = (unsigned long)&my_led_status;
	my_timer.expires = jiffies + BLINK_DELAY;
	add_timer (&my_timer);

	return 0;
}

static void __exit kbleds_cleanup(void)
{
	printk(KERN_INFO "kbleds: unloading...\n");
	del_timer(&my_timer);
	(my_driver->ops->ioctl)(vc_cons[fg_console].d->vc_tty,
					NULL, KDSETLED, RESTORE_LEDS);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);
