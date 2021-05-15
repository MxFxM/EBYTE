# EBYTE

This is a fork from: https://github.com/KrisKasprzak/EBYTE
 
<b><h1><center>EBYTE Transceivers</center></h1></b>

This library is intended to be used with UART type EBYTE transceivers.
This library lets users program the operating parameters and both send and receive data.
All constants were extracted from several data sheets and listed in binary as that's how the data sheet represented each setting.

Usage of this library consumes around 970 bytes

## Default Parameters

----------------------------------------<\r><\n>

Model no.: 45<\r><\n>

Version  : D<\r><\n>

Features : 14<\r><\n>

<\r><\n>

Mode (HEX/DEC/BIN): C0/192/11000000<\r><\n>

AddH (HEX/DEC/BIN): 0/0/0<\r><\n>

AddL (HEX/DEC/BIN): 0/0/0<\r><\n>

Sped (HEX/DEC/BIN): 1A/26/11010<\r><\n>

Chan (HEX/DEC/BIN): 6/6/110<\r><\n>

Optn (HEX/DEC/BIN): 44/68/1000100<\r><\n>

Addr (HEX/DEC/BIN): 0/0/0<\r><\n>

<\r><\n>

SpeedParityBit (HEX/DEC/BIN)    : 0/0/0<\r><\n>

SpeedUARTDataRate (HEX/DEC/BIN) : 3/3/11<\r><\n>

SpeedAirDataRate (HEX/DEC/BIN)  : 2/2/10<\r><\n>

OptionTrans (HEX/DEC/BIN)       : 0/0/0<\r><\n>

OptionPullup (HEX/DEC/BIN)      : 1/1/1<\r><\n>

OptionWakeup (HEX/DEC/BIN)      : 0/0/0<\r><\n>

OptionFEC (HEX/DEC/BIN)         : 1/1/1<\r><\n>

OptionPower (HEX/DEC/BIN)       : 0/0/0<\r><\n>

----------------------------------------<\r><\n>


<b><h3>EBYTE Model numbers, not all tested</b></h3>

E22-900T22S, E22-230T22S, E22-400T22S, E22-230T30S, E22-400T30S, E22-900T30S, E30-433T20S3, E30-170T20D, E30-170T27D, E30-780T20S, E30-868T20S, E30-868T20D, E30-915T20D, E30-490T20D, E30-433T20S, E30-433T20D, E30-915T20S, E30-490T20S, E31-433T30S, E31-433T17S3, E31-230T33D, E31-230T17D, E31-230T27D, E31-433T17S, E31-433T17D, E31-433T27D, E31-433T30D, E31-433T33D, E32-433T20DC, E32-433T20S, E32-433T20S2T, E32-433T27D, E32-433T30D, E32-433T30S, E32-868T20D, E32-868T20S, E32-868T30D, E32-868T30S,
E32-915T20D, E32-915T20S, E32-915T30D, E32-915T30S, E32-170T30D, E32-400T20S
  
<b><h3> Module connection </b></h3>

| Module | MCU | Description |
|--|--|--|
| MO | any digital pin | Pin to control working/program modes
| M1 | any digital pin | Pin to control working/program modes
| Rx | Tx of Serial Interface | Serial communication via UART
| Tx | Rx of Serial Interface | Serial communication via UART
| AUX | any digital pin | Pin to indicate when an operation is complete (low is busy, high is done)
| Vcc | +3v3 | Use 3v3 to be save. Current draw can be high during transmission
| Gnd | Ground | Ground must be common to module and MCU	

<b><h3>NOTES</b></h3>

1. Some MCUs use the same pins for USB connection and serial interface
2. The signal lines for these modules are 3v3
4. The Aux pin is not required. Pass -1 as pin number. Transmissions to the module may take longer

<b><h3>General code usage</b></h3>

1. Create a serial object
2. Create EBYTE object that uses the serial object
3. Begin the serial object
4. Init() the EBYTE object
5. Set parameters (optional but required if sender and receiver are different)
6. Send or listen to sent data (single byte) OR create and send a data structure

<b><h3>Tips on usage, for best range...</b></h3> 

- Keep antennas 2 meters off ground
- Line of sight is ideal
- Slow air data rates can improve range, but obviously increase transmission time
- Consider high gain antennas, but check local laws for transmission power
- Power the units with 5.0 volts, keep 3v3 on the signal lines
 
<b><h3>Data transmission packets</b></h3>
This library has a method for sending single bytes but if more data is to be sent, creating a data structure and sending the data structure using the librarys SendStruct(&struct, sizeof(struct)) method is recommended.
Again slow data rates take longer, you will need to experiment with ideal air data rate range based on data size.
If you need to send data using a struct between different MCUs, changes of how each processor packs integers and floating point numbers will probably be different.
If you get corrupted data on the recieving end, there are ways to force the compiler to not optimize struct packing.

<b><h3>Debugging Tips</b></h3>

If you are using their 1W units, power the unit separately from the MCU's onboard power supply.
The current draw may exceed the onboard rating resulting in destroying the MCU, the voltage regulator or even the USB port on the PC.
If transmitter and receiver are different MCUs (Arduino <-> Teensy), data structures cannot have a mix of data types, due to how an 8-bit processor and 32-bit processor handle ints, floats, etc.
If floats and ints are needed to be sent considering multiplying a float to 100 (and recasting to an int), then divide that value by 100 on the receiving end (recasting to a float).
Other ways of handling data types can be using uint_8t or similar and specify datatypes exactly.
