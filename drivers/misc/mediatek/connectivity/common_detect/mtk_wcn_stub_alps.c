




#define CMB_STUB_LOG_INFO(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#define CMB_STUB_LOG_WARN(fmt, arg...) printk(KERN_WARNING fmt, ##arg)
#define CMB_STUB_LOG_DBG(fmt, arg...) printk(KERN_DEBUG fmt, ##arg)


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <mach/mtk_wcn_cmb_stub.h>

//#include <cust_gpio_usage.h>
//#include <mach/mt6573_pll.h> /* clr_device_working_ability, MT65XX_PDN_PERI_UART3, DEEP_IDLE_STATE, MT65XX_PDN_PERI_MSDC2 */

#include <mach/mt_dcm.h>









#ifndef MTK_WCN_CMB_FOR_SDIO_1V_AUTOK
#define MTK_WCN_CMB_FOR_SDIO_1V_AUTOK 0
#endif

#if MTK_WCN_CMB_FOR_SDIO_1V_AUTOK
struct work_struct *g_sdio_1v_autok_wk = NULL;
#endif
int gConnectivityChipId = -1;

#ifdef MTK_WCN_COMBO_CHIP_SUPPORT
char *wmt_uart_port_desc = "ttyMT2";  // current used uart port name, default is "ttyMT2", will be changed when wmt driver init
EXPORT_SYMBOL(wmt_uart_port_desc);
#endif


static wmt_aif_ctrl_cb cmb_stub_aif_ctrl_cb = NULL;
static wmt_func_ctrl_cb cmb_stub_func_ctrl_cb = NULL;
static wmt_thermal_query_cb cmb_stub_thermal_ctrl_cb = NULL;
static CMB_STUB_AIF_X cmb_stub_aif_stat = CMB_STUB_AIF_0;
static wmt_deep_idle_ctrl_cb cmb_stub_deep_idle_ctrl_cb = NULL;
#if MTK_WCN_CMB_FOR_SDIO_1V_AUTOK
static wmt_get_drv_status cmb_stub_drv_status_ctrl_cb = NULL;
#endif
static wmt_func_do_reset cmb_stub_do_reset_cb = NULL;
 #if 0
static CMB_STUB_AIF_X audio2aif[] = {
    [COMBO_AUDIO_STATE_0] = CMB_STUB_AIF_0,
    [COMBO_AUDIO_STATE_1] = CMB_STUB_AIF_1,
    [COMBO_AUDIO_STATE_2] = CMB_STUB_AIF_2,
    [COMBO_AUDIO_STATE_3] = CMB_STUB_AIF_3,
};
#endif

#ifdef MTK_WCN_COMBO_CHIP_SUPPORT
extern unsigned int mtk_uart_pdn_enable(char *port, int enable);
#endif

#if MTK_WCN_CMB_FOR_SDIO_1V_AUTOK
static void mtk_wcn_cmb_stub_1v_autok_work (struct work_struct *work)
{
	CMB_STUB_LOG_WARN("++enter++\n");
	mtk_wcn_cmb_stub_func_ctrl(11,1);
	mtk_wcn_cmb_stub_func_ctrl(11,0);
	CMB_STUB_LOG_WARN("--exit--\n");
}
static int mtk_wcn_cmb_stub_drv_status(unsigned int type)
{
	if(cmb_stub_drv_status_ctrl_cb)
		return (*cmb_stub_drv_status_ctrl_cb)(type);
	else{
		CMB_STUB_LOG_WARN("cmb_stub_drv_status_ctrl_cb is NULL\n");
		return -1;
	}
}

