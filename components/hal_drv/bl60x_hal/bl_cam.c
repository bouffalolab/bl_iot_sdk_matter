#include <string.h>
#include <stdio.h>
#include <bl60x_glb.h>
#include <bl60x_gpio.h>
#include <bl60x_cam.h>
#include <bl60x_mjpeg.h>
#include <cam_reg.h>
#include <mjpeg_reg.h>
#include <FreeRTOS.h>
#include <event_groups.h>

#include "bl_cam.h"

#include "utils_log.h"

#define DUMP_CAM  0

static bl60x_cam_t camera_dvp;
static bl60x_cam_t *context_cam;
static bl60x_mjpeg_t mjpeg_engine;
static EventGroupHandle_t camera_event;
static const rt_camera_desc *m_desc;

//static bl60x_mjpeg_t *context_mjpeg;
uint8_t  *buffer_cam;
uint32_t  buffer_cam_size;
uint32_t frame_pos;
#define MJPEG_BUFFER_SIZE 0x100000
#define MJPEG_EVENT_FRAME_INT (1 << 0)
#define MJPEG_EVENT_FRAME_ERROR (1 << 1)

#define CAM_REG_INT_STATUS_BITS_ERR     (0x7 << 11)
#define CAMERA_EVENT_FRAME_INT  (1 << 0)
#define CAMERA_EVENT_FRAME_ERROR (1 << 1)

#define mjpeg_printf(...) do {} while (0)

uint8_t jpeg_quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_50;

int bl_cam_config_update(uint8_t quality)
{
    jpeg_quality = quality;
    printf("[MJPEG] Using quality %u\r\n",
            jpeg_quality
    );

    return 0;
}

int bl_cam_config_get(uint8_t *quality, uint16_t *width, uint16_t *height)
{
    if (quality){
        *quality = jpeg_quality;
    }
    if (width){
        *width = m_desc->width;
    }
    if (height){
        *height = m_desc->height;
    }

    return 0;
}

static int cam_init_gpio(void)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_0;
    cfg.gpioFun = GPIO0_FUN_PIX_CLK;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_1;
    cfg.gpioFun = GPIO1_FUN_FRAME_VLD;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_2;
    cfg.gpioFun = GPIO2_FUN_LINE_VLD;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_3;
    cfg.gpioFun = GPIO3_FUN_PIX_DAT0;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_4;
    cfg.gpioFun = GPIO4_FUN_PIX_DAT1;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_5;
    cfg.gpioFun = GPIO5_FUN_PIX_DAT2;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_6;
    cfg.gpioFun = GPIO6_FUN_PIX_DAT3;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_25;
    cfg.gpioFun = GPIO25_FUN_PIX_DAT4;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_26;
    cfg.gpioFun = GPIO26_FUN_PIX_DAT5;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_27;
    cfg.gpioFun = GPIO27_FUN_PIX_DAT6;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_28;
    cfg.gpioFun = GPIO28_FUN_PIX_DAT7;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);
    return 0;
}

