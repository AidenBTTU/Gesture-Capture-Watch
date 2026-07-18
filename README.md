# Gesture-Capture-Watch
A wristband-mounted gesture recognition system built around an STM32F411, IMU, and OLED display, with USB-C battery charging and ESD-protected I/O. Includes full PCB design, manufacturing files, and firmware for real-time motion capture and on-device gesture classification via TFLite Micro. 

# Hardware Overview
-MCU: STM32F411CEU6, high Flash and SRAM to run TFlite model
-IMU: LSM6DSOX, industry standard, small form factor, low power consumption, and a machine learning core for pre-processing
-Power System:
    -Interface: USB-C, chosen for customizable power consumption, industry standard
    -ESD Protection: UMW USBLC6-2SC6, industry standard
    -Lion Battery Charger: BQ24092DGQR, short circuit protection, variable charging
    -Linear Regulator: TPS7A0533PDBZ, low drop-out
-Display: 0.96" OLED Display with SSD1306 chipset
    
