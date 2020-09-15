from __future__ import print_function
from __future__ import unicode_literals
import time
import re

from tiny_test_fw import DUT, App, TinyFW
from ttfw_bl import BL602App, BL602DUT


@TinyFW.test_method(app=BL602App.BL602App, dut=BL602DUT.BL602TyMbDUT, test_suite_name='bl602_demo_event_ble_tc')
def bl602_demo_event_tc(env, extra_data):
    # first, flash dut
    # then, test
    dut1 = env.get_dut("port0", "fake app path")
    dut2 = env.get_dut("port1", "fake app path")

    # TODO: Do parallel?
        
    print('Flashing app to dut1')
    dut1.flash_app(env.log_path, env.get_variable('flash'))
    print('Flashing app to dut2')
    dut2.flash_app(env.log_path, env.get_variable('flash'))
    
    print('Starting dut1 app')
    dut1.start_app()
    print('Starting dut2 app')
    dut2.start_app()
    
    try:
        dut1.expect("Booting BL602 Chip...", timeout=2)
        print('[DUT1] booted')
        dut2.expect("Booting BL602 Chip...", timeout=2)
        print('[DUT2] booted')

        dut1.expect('Init CLI with event Driven', timeout=2)
        print('[DUT1] CLI init done')
        dut2.expect('Init CLI with event Driven', timeout=2)
        print('[DUT2] CLI init done')
        time.sleep(1)

        #dut1.write('stack_wifi')
        #time.sleep(0.5)
        #dut1.write('wifi_ap_start')
        #ap_ssid = dut1.expect(re.compile(r"\[WF\]\[SM\] start AP with ssid (.+?);"), timeout=2)[0]
        #print('[DUT1] Started AP with SSID: {}'.format(ap_ssid))
	
	   
        #dut2.write('stack_wifi')
        #time.sleep(0.5)
        #dut2.write(f'wifi_sta_connect {ap_ssid} dummy_password')
        #sta_ip = dut2.expect(re.compile(r"IP GOT IP:(.+?),"), timeout=20)[0]
        #print(f'[DUT2] Connected to AP, got IP {sta_ip}')
        
        
        dut1.write('stack_ble')
        time.sleep(0.5)
        dut1.write('ble_start_adv 0 0 0x80 0x80')
        dut1.expect('Advertising started',timeout=0.5)
        print('[DUT1] Started adv')
        
        dut1.write('ble_read_local_address')
        a = dut1.expect(re.compile(r"Local public addr : ((\w{2}.){6})"), timeout=2)[0]
        addr = ''.join(a.split(":"))
        print(f'[DUT1] Get ble address: {addr}')
        
        dut2.write('stack_ble')
        time.sleep(0.5)
        dut2.write(f'ble_connect 0 {addr}')
        dut2.expect('ble_tp_connected',timeout=0.5)
        print('[DUT2] Connected')
        
        
        
        

    except DUT.ExpectTimeout:
        print('ENV_TEST_FAILURE: BL602 example test failed')
        raise
    finally:
        # we should ALWAYS stop chip, free resources after test is done
        dut1.halt()
        dut2.halt()


if __name__ == '__main__':
    bl602_demo_event_tc()