uint8_t *frame;
uint32_t size, frames, status, isBlock, frames_count;
static void int_rx(struct bl60x_cam *cam, uint8_t *frame_ptr, uint32_t frame_size, uint32_t frames_total, uint32_t error)
{
#if 0
    if (1 == isBlock) {
        return;
    }
    //FIXME interrupt handler no LOOP here
    while (0 != frames_total) {
        frame = frame_ptr;
        size = frame_size;
        frames = frames_total;
        status = ((error >> 10) & 0x0F);
        printf("frame_ptr %p, len %u, total %u, status %u\r\n",
            frame,
            (unsigned int)size,
            (unsigned int)frames,
            (unsigned int)status
        );
        bl60x_cam_frame_complete(cam, &frame_ptr, &frame_size, &frames_total, &error);
    }
    //FIXME ?
    cam->isbusy = 0;
#else
    BaseType_t xHigherPriorityTaskWoken, xResult;

    xHigherPriorityTaskWoken = pdFALSE;
    xResult = xEventGroupSetBitsFromISR(
            camera_event, /* The event group being updated. */
            (error & CAM_REG_INT_STATUS_BITS_ERR) ?
                (CAMERA_EVENT_FRAME_INT | CAMERA_EVENT_FRAME_ERROR) :
                (CAMERA_EVENT_FRAME_INT), /* The bits being set. */
            &xHigherPriorityTaskWoken
    );
    /* Was the message posted successfully? */
    if( xResult != pdFAIL ) {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE, then a context switch should be requested. The macro used is port-specific and will
        *
        *          be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR(). See the documentation page for the port being used. */

        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
#endif
}

static int dvp_init(int useMjpeg, int frm_vld_high)
{
    memset(&camera_dvp, 0, sizeof(camera_dvp));

    if (useMjpeg) {

        camera_dvp.sw_mode = BL60x_CAM_SW_MODE_AUTO;
        camera_dvp.line = BL60x_CAM_LINE_ACTIVE_POLARITY_HIGH;
        camera_dvp.frame = frm_vld_high;
        camera_dvp.sensor_mode = BL60x_CAM_SENSOR_MODE_V_AND_H;
        camera_dvp.burst = BL60x_CAM_AHB_BURST_INCR16;
        camera_dvp.buffer = buffer_cam;
        camera_dvp.size = buffer_cam_size;
        camera_dvp.isbusy = 0;
        camera_dvp.int_rx = 0;
    } else {
        camera_dvp.sw_mode = BL60x_CAM_SW_MODE_MANUAL;
        camera_dvp.line = BL60x_CAM_LINE_ACTIVE_POLARITY_HIGH;
        camera_dvp.frame = frm_vld_high;
        camera_dvp.sensor_mode = BL60x_CAM_SENSOR_MODE_V_AND_H;
        camera_dvp.burst = BL60x_CAM_AHB_BURST_INCR16;
        camera_dvp.buffer = (uint8_t*)0x21100000;
        camera_dvp.size = 1280 * 720 * 2 * 1;
        camera_dvp.isbusy = 0;
        camera_dvp.int_rx = int_rx;
    }
    context_cam = &camera_dvp;
    bl60x_cam_init(&camera_dvp);
    bl60x_cam_enable(&camera_dvp);

    return 0;
}

static void handler_cam_frame_complete(void)
{
    cam_reg_t *camreg = (cam_reg_t*)CAM_BASE;
    uint8_t *frame;
    uint32_t size, frames, status;

    if (context_cam->isbusy) {
        /*upper layer is busy with handle jpeg*/
        return;
    }
    status = camreg->cam_status_and_error.WORD;
    if (context_cam && context_cam->int_rx) {
        frames = camreg->cam_status_and_error.BF.frame_valid_cnt;
        frame = (uint8_t*)(camreg->frame_start_addr0.WORD);
        size = camreg->frame_byte_cnt0.WORD;
        context_cam->isbusy = 1;
        context_cam->int_rx(context_cam, frame, size, frames, status);
    }
}
void CAM_IRQHandler(void)
{
    uint32_t status;
    cam_reg_t *camreg = (cam_reg_t*)CAM_BASE;

    status = camreg->cam_status_and_error.WORD;
    if (status & (1 << 13)) {
        log_error("[DVP] status is %08lx *******************\r\n", status);
    }
    camreg->cam_status_and_error.BF.reg_int_clr = 1;

    handler_cam_frame_complete();
}

static void int_rx_mjpeg(struct bl60x_mjpeg *mjpeg, uint8_t *frame_ptr, uint32_t frame_size, uint32_t frames_total, uint32_t error)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;

    xHigherPriorityTaskWoken = pdFALSE;
    xResult = xEventGroupSetBitsFromISR(
        camera_event, /* The event group being updated. */
        (error & CAM_REG_INT_STATUS_BITS_ERR) ?
        (CAMERA_EVENT_FRAME_INT | CAMERA_EVENT_FRAME_ERROR) :
        (CAMERA_EVENT_FRAME_INT), /* The bits being set. */
        &xHigherPriorityTaskWoken
    );
    if( xResult != pdFAIL ) {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
    mjpeg_engine.isbusy = 0;
}

int bl_cam_frame_wait(void)
{
    int ret;
    EventBits_t xResult;

    if ((ret = bl60x_mjpeg_frame_count())) {
        return ret;
    }

    xResult = xEventGroupWaitBits(
            camera_event,
            CAMERA_EVENT_FRAME_INT,
            pdTRUE,
            pdFAIL,
            10
    );
    if (xResult & CAMERA_EVENT_FRAME_INT){
        return 0;
    }

    return -1;
}

int bl_cam_yuv_frame_wait(void)
{
    int ret;
    EventBits_t xResult;

    if ((ret = bl60x_cam_frame_count())) {
        return ret;
    }

    xResult = xEventGroupWaitBits(
            camera_event,
            CAMERA_EVENT_FRAME_INT,
            pdTRUE,
            pdFAIL,
            10
    );
    if (xResult & CAMERA_EVENT_FRAME_INT){
        return 0;
    }

    return -1;
}

void bl_cam_frame_edge_drop(void)
{
    uint32_t size, frames, status;
    uint8_t *ptr;

    bl60x_mjpeg_frame_try(&mjpeg_engine, &ptr, &size, &frames, &status);
    size = ((size + 7) >> 3);
    if ((ptr + size) > (mjpeg_engine.buffer_mjpeg + mjpeg_engine.size_mjpeg)) {
        mjpeg_printf("[MJPEG] Edge detected for edge pop\r\n");
        if (frame_pos > 0) {
            frame_pos--;//decrease after pop
        }
        bl60x_mjpeg_frame_pop(&mjpeg_engine);
    }
}

int bl_cam_frame_pop(void)
{
    printf("[MJPEG] [%d] [POP] %lu\r\n", bl60x_mjpeg_frame_count(), frame_pos);
    if (frame_pos > 0) {
        frame_pos--;//decrease after pop
    }
    bl60x_mjpeg_frame_pop(&mjpeg_engine);
    /*try edge drop every time*/
    bl_cam_frame_edge_drop();

    return 0;
}

int bl_cam_yuv_frame_pop(void)
{
    bl60x_cam_frame_pop(&camera_dvp);

    return 0;
}

int bl_cam_frame_pop_old(void)
{
    while (bl60x_mjpeg_frame_count() > 1) {
		bl_cam_frame_pop();
    }
    return 0;
}

int bl_cam_frame_get(uint32_t *frames, uint8_t **ptr1, uint32_t *len1, uint8_t **ptr2, uint32_t *len2)
{
    uint32_t status = 0;
    uint32_t size1;

    bl60x_mjpeg_frame_try(&mjpeg_engine, ptr1, &size1, frames, &status);
    size1 = ((size1 + 7) >> 3);
    if ((*ptr1 + size1) > (mjpeg_engine.buffer_mjpeg + mjpeg_engine.size_mjpeg)) {
        mjpeg_printf("[MJPEG] Edge detected\r\n");
        return -1;
    }
    if (*frames) {
        *len1 = size1;
        *ptr2 = NULL;
        *len2 = 0;
        return 0;
    }

    return -1;
}

int bl_cam_yuv_frame_get(uint32_t *frames, uint8_t **ptr1, uint32_t *len1, uint8_t **ptr2, uint32_t *len2)
{
    uint32_t status = 0;

    bl60x_cam_frame_try(&camera_dvp, ptr1, len1, frames, &status);
    if (*frames) {
        *ptr2 = NULL;
        *len2 = 0;
        return 0;
    }

    return -1;
}

int bl_cam_frame_fifo_get(uint32_t *frames, uint8_t **ptr1, uint32_t *len1, uint8_t **ptr2, uint32_t *len2)
{
    uint32_t status = 0;
    uint32_t size1;
    cam_reg_t *camreg = (cam_reg_t*)CAM_BASE;

    /* FIFO OverWrite interrupt status */
    if (camreg->cam_status_and_error.WORD & (1 << 13)) {
        camreg->cam_status_and_error.BF.reg_int_clr = 1;

        bl_cam_restart(1);

        while (bl60x_mjpeg_frame_count()) {
            bl_cam_frame_pop();
        }

        log_error("FIFO OverWrite interrupt status");
        return -1;
    }

retry:
    if (bl60x_mjpeg_frame_pos(&mjpeg_engine, frame_pos, ptr1, &size1, frames, &status)) {
        /*no enough frame*/
        mjpeg_printf("no enough frame\r\n");
        return -1;
    }
    frame_pos++;//increase after we got frame
    size1 = ((size1 + 7) >> 3);
    if ((*ptr1 + size1) > (mjpeg_engine.buffer_mjpeg + mjpeg_engine.size_mjpeg)) {
        mjpeg_printf("[MJPEG] Edge @%d\r\n", (int)(frame_pos - 1));
        if (1 == frame_pos) {
            mjpeg_printf("[MJPEG] Auto pop at head\r\n");
            bl_cam_frame_pop();
        }
        /*try again*/
        goto retry;
    }

    *len1 = size1;
    *ptr2 = NULL;
    *len2 = 0;

    return 0;
}

static int mjpeg_init()
{
    memset(&mjpeg_engine, 0, sizeof(mjpeg_engine));

    mjpeg_engine.burst = BL60x_MJPEG_TYPE_CONTROL_REG_AHB_BURST_INCR16;
    switch (jpeg_quality) {
        case 5:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_5;
        }
        break;
        case 10:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_10;
        }
        break;
        case 25:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_25;
        }
        break;
        case 50:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_50;
        }
        break;
        case 75:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_75;
        }
        break;
        case 100:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_100;
        }
        break;
        default:
        {
            mjpeg_engine.quality = BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_50;
        }
    }
    mjpeg_engine.yuv = BL60x_MJPEG_TYPE_CONTROL_REG_YUV_MODE_YUV422;
