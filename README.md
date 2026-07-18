# Gesture-Capture-Watch
A wristband-mounted gesture recognition system built around an STM32F411, IMU, and OLED display, with USB-C battery charging and ESD-protected I/O. Includes full PCB design, manufacturing files, and firmware for real-time motion capture and on-device gesture classification via TFLite Micro. 

# Hardware Overview
- **MCU:** [STM32F411CEU6](https://www.st.com/en/microcontrollers-microprocessors/stm32f411.html) — high Flash and SRAM to run TFLite model
- **IMU:** [LSM6DSOX](https://www.st.com/en/mems-and-sensors/lsm6dsox.html) — industry standard, small form factor, low power consumption, includes a machine learning core for pre-processing
- **Power System:**
  - **Interface:** [USB-C](https://www.digikey.com/en/products/detail/gct/USB4105-GF-A-120/14559037), flexible power delivery, industry standard
  - **ESD Protection:** [USBLC6-2SC6](https://www.digikey.com/en/products/detail/stmicroelectronics/USBLC6-2SC6/1040559), industry standard
  - **Li-ion Battery Charger:** [BQ24092DGQR](https://www.ti.com/product/BQ24092/part-details/BQ24092DGQR) — short circuit protection, variable charging
  - **Linear Regulator:** [TPS7A0533PDBZ](https://www.ti.com/product/TPS7A05/part-details/TPS7A0533PDBZT) — low drop-out
- **Display:** 0.96" OLED display with SSD1306 chipset

# Software Overview (WIP)
- **Programming Environment:** STM32 programmed and configured via STLink V2. STM32CubeIDE was used to write code in C, and STM32CubeMX was used to set up peripherals and the external high-speed clock.
- **Libraries:** [u8g2](https://github.com/olikraus/u8g2), [lsm6dsox](https://github.com/STMicroelectronics/lsm6dsox-pid)
    
