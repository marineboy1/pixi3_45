
#ifndef _MTK_KPD_H_
#define _MTK_KPD_H_

#include <linux/kernel.h>
#include <cust_kpd.h>
#include <mach/mt_pm_ldo.h>
#include <linux/delay.h>
/* include PMIC header file */
#include <mach/mt_typedefs.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

#define KPD_PWRKEY_USE_EINT KPD_NO
#define KPD_PWRKEY_USE_PMIC KPD_YES
#define KPD_DRV_CTRL_BACKLIGHT	KPD_NO	/* retired, move to Lights framework */
#define KPD_BACKLIGHT_TIME	8	/* sec */
/* the keys can wake up the system and we should enable backlight */
#define KPD_BACKLIGHT_WAKE_KEY	\
{				\
	KEY_ENDCALL, KEY_POWER,	\
}

#define KPD_HAS_SLIDE_QWERTY	KPD_NO
#if KPD_HAS_SLIDE_QWERTY
static inline bool powerOn_slidePin_interface() {
	return hwPowerOn(MT65XX_POWER_LDO_VCAM_IO, VOL_2800, "Qwerty slide");
}
static inline bool powerOff_slidePin_interface(){
	return hwPowerDown(MT65XX_POWER_LDO_VCAM_IO, "Qwerty slide");
}
#endif
#define KPD_SLIDE_EINT		CUST_EINT_KPD_SLIDE_NUM
#define KPD_SLIDE_DEBOUNCE	CUST_EINT_KPD_SLIDE_DEBOUNCE_CN		/* ms */
#define KPD_SLIDE_POLARITY	CUST_EINT_KPD_SLIDE_POLARITY
#define KPD_SLIDE_SENSITIVE	CUST_EINT_KPD_SLIDE_SENSITIVE

#if KPD_DRV_CTRL_BACKLIGHT
extern void kpd_enable_backlight(void);
extern void kpd_disable_backlight(void);
extern void kpd_backlight_handler(bool pressed, u16 linux_keycode);
#else
#define kpd_enable_backlight()		do {} while (0)
#define kpd_disable_backlight()		do {} while (0)
#define kpd_backlight_handler(pressed, linux_keycode)	do {} while (0)
#endif

/* for META tool */
extern void kpd_set_backlight(bool onoff, void *val1, void *val2);

#if KPD_PWRKEY_USE_PMIC
void kpd_pwrkey_pmic_handler(unsigned long pressed);
#else
static inline void kpd_pwrkey_pmic_handler(unsigned long data){}
#endif

void kpd_pmic_rstkey_handler(unsigned long pressed);

/* [149730]Change to one key reboot by [bin.song.hz] at [2015.3.23] begin */
#define ONEKEY_REBOOT_NORMAL_MODE
//#define TWOKEY_REBOOT_NORMAL_MODE
#define ONEKEY_REBOOT_OTHER_MODE
//#define TWOKEY_REBOOT_OTHER_MODE
/* [149730]Change to one key reboot by [bin.song.hz] at [2015.3.23] end */
/* KPD_PMIC_RSTKEY_MAP is defined in cust_kpd.h */
/*[935914] for FCC/PTCRB authentication by [liming.zhu] at [20150401] bigen*/
#ifndef CONFIG_FCC_PTCRB_SUPPORT
// modify don't define LPRST in MINI sw for block LPRST function by ronghui.wang for Task begin
#ifndef TARGET_BUILD_MMITEST
#define KPD_PMIC_LPRST_TD 1 /* timeout period. 0: 8sec; 1: 11sec; 2: 14sec; 3: 5sec */
#endif
// modify don't define LPRST in MINI sw for block LPRST function by ronghui.wang for Task end
#endif 
/*[935914] for FCC/PTCRB authentication by [liming.zhu] at [20150401] end*/
#endif