#if 1
    mjpeg_engine.buffer_mjpeg = (uint8_t*)0x21300000;
    mjpeg_engine.size_mjpeg = 0x100000;
#else
#if 0
    mjpeg_engine.buffer_mjpeg = (uint8_t*)(0x20010000 - 0x5000);;
    mjpeg_engine.size_mjpeg = 0x9000;
#else
    mjpeg_engine.buffer_mjpeg = buffer_cam;
    mjpeg_engine.size_mjpeg = buffer_cam_size;
#endif
#endif
    mjpeg_engine.buffer_cam = buffer_cam;
    mjpeg_engine.size_cam = buffer_cam_size;
    mjpeg_engine.resolution_x = m_desc->width;
    mjpeg_engine.resolution_y = m_desc->height;
    mjpeg_engine.isbusy = 0;
    mjpeg_engine.bitorder_enable = 1;
    mjpeg_engine.int_rx = int_rx_mjpeg;

    bl60x_mjpeg_init(&mjpeg_engine);
    bl60x_mjpeg_enable(&mjpeg_engine);

    return 0;
}

static int video_init(int useMjpeg)
{
#if 1
    static char video_init_flag = 0;

    if (0 == video_init_flag) {
        camera_event = xEventGroupCreate();
        video_init_flag = 1;
    }
#else
    if (NULL != camera_event) {
        vEventGroupDelete(camera_event);
    }
    camera_event = xEventGroupCreate();
#endif

    if (useMjpeg) {
#if 0
        buffer_cam = rt_malloc(CAMERA_BUFFER_SIZE_WHEN_MJPEG);
        buffer_cam_size = CAMERA_BUFFER_SIZE_WHEN_MJPEG;
#else
        buffer_cam = (uint8_t*)(0x20010000 - 0xA000);
        buffer_cam_size = m_desc->width * 2 * 8 * 2;
#endif
        bl60x_cam_disable(&camera_dvp);
        bl60x_mjpeg_disable(&mjpeg_engine);
#if !DUMP_CAM
        mjpeg_init();
#endif
        dvp_init(!DUMP_CAM, m_desc->frm_vld_high);
    } else {
        bl60x_cam_disable(&camera_dvp);
        bl60x_mjpeg_disable(&mjpeg_engine);
        dvp_init(0, m_desc->frm_vld_high);
    }

    return 0;
}

