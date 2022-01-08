# USB SFC Gamepad
 
This project remake your old SFC(SNES) Controller into a usb gamepad:video_game:  
Let's make a USB SFC Gamepad and play retro games as well as recent games distributed on Steam. 

There is a Japanese support page in my website:crossed_flags:
https://geekyfab.com/sfc_gamepad
You can read more details there. 

# DEMO

 
# Features
In this project, I share circuit, PCB gerber data, BOM and firmware.  
You order your PCB with the gerber data, buy the materials on BOM, burn the firmware and assemble all, then you can get your USB SFC Gamepad.
Though you need to use a driver and a soldering iron, it is fun and worth a try.


# Tested OS
- Windows 10
- Raspberry PI 3
    - Raspberry Pi OS 
    - Recalbox for RASPBERRY PI 3
    - RetroPi 4.7.1 for RASPBERRY PI 2/3
- Raspberry PI 4
    - Raspberry Pi OS
    - Recalbox for RASPBERRY PI 4/400
    - RetroPie 4.7.1 for RASPBERRY PI 4/400

# Usage
## Windows
In a Windows environment, USB SFC Gamepad runs with standard driver.  
You don't need to install additional drivers.  

## Raspberry PI OS
In a Raspberry PI OS environment, you need to install 'joystick' package like this
```bash
sudo apt install joystick
```
It is also good to install 'jstest-gtk' for operation test.
```bash
sudo apt install jstest-gtk
```

## Recalbox and RetroPie
In Recalbox and RetroPie, you don't need to install 'joystick' package.
Maybe Recalbox and RetroPie install it automatically.

# Manufacturing
To make a PCB you need to download the zipped gerber files and upload them to a PCB manufacturer like AllPCB.  
When you order the PCB, it is better to select "gold-plating" option. No other options are needed.
 
## Important
This PCB is designed for Super Famicon(SFC) and I have not check if this PCB fit for Super Nintendo(SNES):relieved:  
If you live where you have played with SNES and make your USB SFC Gamepad, please let me know this PCB fit for SNES or not. 

# Assembly
1. Remove the screws and uncover the SFC controller.
2. remove the original PCB and put in the USB SFC Gamepad PCB.
3. Cover the SFC controller and tighten the screws.
 
# Author
* Takusan
* Geeky Fab.
* geekyfab.com
 
# License
The software files of "USB SFC Gamepad" are under [Apache license 2.0](https://www.apache.org/licenses/LICENSE-2.0).
The hardware files of "USB SFC Gamepad" are under [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/)