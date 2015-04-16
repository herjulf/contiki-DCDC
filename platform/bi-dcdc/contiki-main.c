/*
 * Copyright (c) 2013, KTH, Royal Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/etimer.h>
#include <sys/autostart.h>
#include <sys/clock.h>
#include <contiki-net.h>
#include <net/dhcpc.h>
#include "contiki-conf.h"
#include "debug-uart.h"
#include "emac-driver.h"
#include <net/uip-debug.h>
#include "dev/leds-arch.h"
#include "dev/leds.h"
#include "control.h"
#include "system_LPC17xx.h"

unsigned int idle_count = 0;

float v_in_corr = 1.0;
float v_out_corr = 1.0;
float io_corr_k = 1.0;
float io_corr_l = 1.0;

void start_bangbang(void) {
	GPIOInit();
	TimerInit();
	ValueInit();
	VSC_Init();
	ADCInit();
}

int main()
{
  unsigned char lladdr[8];
  int i;

  debug_uart_setup();
  printf("Initializing clocks\n");
  clock_init();
  printf("The system main CPU clock speed is %dHz\n",SystemCoreClock);
  printf("Initializing processes\n");
  process_init();

  #if defined (PLATFORM_HAS_LEDS)
  printf("Initializing leds\n");
  leds_arch_init();
  #endif

  printf("Starting bangbang\n");
  start_bangbang();

  printf("Starting etimers\n");
  process_start(&etimer_process, NULL);
  printf("Starting EMAC service\n");
  process_start(&emac_lpc1768, NULL);

  printf("Initializing i2c and EUI64/EEPROM\n");
  eeprom_init();
  eeprom_read(0xF8, &lladdr, 8); /* See EEPROM datasheet for addr */

#if UIP_CONF_IPV6
  //uip_ds6_addr_t *lladdr;
  //lladdr = uip_ds6_get_link_local(-1);

  printf("EEPROM EUI64 address: ");
  for(i=0; i<8; i++)
    printf("%02X", lladdr[i]);
  printf("\n");

  // set MAC address according to EEPROM EUI64 address
  uip_lladdr.addr[0] = lladdr[0];
  uip_lladdr.addr[1] = lladdr[1];
  uip_lladdr.addr[2] = lladdr[2];
  uip_lladdr.addr[3] = lladdr[5];
  uip_lladdr.addr[4] = lladdr[6];
  uip_lladdr.addr[5] = lladdr[7];

  // Configure global IPv6 address
  //uip_ipaddr_t ipaddr;
  //uip_ip6addr(&ipaddr, 0x2000, 0, 0, 0, 0, 0, 0, 0);
  //uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  //uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
  //printf("Manual global IPv6 address: ");
  //uip_debug_ipaddr_print(&ipaddr);
  //printf("\n");

  // Add prefix of manually set global address to prefix list
  //uip_ds6_prefix_add(&ipaddr, 64, 0);

#else
  // init MAC address
  uip_ethaddr.addr[0] = EMAC_ADDR0;
  uip_ethaddr.addr[1] = EMAC_ADDR1;
  uip_ethaddr.addr[2] = EMAC_ADDR2;
  uip_ethaddr.addr[3] = EMAC_ADDR3;
  uip_ethaddr.addr[4] = EMAC_ADDR4;
  uip_ethaddr.addr[5] = EMAC_ADDR5;
  uip_setethaddr(uip_ethaddr);

  uip_ipaddr(&addr, 192, 168, 1, 5);
  printf("IP Address:  %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_sethostaddr(&addr);

  uip_ipaddr(&addr, 255, 255, 255, 0);
  printf("Subnet Mask: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_setnetmask(&addr);

  uip_ipaddr(&addr, 192, 168, 1, 1);
  printf("Def. Router: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_setdraddr(&addr);
#endif

  if (lladdr[7] == 0x42  && lladdr[6] == 0x5c ) {
      printf("Setting ADC corrections\n");
      v_in_corr = 0.9761;
      v_out_corr = 0.96705;
      io_corr_k = 1.03138;
      io_corr_l = 0.245237;
  }
  if (lladdr[7] == 0xe7  && lladdr[6] == 0x48 ) {
    printf("Setting ADC corrections\n");
    v_in_corr = 0.9793;
    v_out_corr = 0.9659;
    io_corr_k = 0.590486;
    io_corr_l = 0.0529743;
  }

  printf("Starting TCP/IP service\n");
  process_start(&tcpip_process, NULL);          // invokes uip_init();

  autostart_start(autostart_processes);

  printf("Processes running\n");
  while (1){

      do
        {
        }
      while (process_run() > 0);
      idle_count++;
      /* Idle! */
      /* Stop processor clock */
      /* asm("wfi"::); */
    }
  return 0;
}

void
uip_log(char *m)
{
  printf("uIP: '%s'\n", m);
}
// required for #define LOG_CONF_ENABLED
void
log_message(const char *part1, const char *part2)
{
  printf("log: %s: %s\n", part1, part2);
}