int mtk_wcn_cmb_stub_1vautok_for_dvfs(void)
{
	int wmt_status;
	CMB_STUB_LOG_WARN("DVFS driver call sdio 1v autok\n");

	wmt_status = mtk_wcn_cmb_stub_drv_status(4);
	CMB_STUB_LOG_WARN("current mt6630 status is %d\n",wmt_status);
	if(0 == wmt_status){
		if(g_sdio_1v_autok_wk)
			schedule_work(g_sdio_1v_autok_wk);
		else
			CMB_STUB_LOG_WARN("g_sdio_1v_autok_wk is NULL\n");
	}else if((2 == wmt_status) || (1 == wmt_status)){
		CMB_STUB_LOG_WARN("mt6630 is on state,skip AUTOK\n");
	}else{
		CMB_STUB_LOG_WARN("mt6630 is unknow state(%d)\n",wmt_status);
	}

	return wmt_status;

}
#endif
int
mtk_wcn_cmb_stub_reg (P_CMB_STUB_CB p_stub_cb)
{
    if ( (!p_stub_cb )
        || (p_stub_cb->size != sizeof(CMB_STUB_CB)) ) {
        CMB_STUB_LOG_WARN( "[cmb_stub] invalid p_stub_cb:0x%p size(%d)\n",
            p_stub_cb, (p_stub_cb) ? p_stub_cb->size: 0);
        return -1;
    }

    CMB_STUB_LOG_DBG( "[cmb_stub] registered, p_stub_cb:0x%p size(%d)\n",
        p_stub_cb, p_stub_cb->size);

    cmb_stub_aif_ctrl_cb = p_stub_cb->aif_ctrl_cb;
    cmb_stub_func_ctrl_cb = p_stub_cb->func_ctrl_cb;
    cmb_stub_thermal_ctrl_cb = p_stub_cb->thermal_query_cb;
    cmb_stub_deep_idle_ctrl_cb = p_stub_cb->deep_idle_ctrl_cb;
    cmb_stub_do_reset_cb = p_stub_cb->wmt_do_reset_cb;
#if MTK_WCN_CMB_FOR_SDIO_1V_AUTOK
	cmb_stub_drv_status_ctrl_cb = p_stub_cb->get_drv_status_cb;
	g_sdio_1v_autok_wk = vmalloc(sizeof(struct work_struct));
	if (!g_sdio_1v_autok_wk) {
		CMB_STUB_LOG_WARN("vmalloc work_struct(%zd) fail\n", sizeof(struct work_struct));
	}else{
		INIT_WORK(g_sdio_1v_autok_wk, mtk_wcn_cmb_stub_1v_autok_work);
	}

#endif

    return 0;
}

int
mtk_wcn_cmb_stub_unreg (void)
{
    cmb_stub_aif_ctrl_cb = NULL;
    cmb_stub_func_ctrl_cb = NULL;
	cmb_stub_thermal_ctrl_cb = NULL;
	cmb_stub_deep_idle_ctrl_cb = NULL;
    cmb_stub_do_reset_cb = NULL;
    CMB_STUB_LOG_INFO("[cmb_stub] unregistered \n"); /* KERN_DEBUG */

#if MTK_WCN_CMB_FOR_SDIO_1V_AUTOK
	if(g_sdio_1v_autok_wk){
		vfree(g_sdio_1v_autok_wk);
		g_sdio_1v_autok_wk = NULL;
	}
#endif

    return 0;
}

/* stub functions for kernel to control audio path pin mux */
int mtk_wcn_cmb_stub_aif_ctrl (CMB_STUB_AIF_X state, CMB_STUB_AIF_CTRL ctrl)
{
    int ret;

    if ( (CMB_STUB_AIF_MAX <= state)
        || (CMB_STUB_AIF_CTRL_MAX <= ctrl) ) {

        CMB_STUB_LOG_WARN("[cmb_stub] aif_ctrl invalid (%d, %d)\n", state, ctrl);
        return -1;
    }

    /* avoid the early interrupt before we register the eirq_handler */
    if (cmb_stub_aif_ctrl_cb){
        ret = (*cmb_stub_aif_ctrl_cb)(state, ctrl);
        CMB_STUB_LOG_INFO( "[cmb_stub] aif_ctrl_cb state(%d->%d) ctrl(%d) ret(%d)\n",
            cmb_stub_aif_stat , state, ctrl, ret); /* KERN_DEBUG */

        cmb_stub_aif_stat = state;
        return ret;
    }
    else {
        CMB_STUB_LOG_WARN("[cmb_stub] aif_ctrl_cb null \n");
        return -2;
    }
}


void mtk_wcn_cmb_stub_func_ctrl (unsigned int type, unsigned int on) {
    if (cmb_stub_func_ctrl_cb) {
        (*cmb_stub_func_ctrl_cb)(type, on);
    }
    else {
        CMB_STUB_LOG_WARN("[cmb_stub] func_ctrl_cb null \n");
    }
}

signed long mtk_wcn_cmb_stub_query_ctrl()
{
	signed long temp = 0;

	if(cmb_stub_thermal_ctrl_cb)
	{
		temp = (*cmb_stub_thermal_ctrl_cb)();
	}
	else
	{
		CMB_STUB_LOG_WARN("[cmb_stub] thermal_ctrl_cb null\n");
	}

	return temp;
}

/*platform-related APIs*/
//void clr_device_working_ability(UINT32 clockId, MT6573_STATE state);
//void set_device_working_ability(UINT32 clockId, MT6573_STATE state);