static int _quality_convert_to_hardware(uint32_t quality)
{
    switch (quality) {
        case 100:
        {
            return BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_100;
        }
        break;
        case 75:
        {
            return BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_75;
        }
        break;
        case 50:
        {
            return BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_50;
        }
        break;
        case 25:
        {
            return BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_25;
        }
        break;
        case 10:
        {
            return BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_10;
        }
        break;
        case 5:
        {
            return BL60x_MJPEG_TYPE_CONTROL_REG_Q_MODE_5;
        }
        break;
        default:
        {
            return -1;
        }
    }
    return -1;
}

static void set_camera_desc(const rt_camera_desc *desc)
{
    m_desc = desc;
}

int bl_cam_mjpeg_encoder(uint32_t yuv_addr, uint32_t jpeg_addr, uint32_t *jpeg_size,  uint32_t width, uint32_t height, uint32_t quality)
{
    mjpeg_reg_t *mjpegreg = (mjpeg_reg_t*)MJPEG_BASE;
    uint32_t val;
    int q_mode;

    /*RESET JPEG module*/
    *(volatile uint32_t*)0x40000010 = 0x40;
    vTaskDelay(1);
    *(volatile uint32_t*)0x40000010 = 0x00;

    //disable mjpeg
    mjpegreg->mjpeg_control.BF.reg_mjpeg_enable = 0;

    //basic stuff
    mjpegreg->mjpeg_control.BF.reg_yuv_mode =  BL60x_MJPEG_TYPE_CONTROL_REG_YUV_MODE_YUV422;
    q_mode = _quality_convert_to_hardware(quality);
    if (q_mode < 0) {
        return -1;
    }
    mjpegreg->mjpeg_control.BF.reg_q_mode =  q_mode;
    mjpegreg->mjpeg_control.BF.reg_ahb_burst_w =  BL60x_MJPEG_TYPE_CONTROL_REG_AHB_BURST_INCR16;
    mjpegreg->mjpeg_control.BF.reg_total_block_x =  (width >> 3);
    mjpegreg->mjpeg_control.BF.reg_total_block_y =  (height >> 3);
    mjpegreg->mjpeg_control.BF.reg_mjpeg_bit_order =  1;

    /*align buffer to 16 bytes boundary, should be kept the same as CAM module*/
    mjpegreg->mjpeg_read_addr_start.WORD = yuv_addr;
    mjpegreg->cam_store_memory.BF.reg_cam_store_block_y = (height >> 3);

    /*align buffer to 16 bytes boundary*/
    mjpegreg->mjpeg_write_add_start.WORD = jpeg_addr;
    /*align buffer size in unit of 64 bytes */
    mjpegreg->mjpeg_store_memory.BF.reg_store_burst_cnt_w = ((*jpeg_size) >> 6);
    mjpegreg->mjpeg_control2.BF.mjpeg_int_clear = 1;
    /*config INT*/
    mjpegreg->mjpeg_control2.BF.reg_int_normal_en = 0;
    mjpegreg->mjpeg_control2.BF.reg_int_cam_en = 0;
    mjpegreg->mjpeg_control2.BF.reg_int_mem_en = 0;
    mjpegreg->mjpeg_control2.BF.reg_int_frame_en = 0;

    /*MAP UYVY to YUYV
     * TODO
     * */
#if 1
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte0 = 1; //Yn
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte1 = 0; //Un
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte2 = 3; //Yn+1
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte3 = 2; //Vn
#else
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte0 = 0; //Yn
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte1 = 1; //Un
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte2 = 2; //Yn+1
    mjpegreg->mjpeg_rdata_order.BF.reg_rda_order_byte3 = 3; //Vn
#endif

    /*Enable SW mode*/
    val = (1 << 0) | (1 << 8) | (0 << 9);
    mjpegreg->mjpeg_sw_mode.WORD = val;
    /*Trigger Encoder mode*/
    val = (1 << 0) | (0 << 8) | (1 << 9);
    mjpegreg->mjpeg_sw_mode.WORD = val;

    while (1) {
        printf("[MJPEG] Waiting Encoder done...\r\n");
        if (mjpegreg->mjpeg_start_addr0.WORD) {
            break;
        }
        vTaskDelay(4);
    }
    *jpeg_size = mjpegreg->mjpeg_bit_cnt0.WORD;

    return 0;
}

int bl_cam_init(int enable_mjpeg, const rt_camera_desc *desc)
{
    cam_init_gpio();
    set_camera_desc(desc);
    video_init(enable_mjpeg);
    return 0;
}

int bl_cam_restart(int enable_mjpeg)
{
    cam_init_gpio();
    video_init(enable_mjpeg);
    return 0;
}

int bl_cam_enable_24MRef(void)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_24;
    cfg.gpioFun = GPIO24_FUN_CAM_REF_CLK;
    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    GLB_Set_Cam_Ref_CLK(1, 0);
    return 0;
}
