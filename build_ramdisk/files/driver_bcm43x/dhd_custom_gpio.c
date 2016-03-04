/*
* Customer code to add GPIO control during WLAN start/stop
* Copyright (C) 1999-2014, Broadcom Corporation
* 
*      Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the "GPL"),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the
* following added to such license:
* 
*      As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions of
* the license of that module.  An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* 
*      Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
* $Id: dhd_custom_gpio.c 466835 2014-04-01 20:44:55Z $
*/

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>

#include <wlioctl.h>
#include <wl_iw.h>

#include <asm/intel-mid.h>
#include <linux/gpio.h>

#if defined(SUPPORT_MULTIPLE_REVISION)
#include "bcmdevs.h"
#endif /* SUPPORT_MULTIPLE_REVISION */

#define WL_ERROR(x) printf x
#define WL_TRACE(x)

#ifdef CUSTOMER_HW
extern  void bcm_wlan_power_off(int);
extern  void bcm_wlan_power_on(int);
#endif /* CUSTOMER_HW */

#if defined(OOB_INTR_ONLY)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /* (BCMLXSDMMC)  */

#if defined(CUSTOMER_HW3)
#include <mach/gpio.h>
#endif

/* Customer specific Host GPIO defintion  */
static int dhd_oob_gpio_num = -1;

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

/* This function will return:
 *  1) return :  Host gpio interrupt number per customer platform
 *  2) irq_flags_ptr : Type of Host interrupt as Level or Edge
 *
 *  NOTE :
 *  Customer should check his platform definitions
 *  and his Host Interrupt spec
 *  to figure out the proper setting for his platform.
 *  Broadcom provides just reference settings as example.
 *
 */
int dhd_customer_oob_irq_map(void *adapter, unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#if defined(CUSTOMER_HW2) || defined(CUSTOMER_HW4)
	host_oob_irq = wifi_platform_get_irq_number(adapter, irq_flags_ptr);
	if (!wifi_platform_irq_is_fastirq(adapter)) {
		if (gpio_request(host_oob_irq, "bcm43xx_irq") < 0) {
			WL_ERROR(("%s: Error on gpio_request bcm43xx_irq: %d\n", __func__, host_oob_irq));
			return -1;
		}
		if (gpio_direction_input(host_oob_irq) < 0) {
			WL_ERROR(("%s: Error on gpio_direction_input\n", __func__));
			return -1;
		}
		if (gpio_set_debounce(host_oob_irq, 0) < 0) {
			WL_ERROR(("%s: Error on gpio_set_debounce\n", __func__));
			return -1;
		}
		dhd_oob_gpio_num = host_oob_irq;
		host_oob_irq = gpio_to_irq(host_oob_irq);
	}
#else
#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif /* CUSTOMER_OOB_GPIO_NUM */

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#if defined CUSTOMER_HW3
	gpio_request(dhd_oob_gpio_num, "oob irq");
	host_oob_irq = gpio_to_irq(dhd_oob_gpio_num);
	gpio_direction_input(dhd_oob_gpio_num);
#endif /* CUSTOMER_HW3 */
#endif /* CUSTOMER_HW2 || CUSTOMER_HW4 */

	return (host_oob_irq);
}

void dhd_customer_oob_irq_unmap(void)
{
	if (dhd_oob_gpio_num > 0)
		gpio_free(dhd_oob_gpio_num);
}
#endif  /* OOB_INTR_ONLY */

/* Customer function to control hw specific wlan gpios */
int
dhd_customer_gpio_wlan_ctrl(void *adapter, int onoff)
{
	int err = 0;

	return err;
}

#ifdef GET_CUSTOM_MAC_ENABLE
#define MAC_ADDRESS_LEN 12

int wifi_get_mac_addr_intel(unsigned char *buf){
	int ret = 0;
	int i;
	struct file *fp = NULL;
	unsigned char c_mac[MAC_ADDRESS_LEN];
	char fname[]="/config/wifi/mac.txt";

	WL_TRACE(("%s Enter\n", __FUNCTION__));

	fp = dhd_os_open_image(fname);
	if (fp== NULL){
		WL_ERROR(("%s: unable to open %s\n",__FUNCTION__, fname));
		return 1;
	}

	if ( dhd_os_get_image_block(c_mac, MAC_ADDRESS_LEN, fp) != MAC_ADDRESS_LEN ){
		WL_ERROR(("%s: Error on reading mac address from %s \n",__FUNCTION__, fname));
		dhd_os_close_image(fp);
		return 1;
	}
	dhd_os_close_image(fp);

	for (i =0; i< MAC_ADDRESS_LEN ; i+=2){
		c_mac[i] = bcm_isdigit(c_mac[i]) ? c_mac[i]-'0' : bcm_toupper(c_mac[i])-'A'+10;
		c_mac[i+1] = bcm_isdigit(c_mac[i+1]) ? c_mac[i+1]-'0' : bcm_toupper(c_mac[i+1])-'A'+10;

		buf[i/2] = c_mac[i]*16 + c_mac[i+1];
	}

	WL_TRACE(("%s: read from file mac address: %x:%x:%x:%x:%x:%x\n",
			 __FUNCTION__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]));

	return ret;
}


