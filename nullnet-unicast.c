#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <random.h> // for generating random values
#include <string.h>
#include <stdio.h> /* For printf() */

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (3 * CLOCK_SECOND)
static linkaddr_t dest_addr = {{ 0x01, 0x01, 0x01, 0x00, 0x01, 0x74, 0x12, 0x00 }};

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x01, 0x01, 0x00, 0x01, 0x74, 0x12, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

static struct{
unsigned short temp;
	 }values; // struct for data
	 
static uint8_t numOfMotes = 4; // number of slave motes
unsigned sum = 0; // sum variable 
unsigned tracker = 0;    // keeps the track of last values
unsigned avg = 0; // Avg temperature

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{	
	if(tracker < numOfMotes){
    unsigned temp_val; // temperature coming from nodes 
    memcpy(&temp_val, data, sizeof(temp_val)); 
	sum += temp_val; 
    LOG_INFO("Received %u from ", temp_val);
    LOG_INFO_LLADDR(src);
	LOG_INFO_("\n");
	tracker++;
	}else{
	avg = sum / numOfMotes; //  calculating the Average Temperature
	LOG_INFO("Average Temperature is %u ", avg);
	LOG_INFO_("\n");
	//reseting the values for next iteration
	tracker = 0;
	avg = 0;
	sum = 0;
	}	
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  PROCESS_BEGIN();

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */
  /* Initialize NullNet */
  
  nullnet_buf = (uint8_t *)&values; // address of struct passign to nullnet_buf
  nullnet_len = sizeof(values);
  nullnet_set_input_callback(input_callback); // setting the destination
  if(!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) 
  {
    etimer_set(&periodic_timer, SEND_INTERVAL);
    
	while(1) {
		values.temp =  (random_rand()%100); // Generating random temperature values every 5 seconds
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		LOG_INFO_LLADDR(&linkaddr_node_addr);
		LOG_INFO_("\n");
		NETSTACK_NETWORK.output(&dest_addr); // sending the values to destination node
		etimer_reset(&periodic_timer); 
		}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
