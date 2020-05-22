### Windows

1. Install [msys2](https://www.msys2.org/)
   * install make by `pacman -S make`
3. The GNU Embedded Toolchain [`gcc-arm-none-eabi-4_9-2015q3-20150921-win32.zip`](https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update)
```
https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update
```

4. Edit `make_scripts/toolchain.mk`
```
CONFIG_TOOLPREFIX := /c/tool/gcc/2015q3/bin/arm-none-eabi-
```

5. build bl60x_demo_wlan
```
cd /c/bl60x_sp_sdk/customer_app/bl60x_demo_event
make all
```

6. Get your bin file from
```
customer_app/bl60x_demo_event/build_out
```

### Linux
1. Install linux toolchain [`gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2`](https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update) to `/usr/local/arm/`
```
https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update
```

2. Edit `make_scripts/toolchain.mk`
```
CONFIG_TOOLPREFIX := /usr/local/arm/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-
```

3. build bl60x_demo_event
```
cd customer_app/bl60x_demo_event
make all
```

4. Get your bin file from

```
customer_app/bl60x_demo_event/build_out
```