static int
_mt_combo_plt_do_deep_idle(COMBO_IF src, int enter) {
    int ret = -1;

#if 0
    const char *combo_if_name[] =
    {   "COMBO_IF_UART",
        "COMBO_IF_MSDC"
    };
#endif

    if(src != COMBO_IF_UART && src!= COMBO_IF_MSDC && src != COMBO_IF_BTIF){
        CMB_STUB_LOG_WARN("src = %d is error\n", src);
        return ret;
    }
#if 0
    if(src >= 0 && src < COMBO_IF_MAX){
        CMB_STUB_LOG_INFO("src = %s, to enter deep idle? %d \n",
            combo_if_name[src],
            enter);
    }
#endif
    /*TODO: For Common SDIO configuration, we need to do some judgement between STP and WIFI
            to decide if the msdc will enter deep idle safely*/

    switch(src){
        case COMBO_IF_UART:
            if(enter == 0){
                //clr_device_working_ability(MT65XX_PDN_PERI_UART3, DEEP_IDLE_STATE);
                //disable_dpidle_by_bit(MT65XX_PDN_PERI_UART2);
#ifdef MTK_WCN_COMBO_CHIP_SUPPORT
                ret = mtk_uart_pdn_enable(wmt_uart_port_desc, 0);
                if (ret < 0) {
                    CMB_STUB_LOG_WARN("[CMB] %s exit deep idle failed\n", wmt_uart_port_desc);
                }
#endif
            } else {
                //set_device_working_ability(MT65XX_PDN_PERI_UART3, DEEP_IDLE_STATE);
                //enable_dpidle_by_bit(MT65XX_PDN_PERI_UART2);
#ifdef MTK_WCN_COMBO_CHIP_SUPPORT
                ret = mtk_uart_pdn_enable(wmt_uart_port_desc, 1);
                if (ret < 0) {
                    CMB_STUB_LOG_WARN("[CMB] %s enter deep idle failed\n", wmt_uart_port_desc);
                }
#endif
            }
            ret = 0;
            break;

        case COMBO_IF_MSDC:
            if(enter == 0){
                //clr_device_working_ability(MT65XX_PDN_PERI_MSDC2, DEEP_IDLE_STATE);
            } else {
                //set_device_working_ability(MT65XX_PDN_PERI_MSDC2, DEEP_IDLE_STATE);
            }
            ret = 0;
            break;
			
		case COMBO_IF_BTIF:
			if(cmb_stub_deep_idle_ctrl_cb)
			{
				ret = (*cmb_stub_deep_idle_ctrl_cb)(enter);
			}else
			{
				CMB_STUB_LOG_WARN("NULL function pointer\n");
			}
			if(ret)
			{
				CMB_STUB_LOG_WARN("%s deep idle fail(%d)\n",enter == 1?"enter":"exit",ret);
			}else
			{
				CMB_STUB_LOG_DBG("%s deep idle ok(%d)\n",enter == 1?"enter":"exit",ret);
			}
			
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}

int
mt_combo_plt_enter_deep_idle (
    COMBO_IF src
    ) {
    //return 0;
    // TODO: [FixMe][GeorgeKuo] handling this depends on common UART or common SDIO
    return _mt_combo_plt_do_deep_idle(src, 1);
}

int
mt_combo_plt_exit_deep_idle (
    COMBO_IF src
    ) {
    //return 0;
    // TODO: [FixMe][GeorgeKuo] handling this depends on common UART or common SDIO
    return _mt_combo_plt_do_deep_idle(src, 0);
}

int mtk_wcn_wmt_chipid_query(void)
{
	return gConnectivityChipId;
}

void mtk_wcn_wmt_set_chipid(int chipid)
{
	CMB_STUB_LOG_INFO("set current consys chipid (0x%x)\n",chipid);
	gConnectivityChipId = chipid;
}

int
mtk_wcn_cmb_stub_do_reset (unsigned int type)
{
	if (cmb_stub_do_reset_cb)
		return (*cmb_stub_do_reset_cb)(type);
	else
		return -1;
}

EXPORT_SYMBOL(mt_combo_plt_exit_deep_idle);
EXPORT_SYMBOL(mt_combo_plt_enter_deep_idle);
EXPORT_SYMBOL(mtk_wcn_cmb_stub_func_ctrl);
EXPORT_SYMBOL(mtk_wcn_cmb_stub_aif_ctrl);
EXPORT_SYMBOL(mtk_wcn_cmb_stub_unreg);
EXPORT_SYMBOL(mtk_wcn_cmb_stub_reg);
EXPORT_SYMBOL(mtk_wcn_wmt_chipid_query);
EXPORT_SYMBOL(mtk_wcn_wmt_set_chipid);
EXPORT_SYMBOL(mtk_wcn_cmb_stub_do_reset);
