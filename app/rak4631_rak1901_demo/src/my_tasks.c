#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "my_tasks.h"
#include "rak_sidewalk.h"

#include <sm_task.h>

#define VERSION (8)

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(my_tasks);

#define MEASUREMENT_INTERVAL    (60*1000) // Check sensor status at every 10 secons without

static struct k_timer timer_reading_sensor;

/* Thread configuration */
#define STACKSIZE 	8192

static struct k_thread reading_sensor_id;
K_THREAD_STACK_DEFINE(reading_sensor_stack, STACKSIZE);
/* ----- */

/* Semaphores and Mutexes */
static struct k_sem reading_sensor_sem;
/* ----- */

volatile bool is_button_status_changed = false;

static void reading_sensor_thread_handler(void *arg1, void *arg2, void *arg3)
{  
    while(1)
    {
        if(k_sem_take(&reading_sensor_sem, K_FOREVER /*K_MSEC(50)*/) == 0)
        {  
            sidewalk_payload_t payload = rak_sidewalk_get_payload();
            LOG_INF("temp: %f", payload.temp);
            LOG_INF("hum: %f", payload.hum);
            if (sm_app_state_get() == DEMO_APP_STATE_NOTIFY_SENSOR_DATA && sm_is_sidewalk_ready())
            {
                enum event_type event = EVENT_NOTIFICATION_TIMER_FIRED;
                sm_main_task_msg_q_write(event);
                LOG_INF("Send over sidewalk!");
            }
            else
            {
                LOG_INF("Sidewalk link is not ready!!!");
                LOG_INF("State: %d, Ready: %d", sm_app_state_get(), sm_is_sidewalk_ready());


            }

        }
    }    
}

static void timer_reading_sensor_expire(struct k_timer *timer)
{
	k_sem_give(&reading_sensor_sem);
}
static void timer_reading_sensor_stop(struct k_timer *timer)
{
	LOG_INF("Timer stopped.");
}

void my_tasks_init(void)
{
    k_sem_init(&reading_sensor_sem, 0, 1);
    (void)k_thread_create(&reading_sensor_id, reading_sensor_stack, K_THREAD_STACK_SIZEOF(reading_sensor_stack) , reading_sensor_thread_handler, NULL, NULL, NULL, K_PRIO_PREEMPT(4) , 0, K_NO_WAIT);
    k_thread_name_set(&reading_sensor_id, "reading_sensor_id");
    k_timer_init(&timer_reading_sensor, timer_reading_sensor_expire, timer_reading_sensor_stop);

    k_timer_start(&timer_reading_sensor, K_MSEC(MEASUREMENT_INTERVAL), K_MSEC(MEASUREMENT_INTERVAL));
}
