#include "ets_sys.h"		// Core system stuff??
#include "osapi.h"			// User-level stuff??
#include "gpio.h"			// GPIO control.
#include "os_type.h"		// Type definitions for OS stuff??
#include "mem.h"			// Memory management, os_memcpy, etc?
#include "user_interface.h"	// OS and WiFi stuff?
#include "ip_addr.h"		// Helpers for IP address definition and display.
#include "espconn.h"		// TCP and UDP stuff.

#include "user_config.h"	// Secrets (i.e. wifi password).


#define BIT_0	1
#define BIT_1	2
#define BIT_2	4


enum {
	TIMER_WIFI = 1,
	TIMER_LED_OFF = 2,
};

os_timer_t main_timer;

static void ICACHE_FLASH_ATTR handle_timer_event(void*);

static void ICACHE_FLASH_ATTR handle_wifi_connected()
{
	struct ip_info ip;
	wifi_get_ip_info(STATION_IF, &ip);
	os_printf(
		"IP:" IPSTR " MASK:" IPSTR " GW:" IPSTR "\n",
		IP2STR(&ip.ip), IP2STR(&ip.netmask), IP2STR(&ip.gw)
	);
	// Flash LED attached to GPIO2.
	// Set GPIO2 low, which turns on the LED:
	gpio_output_set(0, BIT_2, 0, 0);
	os_timer_setfn(&main_timer, handle_timer_event, (void*)TIMER_LED_OFF);
	os_timer_arm(&main_timer, 200, false); // false => one-shot.
}


static void ICACHE_FLASH_ATTR handle_timer_event(void* arg)
{
	switch ((int)arg)
	{
		case TIMER_LED_OFF:
		{
			// Set GPIO2 high, which turns the LED off:
			gpio_output_set(BIT_2, 0, 0, 0);
			os_timer_disarm(&main_timer);
			break;
		}
		case TIMER_WIFI:
		{
			// Check if wifi is connected yet:
			uint8 status = wifi_station_get_connect_status();
			switch (status)
			{
				case STATION_WRONG_PASSWORD:
				case STATION_NO_AP_FOUND:
				case STATION_CONNECT_FAIL:
				{
					os_printf("Wifi connection failed. Status: %d\n", status);
					os_timer_disarm(&main_timer);
					break;
				}
				case STATION_GOT_IP:
				{
					os_printf("Wifi connected\n");
					os_timer_disarm(&main_timer);
					handle_wifi_connected();
					break;
				}
			}
			break;
		}
		default:
		{
			os_printf("Unexpected timer event 0x%x; stopping main_timer.\n", arg);
			os_timer_disarm(&main_timer);
			break;
		}
	}
}

static void ICACHE_FLASH_ATTR app_main()
{
	// Connect to wifi:
	char ssid[32] = SSID;
	char password[64] = SSID_PASSWORD;
	struct station_config conf;
	conf.bssid_set = 0;
	os_memcpy(&conf.ssid, ssid, sizeof(ssid));
	os_memcpy(&conf.password, password, sizeof(password));
	wifi_station_set_config(&conf);
	os_printf("Attempting WiFi connect to: %s\n", ssid);
	wifi_station_connect();

	// Let a timer check every 200ms to catch when we're connected.
	os_timer_disarm(&main_timer);
	os_timer_setfn(&main_timer, handle_timer_event, (void*)TIMER_WIFI);
	os_timer_arm(&main_timer, 200, true); // true => repeat.
}

void user_init()
{
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_printf("\n\n01-tcp-server boot\n");

	gpio_init();
	// Configure GPIO2 pin to actually function as GPIO2:
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	// Configure GPIO2 to be output, and set it high
	// (which turns the LED OFF):
	gpio_output_set(BIT_2, 0, BIT_2, 0);

	// Disable OS debugging output:
	//system_set_os_print(0);

	wifi_station_set_auto_connect(0);
	// Configure wifi for "Station" (client) mode:
	wifi_set_opmode_current(STATION_MODE);

	// Set a callback for when full system init is done:
	system_init_done_cb(app_main);
}
