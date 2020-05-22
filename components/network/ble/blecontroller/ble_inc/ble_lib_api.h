#ifndef BLE_LIB_API_H_
#define BLE_LIB_API_H_

void ble_controller_init(uint8_t task_priority);

// return sleep duration, in unit of 1/32768s
// if 0, means not allow sleep
// if -1, means allow sleep, but there is no end of sleep interrupt (ble core deep sleep is not enabled)
int32_t ble_controller_sleep(void);

void ble_controller_wakeup(void);

#endif