/* Function to get custom MAC address */
int
dhd_custom_get_mac_address(unsigned char *buf)
{
	int ret = 0;

	WL_TRACE(("%s Enter\n", __FUNCTION__));
	if (!buf)
		return -EINVAL;

	/* Customer access to MAC address stored outside of DHD driver */
#if (defined(CUSTOMER_HW2) || defined(CUSTOMER_HW10)) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	ret = wifi_get_mac_addr_intel(buf);
#endif

#ifdef EXAMPLE_GET_MAC
	/* EXAMPLE code */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /* EXAMPLE_GET_MAC */

	return ret;
}
#endif /* GET_CUSTOM_MAC_ENABLE */

/* Customized Locale table : OPTIONAL feature */
const struct cntry_locales_custom translate_custom_table[] = {
/* Table should be filled out based on custom platform regulatory requirement */
/* Table must match BRCM firmware tables */
/*
 * Only modify the Country Code which have a problem regarding Intel requirement.
 * Currently, on BCM4335/4339 when using "FR" it is required to use "FR/5" to enable 802.11ac.
 *
 * In addition, since Finland team is also testing 802.11ac,
 * it is required to use "FR/5" instead of "FI".
 * No issue to use FR/5 instead of FI since they are EU...
 *
 * Fix Korea country code to "KR/4" for 43340, 43241, 4335/4339, 4354 but still use "KR/0" for 4334.
 *
 * Fix US country code to "US/0" for 4334, 43340, 43241, 4335/4339, 4354. It enable 5Ghz channels.
 */
#ifdef BOARD_INTEL
#if defined(SUPPORT_MULTIPLE_REVISION)
	{BCM4334_CHIP_ID,  "US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{BCM4324_CHIP_ID,  "US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{BCM43341_CHIP_ID, "US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{BCM4335_CHIP_ID,  "US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{BCM4339_CHIP_ID,  "US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{BCM4354_CHIP_ID,  "US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{BCM4335_CHIP_ID,  "FR", "FR", 5}, /* Translate FR into FR/5 in order to enable 802.11ac */
	{BCM4339_CHIP_ID,  "FR", "FR", 5}, /* Translate FR into FR/5 in order to enable 802.11ac */
	{BCM4354_CHIP_ID,  "FR", "FR", 5}, /* Translate FR into FR/5 in order to enable 802.11ac */
	{BCM4335_CHIP_ID,  "FI", "FR", 5}, /* Translate FI into FR/5 in order to enable 802.11ac */
	{BCM4339_CHIP_ID,  "FI", "FR", 5}, /* Translate FI into FR/5 in order to enable 802.11ac */
	{BCM4354_CHIP_ID,  "FI", "FR", 5}, /* Translate FI into FR/5 in order to enable 802.11ac */
	{BCM4324_CHIP_ID,  "KR", "KR", 4}, /* Enable correct 5Ghz channel */
	{BCM43341_CHIP_ID, "KR", "KR", 4}, /* Enable correct 5Ghz channel */
	{BCM4335_CHIP_ID,  "KR", "KR", 4}, /* Enable correct 5Ghz channel */
	{BCM4339_CHIP_ID,  "KR", "KR", 4}, /* Enable correct 5Ghz channel */
	{BCM4354_CHIP_ID,  "KR", "KR", 4}, /* Enable correct 5Ghz channel */
	{BCM4339_CHIP_ID,  "CN", "CN", 24}, /* Enable correct 5Ghz channel for CN */
	{BCM4354_CHIP_ID,  "CN", "CN", 24}, /* Enable correct 5Ghz channel for CN */
#else /* SUPPORT_MULTIPLE_REVISION */
	{"US", "US", 0}, /* Translate US into US/0 in order to enable 5GHz band */
	{"FR", "FR", 5}, /* Translate FR into FR/5 in order to enable 802.11ac */
	{"FI", "FR", 5}, /* Translate FI into FR/5 in order to enable 802.11ac */
	{"KR", "KR", 4}, /* Enable correct 5Ghz channel */
#endif /* SUPPORT_MULTIPLE_REVISION */
#endif /* BOARD_INTEL */
};

/* Customized Locale convertor
*  input : ISO 3166-1 country abbreviation
*  output: customized cspec
*/
void get_customized_country_code(void *adapter, char *country_iso_code, wl_country_t *cspec)
{
#if defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))

	int size, i;

	size = ARRAYSIZE(translate_custom_table);

	if (cspec == 0)
		 return;

	if (size == 0)
		 return;

	for (i = 0; i < size; i++) {
		uint chip = 0;
#if defined(SUPPORT_MULTIPLE_REVISION)
		chip = bcm_get_chip_id();
		if (chip != translate_custom_table[i].chip_id) {
			continue;
		}
#endif
		if (strcmp(country_iso_code, translate_custom_table[i].iso_abbrev) == 0) {
			memcpy(cspec->ccode,
				translate_custom_table[i].custom_locale, WLC_CNTRY_BUF_SZ);
			cspec->rev = translate_custom_table[i].custom_locale_rev;
			printk(KERN_ERR "%s: chip = %x => ccode = %s, regrev = %d\n", __func__, chip, cspec->ccode, cspec->rev);
			return;
		}
	}
	return;
#endif /* defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)) */
}
