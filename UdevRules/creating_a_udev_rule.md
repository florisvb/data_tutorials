# Creating a udev rule for a usb device

1. With device unplugged, type `dmesg` in the terminal and note the last device that is shown
2. Plug in the device, and type `dmesg` again. You should now see the information for your device, for example:

	[3447934.369359] usb 3-2: new full-speed USB device number 4 using xhci_hcd
	[3447934.805911] usb 3-2: New USB device found, idVendor=2341, idProduct=0042, bcdDevice= 0.01
	[3447934.805916] usb 3-2: New USB device strings: Mfr=1, Product=2, SerialNumber=220
	[3447934.805919] usb 3-2: Manufacturer: Arduino (www.arduino.cc)
	[3447934.805922] usb 3-2: SerialNumber: 5593034363635121D191
	[3447934.816770] cdc_acm 3-2:1.0: ttyACM1: USB ACM device

Note: you can get much more info if you type `sudo lsusb -v`, but we don't need all that.

3. Take note of the following key numbers: idVendor, idProduct, and SerialNumber.

4. Create a new udev rule: `sudo gedit /etc/udev/rules.d/99-YOUR-RULE.rules`. The 99 signifies that this will be among the last rules to be applied, which is safest. But, if you have a rule that applies to all arduinos, say, and you wish to also have a rule for a specific one identified by serial number, then put the serial number specific one first, e.g. use 90 or some other small number.

5. The contents of the rule should be something like the following. Only use the serial tag if you want a specific rule for a specific device. Some devices use subsystem tty, and others usb. 

	SUBSYSTEM=="tty", ATTRS{idVendor}=="2341", ATTRS{idProduct}=="0042", ATTRS{serial}=="5593034363635121D191", GROUP="plugdev", SYMLINK+="ledctrl", MODE:="0666"

6. After saving the rule, reload your rules: `sudo udevadm control --reload-rules`

7. Unplug your device, and replug, check `/dev` to see if it has appeared. If you used the symlink tag indicated in step 5, it should appear under that symlink, e.g. `/dev/ledctrl`. It will also appear under tty, for example, an arduino will also show up as (for example) `ttyACM1`. 

8. Note: there will be no errors if the rule doesn't work, which can be frustrating. Check the exact syntax. Also check the subsystem. Without the rule, take note of the tty device, e.g. `/dev/ttyACM1`. It will only exist if the device is plugged in. Then type: `udevadm info --query=all --name=/dev/ttyACM1`. This will show the subsystem. 