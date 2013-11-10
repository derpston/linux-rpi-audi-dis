#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "gpio.h"

#define MAX_PROC_SIZE 20
#define PROC_FILE_NAME "dis"
static char message[MAX_PROC_SIZE];
static struct proc_dir_entry *proc_write_entry;

static struct hrtimer hr_timer;
ktime_t ktime;

// Sending states
#define IDLE 0
#define PRE_ENABLE 1
#define PRE_SEND 2
#define SENDING 3
#define SENDING_CLOCK 4

int state = IDLE;
int msgindex = 0;
int bitindex = 0;

// pin assignments
#define CLOCK   14
#define DATA    15
#define ENABLE  18

// Calculate the checksum of a message.
char getchecksum(char *message)
{
    char checksum = 0;
    int i;

    for(i = 0; i < MAX_PROC_SIZE && message[i] != 0; i++)
        checksum += message[i];

    return (checksum ^ 0xff) & 0xff;
}

enum hrtimer_restart hrtimer_handler(struct hrtimer *timer)
{
    ktime_t kt_now;
    char chr;
    bool bit;

    switch(state)
    {
        case PRE_ENABLE:
            clear(ENABLE);

            ktime = ktime_set(0, 400000);
            kt_now = hrtimer_cb_get_time(&hr_timer);
            hrtimer_forward(&hr_timer, kt_now, ktime);

            state = PRE_SEND;
            return HRTIMER_RESTART;
        break;

        case PRE_SEND:
            set(ENABLE);
            state = SENDING;
            ktime = ktime_set(0, 100);
            kt_now = hrtimer_cb_get_time(&hr_timer);
            hrtimer_forward(&hr_timer, kt_now, ktime);
            return HRTIMER_RESTART;
        break;

        case SENDING:
            chr = message[msgindex];

            if(chr == 0 || msgindex == MAX_PROC_SIZE)
            {
                state = IDLE;
                clear(ENABLE);
                set(DATA);
                set(CLOCK);
                return HRTIMER_NORESTART;
            }
            else
            {
                bit = (chr >> (7 - bitindex)) & 1;

                if(!bit)
                    set(DATA);
                else
                    clear(DATA);
                
                bitindex++;

                if(bitindex == 8)
                {
                    bitindex = 0;
                    msgindex++;
                }

                clear(CLOCK);
                state = SENDING_CLOCK;
                ktime = ktime_set(0, 200000);
                kt_now = hrtimer_cb_get_time(&hr_timer);
                hrtimer_forward(&hr_timer, kt_now, ktime);
                return HRTIMER_RESTART;
            }
        break;

        case SENDING_CLOCK:
            set(CLOCK);
            state = SENDING;
            ktime = ktime_set(0, 100000);
            kt_now = hrtimer_cb_get_time(&hr_timer);
            hrtimer_forward(&hr_timer, kt_now, ktime);
            return HRTIMER_RESTART;
        break;
    }

    // Should never get here, but just in case stop the timer.
    return HRTIMER_NORESTART;
}

int write_proc(struct file *file,const char *buf,int count,void *data )
{
    char checksum;
    static char input[MAX_PROC_SIZE];

    // Prevent content of previous messages leaking into this one.
    memset(message, 0, MAX_PROC_SIZE);
    memset(input, 0, MAX_PROC_SIZE);

    // Already transmitting! Reject this message.
    if(state != IDLE)
        return -EFAULT;

    // Read the new message from userspace.
    if(count > MAX_PROC_SIZE)
        count = MAX_PROC_SIZE;
    if(copy_from_user(input, buf, count))
        return -EFAULT;
    
    // Header byte, space-padded message, control byte.
    sprintf(message, "\xf0%-15s\x1c", input);

    // Calculate the checksum.
    checksum = getchecksum(message);

    // Add the checksum to the message.
    message[17] = checksum;
    message[18] = 0;
    
    // Prepare to transmit.
    msgindex = 0;
    state = PRE_ENABLE;
    set(ENABLE);
    
    // Enable the timer.
    ktime = ktime_set(0, 500000);
    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hr_timer.function = &hrtimer_handler;
    hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);

    // Tell the user how many bytes were accepted.
    return count;
}


int __init dis_init(void)
{
    proc_write_entry = create_proc_entry(PROC_FILE_NAME, 0666, NULL);
    proc_write_entry->write_proc = write_proc;

    // Configure the pins as outputs and set their initial state.
    set_output(CLOCK);
    set_output(DATA);
    set_output(ENABLE);
    set(CLOCK);
    set(DATA);
    clear(ENABLE);
	
	printk( KERN_INFO "DIS LCD module loaded.");

	return 0;
}

void __exit dis_exit( void )
{
    clear(CLOCK);
    clear(DATA);
    clear(ENABLE);

    hrtimer_cancel(&hr_timer);
    remove_proc_entry(PROC_FILE_NAME, NULL);

	printk( KERN_INFO "DIS LCD module unloaded.");
}

module_init(dis_init);
module_exit(dis_exit);

MODULE_AUTHOR("Derpston");
MODULE_DESCRIPTION("Audi DIS-proc interface");
MODULE_LICENSE("Dual BSD/GPL");

