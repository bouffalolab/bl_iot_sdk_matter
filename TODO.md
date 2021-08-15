## TODO List
### BUG
- channel switch in sniffer mode is NOT working.
- Cellphone but not computer can connect to AP mode


--------------------------------------------------------
### Enhancement
- Pbuf Zero-Copy
- MJPEG Server framework
- Flash API under XIP mode should be running under 'fw' thread
- 晚上Wi-Fi FW框架，需要应对引入Flash擦写之后带来的TIMER ASSERT的问题(概率事件)
- float 打印的支持 需要支持如下类型的打印：
```
printf("float number is %f\r\n", 1.23);
```
- 将所有PIN MUX的配置放到components/bl60x/bsp目录下对应的board目录中，同时将所有对UART、I2C等数目的配置也放置到此目录。主要目的是隔离板级配置的不同。
- Wi-Fi Manager API优化，需要将所有API的执行上下文切换到Wi-Fi Manager的主线程
- Keil Project Support
- UDP/TCP Performance optimization
- Genrate Keil Project file from mk file
- Use slef-contained libc, but not from compiler
- AP mode with Encrption support



--------------------------------------------------------
### Feature
- MJPEG sink based on UDP
- MJPEG source based on Camera
- MJPEG source based on static pic on PSRAM
- Block Device Support
- Serial Console support
- HTTP Client
- HTTP Server with CGI and FS support
- MQTT Client
- PIN MUX配置层
- 外设数量配置接口
- STA管理包括以下内容：
    - 管理每个STA的上下线、RSSI、MAC地址
    - 管理每个STA的```PowerSaving```状态
    - 统计STA的最近的速率状态，以及数据收发量
- 当前STA的连接AP的结果只有成功或者失败，需要上报更多结果给上层，可能包括如下信息：
    - AP found， or NOT found
    - Password is correct, or NOT
- A-MSDU Support
- A-MPDU Support
- PSM Support
- Audio Layer support
- FATFS Support
- VFS Support
- UART Console support
- Audio codec support
- Audio Streaming based on SDCARD FATFS, Audio format is WAV/MP3
- Audio Streaming based on HTTP, Audio format is WAV/MP3
- IoT Worker任务专用线程。主要目的是将SDK中所有的任务转移到这个线程来完成，这个Worker线程采用事件驱动编程模型。实际效果是降低整个系统的Memory使用和不必要的线程切换开销。中期计划是讲文件系统、SDH、Audio等任务全部以事件的方式集成到这个Worker线程中去。


--------------------------------------------------------
- Amazon AWS IoT support
- AliOS cloud support
