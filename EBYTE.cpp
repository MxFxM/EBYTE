/* 
  Original code by: Kris Kasprzak kris.kasprzak@yahoo.com
  Modified by: Max-Felix Mueller mxfxmmueller@gmail.com

  This library is intended to be used with EBYTE transcievers, small wireless units for MCU's such as
  Teensy and Arduino. This library let's users program the operating parameters and both send and recieve data.
  This company makes several modules with different capabilities, but most #defines here should be compatible with them
  All constants were extracted from several data sheets and listed in binary as that's how the data sheet represented each setting
  Hopefully, any changes or additions to constants can be a matter of copying the data sheet constants directly into these #defines
  Usage of this library consumes around 970 bytes
*/

#include <EBYTE.h>
#include <Stream.h>
#include <Arduino.h>

/*
Create the tranceiver object
*/
EBYTE::EBYTE(Stream *s, uint8_t PIN_M0, uint8_t PIN_M1, uint8_t PIN_AUX) {

	// serial port for communication with the module
	_s = s;

	// other digital pins connecting the module
	_M0 = PIN_M0;
	_M1 = PIN_M1;
	_AUX = PIN_AUX;		
}

/*
Initialize the unit
Read the module parameters and store default parameters for future module settings
*/
bool EBYTE::init() {
	// set pins to output
	pinMode(_AUX, INPUT);
	pinMode(_M0, OUTPUT);
	pinMode(_M1, OUTPUT);

	// normal operating mode
	SetMode(MODE_NORMAL);
	
	// get module data
	if (!ReadModelData()) {
		return false;
	}

	// get parameters to put unit defaults into the class variables
	if (!ReadParameters()) {
		return false;
	}

	return true;
}

/*
Availability of the serial interface
Returns true if at least 1 byte is available
*/
bool EBYTE::available() {
	return _s->available();
}

/*
Wait for all data in the serial buffer to be transmitted
*/
void EBYTE::flush() {
	_s->flush();
}

/*
Write a single byte to the module via the serial interface
For more than one byte, send it as a data structure
*/
void EBYTE::SendByte( uint8_t TheByte) {
	_s->write(TheByte);
}

/*
Read a single byte from the serial interface
For more than one byte, receive it as a data structure
*/
uint8_t EBYTE::GetByte() {
	return _s->read();
}

/*
Send multiple bytes as a data sturcture
TIP: structure definition in a .h file and include in both sender and reciever
NOTE: different MCU's handle ints and floats differently (Arduino, Teensy, ...)
*/
bool EBYTE::SendStruct(const void *dataStruct, uint16_t size_) {
		// get the return of the write operation
		_buf = _s->write((uint8_t *) dataStruct, size_);
		
		// wait for the transmission to finish
		CompleteTask(1000);
		
		// write will return the number of bytes written
		// if the number does not match, not all bytes have been written
		return (_buf == size_);
}

/*
Read multiple bytes in a data structure
TIP: structure definition in a .h file and include in both sender and reciever
NOTE: different MCU's handle ints and floats differently (Arduino, Teensy, ...)
*/
bool EBYTE::GetStruct(const void *dataStruct, uint16_t size_) {
	// read the expected number of bytes
	_buf = _s->readBytes((uint8_t *) dataStruct, size_);

	// wait for the transmission to finish
	CompleteTask(1000);

	// readBytes will return the number of bytes read
	// if the number does not match, not all bytes have been read
	return (_buf == size_);
}

/*
Wait until module is done tranmitting
Timeout in milliseconds is provided to avoid an infinite loop
TIP: if no AUX pin is used, pull the module AUX pin high with 4k7
NOTE: worst case timeout is nearly doubled due to timer overflow handling
NOTE: no AUX pin is necessary, but then a fixed delay of 1 second will be used
*/
void EBYTE::CompleteTask(unsigned long timeout) {
	// get starting time of task
	unsigned long t = millis();

	// if millis would overflow during timeout
	if (((unsigned long) (t + timeout)) == 0){
		// wait until after the overflow and then start timeout timer
		t = 0;
	}

	// if AUX pin was set up
	if (_AUX != -1) {
		// wait for a high signal on the AUX pin
		while (digitalRead(_AUX) == LOW) {
			// and check the timeout
			if ((millis() - t) > timeout){
				break;
			}
		}
	} else {
		// wait at least 1 second if no AUX pin is used
		if (timeout < 1000) {
			// you may need to adjust this value if transmissions fail
			delay(1000);
		} else {
			delay(timeout);
		}
	}

	// per data sheet control after aux goes high is 2ms so delay for at least that long)
	delay(AUX_PIN_RECOVER);
}

/*
Method to set the mode
Available modes are: MODE_NORMAL
					 MODE_WAKEUP
					 MODE_POWERDOWN
					 MODE_PROGRAM
*/
void EBYTE::SetMode(uint8_t mode) {
	// give the module some time
	delay(PIN_RECOVER);
	
	if (mode == MODE_NORMAL) {
		digitalWrite(_M0, LOW);
		digitalWrite(_M1, LOW);
	}
	else if (mode == MODE_WAKEUP) {
		digitalWrite(_M0, HIGH);
		digitalWrite(_M1, LOW);
	}
	else if (mode == MODE_POWERDOWN) {
		digitalWrite(_M0, LOW);
		digitalWrite(_M1, HIGH);
	}
	else if (mode == MODE_PROGRAM) {
		digitalWrite(_M0, HIGH);
		digitalWrite(_M1, HIGH);
	}

	// wait for the module to accept new mode
	delay(PIN_RECOVER);
	
	// wait until aux pin goes back low
	// CompleteTask(4000);
}

// i have no clue what this is supposed to do
// seems everytime I try using it, it sets all parameters to 0
// i've asked EBYTE what's supposed to happen--got an unclear answer
// hence this is a private function to keep users from crashing their modeules
void EBYTE::Reset() {

	
	SetMode(MODE_PROGRAM);

	delay(50);

	_s->write(0xC4);
	_s->write(0xC4);
	_s->write(0xC4);

	CompleteTask(4000);

	SetMode(MODE_NORMAL);

}

/*
Set the speed of
*/
void EBYTE::SetSpeed(uint8_t val) {
	_Speed = val;
}

/*
Get the speed of
*/
uint8_t EBYTE::GetSpeed() {
	return _Speed ;
}

/*
Set options
*/
void EBYTE::SetOptions(uint8_t val) {
	_Options = val;
}

/*
Get options
*/
uint8_t EBYTE::GetOptions() {
	return _Options;
}

/*
Set the high byte of the address
*/
void EBYTE::SetAddressH(uint8_t val) {
	_AddressHigh = val;
}

/*
Get the high byte of the address
*/
uint8_t EBYTE::GetAddressH() {
	return _AddressHigh;
}

/*
Set the low byte of the address
*/
void EBYTE::SetAddressL(uint8_t val) {
	_AddressLow = val;
}

/*
Get the low byte of the address
*/
uint8_t EBYTE::GetAddressL() {
	return _AddressLow;
}

/*
Set the channel
*/
void EBYTE::SetChannel(uint8_t val) {
	_Channel = val;
}

/*
Get the channel
*/
uint8_t EBYTE::GetChannel() {
	return _Channel;
}

/*
Set the air data rate
*/
void EBYTE::SetAirDataRate(uint8_t val) {
	_AirDataRate = val;

	// part of multiple options in one byte
	// combined here
	BuildSpeedByte();
}

/*
Get the air data rate
*/
uint8_t EBYTE::GetAirDataRate() {
	return _AirDataRate;
}

/*
Set the parity bit
*/
void EBYTE::SetParityBit(uint8_t val) {
	_ParityBit = val;

	// part of multiple options in one byte
	// combined here
	BuildSpeedByte();
}

/*
Get the parity bit
*/
uint8_t EBYTE::GetParityBit( ) {
	return _ParityBit;
}

/*
Set the transmission mode
*/
void EBYTE::SetTransmissionMode(uint8_t val) {
	_OptionTrans = val;

	// part of multiple options in one byte
	// combined here
	BuildOptionByte();
}

/*
Get the transmission mode
*/
uint8_t EBYTE::GetTransmissionMode( ) {
	return _OptionTrans;
}

/*
Set the pullup mode
*/
void EBYTE::SetPullupMode(uint8_t val) {
	_OptionPullup = val;

	// part of multiple options in one byte
	// combined here
	BuildOptionByte();
}

/*
Get the pullup mode
*/
uint8_t EBYTE::GetPullupMode( ) {
	return _OptionPullup;
}

/*
Set the
*/
void EBYTE::SetWORTIming(uint8_t val) {
	_OptionWakeup = val;

	// part of multiple options in one byte
	// combined here
	BuildOptionByte();
}

/*
Get the
*/
uint8_t EBYTE::GetWORTIming() {
	return _OptionWakeup;
}

/*
Set the fec mode
*/
void EBYTE::SetFECMode(uint8_t val) {
	_OptionFEC = val;

	// part of multiple options in one byte
	// combined here
	BuildOptionByte();
}

/*
Set the fec mode
*/
uint8_t EBYTE::GetFECMode( ) {
	return _OptionFEC;
}

/*
Set the transmit power
*/
void EBYTE::SetTransmitPower(uint8_t val) {
	_OptionPower = val;

	// part of multiple options in one byte
	// combined here
	BuildOptionByte();
}

/*
Get the transmit power
*/
uint8_t EBYTE::GetTransmitPower() {
	return _OptionPower;
}

/*
Compute high and low bytes from a 16 bit address
*/
void EBYTE::SetAddress(uint16_t Val) {
	_AddressHigh = ((Val & 0xFFFF) >> 8);
	_AddressLow = (Val & 0xFF);
}

/*
Get the complete address as 16 bit value
*/
uint16_t EBYTE::GetAddress() {
	return (_AddressHigh << 8) | (_AddressLow);
}

/*
Set the UART baud rate
*/
void EBYTE::SetUARTBaudRate(uint8_t val) {
	_UARTDataRate = val;

	// part of multiple options in one byte
	// combined here
	BuildSpeedByte();
}

/*
Get the UART baud rate
*/
uint8_t EBYTE::GetUARTBaudRate() {
	return _UARTDataRate;
}

/*
Build the byte for programming
This is a collection of: Parity
						 UART baud rate
						 Air data rate
*/
void EBYTE::BuildSpeedByte() {
	_Speed = 0;
	_Speed = ((_ParityBit & 0xFF) << 6) | ((_UARTDataRate & 0xFF) << 3) | (_AirDataRate & 0xFF);
}

/*
Build the option byte for programming
This is a collection of: Transmission mode
						 Pullup mode
						 WORTIming
						 FEC mode
						 Transmit power
*/
void EBYTE::BuildOptionByte() {
	_Options = 0;
	_Options = ((_OptionTrans & 0xFF) << 7) | ((_OptionPullup & 0xFF) << 6) | ((_OptionWakeup & 0xFF) << 3) | ((_OptionFEC & 0xFF) << 2) | (_OptionPower&0b11);
}

/*
Save parameters to the module
Either save even when powered down or only temporary
Options are: PERMANENT
			 TEMPORARY
NOTE: All parameters have to be saved at once
*/
void EBYTE::SaveParameters(uint8_t retention) {
	// switch into programming mode
	SetMode(MODE_PROGRAM);
	
	// ignore all bytes in serial interface
	ClearBuffer();
	delay(5);

	// send options
	_s->write(retention);
	_s->write(_AddressHigh);
	_s->write(_AddressLow);
	_s->write(_Speed);
	_s->write(_Channel);
	_s->write(_Options);

	// wait for transmission to complete
	delay(50);
	CompleteTask(4000);
	
	// return to normal operating mode
	SetMode(MODE_NORMAL);
}

/*
Debug method to print all parameters
Can be called anytime after init()
*/
void EBYTE::PrintParameters() {
	_ParityBit = (_Speed & 0XC0) >> 6;
	_UARTDataRate = (_Speed & 0X38) >> 3;
	_AirDataRate = _Speed & 0X07;

	_OptionTrans = (_Options & 0X80) >> 7;
	_OptionPullup = (_Options & 0X40) >> 6;
	_OptionWakeup = (_Options & 0X38) >> 3;
	_OptionFEC = (_Options & 0X07) >> 2;
	_OptionPower = (_Options & 0X03);

	Serial.println("----------------------------------------");
	Serial.print(F("Model no.: "));  Serial.println(_Model, HEX);
	Serial.print(F("Version  : "));  Serial.println(_Version, HEX);
	Serial.print(F("Features : "));  Serial.println(_Features, HEX);
	Serial.println(F(" "));
	Serial.print(F("Mode (HEX/DEC/BIN): "));  Serial.print(_Save, HEX); Serial.print(F("/"));  Serial.print(_Save, DEC); Serial.print(F("/"));  Serial.println(_Save, BIN);
	Serial.print(F("AddH (HEX/DEC/BIN): "));  Serial.print(_AddressHigh, HEX); Serial.print(F("/")); Serial.print(_AddressHigh, DEC); Serial.print(F("/"));  Serial.println(_AddressHigh, BIN);
	Serial.print(F("AddL (HEX/DEC/BIN): "));  Serial.print(_AddressLow, HEX); Serial.print(F("/")); Serial.print(_AddressLow, DEC); Serial.print(F("/"));  Serial.println(_AddressLow, BIN);
	Serial.print(F("Sped (HEX/DEC/BIN): "));  Serial.print(_Speed, HEX); Serial.print(F("/")); Serial.print(_Speed, DEC); Serial.print(F("/"));  Serial.println(_Speed, BIN);
	Serial.print(F("Chan (HEX/DEC/BIN): "));  Serial.print(_Channel, HEX); Serial.print(F("/")); Serial.print(_Channel, DEC); Serial.print(F("/"));  Serial.println(_Channel, BIN);
	Serial.print(F("Optn (HEX/DEC/BIN): "));  Serial.print(_Options, HEX); Serial.print(F("/")); Serial.print(_Options, DEC); Serial.print(F("/"));  Serial.println(_Options, BIN);
	Serial.print(F("Addr (HEX/DEC/BIN): "));  Serial.print(GetAddress(), HEX); Serial.print(F("/")); Serial.print(GetAddress(), DEC); Serial.print(F("/"));  Serial.println(GetAddress(), BIN);
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit (HEX/DEC/BIN)    : "));  Serial.print(_ParityBit, HEX); Serial.print(F("/"));  Serial.print(_ParityBit, DEC); Serial.print(F("/"));  Serial.println(_ParityBit, BIN);
	Serial.print(F("SpeedUARTDataRate (HEX/DEC/BIN) : "));  Serial.print(_UARTDataRate, HEX); Serial.print(F("/"));  Serial.print(_UARTDataRate, DEC); Serial.print(F("/"));  Serial.println(_UARTDataRate, BIN);
	Serial.print(F("SpeedAirDataRate (HEX/DEC/BIN)  : "));  Serial.print(_AirDataRate, HEX); Serial.print(F("/"));  Serial.print(_AirDataRate, DEC); Serial.print(F("/"));  Serial.println(_AirDataRate, BIN);
	Serial.print(F("OptionTrans (HEX/DEC/BIN)       : "));  Serial.print(_OptionTrans, HEX); Serial.print(F("/"));  Serial.print(_OptionTrans, DEC); Serial.print(F("/"));  Serial.println(_OptionTrans, BIN);
	Serial.print(F("OptionPullup (HEX/DEC/BIN)      : "));  Serial.print(_OptionPullup, HEX); Serial.print(F("/"));  Serial.print(_OptionPullup, DEC); Serial.print(F("/"));  Serial.println(_OptionPullup, BIN);
	Serial.print(F("OptionWakeup (HEX/DEC/BIN)      : "));  Serial.print(_OptionWakeup, HEX); Serial.print(F("/"));  Serial.print(_OptionWakeup, DEC); Serial.print(F("/"));  Serial.println(_OptionWakeup, BIN);
	Serial.print(F("OptionFEC (HEX/DEC/BIN)         : "));  Serial.print(_OptionFEC, HEX); Serial.print(F("/"));  Serial.print(_OptionFEC, DEC); Serial.print(F("/"));  Serial.println(_OptionFEC, BIN);
	Serial.print(F("OptionPower (HEX/DEC/BIN)       : "));  Serial.print(_OptionPower, HEX); Serial.print(F("/"));  Serial.print(_OptionPower, DEC); Serial.print(F("/"));  Serial.println(_OptionPower, BIN);

	Serial.println("----------------------------------------");
}

/*
Read all parameters from module
*/
bool EBYTE::ReadParameters() {
	// clear stored parameters
	_Params[0] = 0;
	_Params[1] = 0;
	_Params[2] = 0;
	_Params[3] = 0;
	_Params[4] = 0;
	_Params[5] = 0;

	// switch into programming mode
	SetMode(MODE_PROGRAM);

	// clear serial input buffer
	ClearBuffer();
	delay(5);

	// request parameters
	_s->write(0xC1);
	_s->write(0xC1);
	_s->write(0xC1);
	delay(5);

	// read parameter bytes into array
	_s->readBytes((uint8_t*)&_Params, (uint8_t) sizeof(_Params));
	delay(5);

	// unpack the parameters
	_Save = _Params[0];
	_AddressHigh = _Params[1];
	_AddressLow = _Params[2];
	_Speed = _Params[3];
	_Channel = _Params[4];
	_Options = _Params[5];

	_Address =  (_AddressHigh << 8) | (_AddressLow);
	_ParityBit = (_Speed & 0XC0) >> 6;
	_UARTDataRate = (_Speed & 0X38) >> 3;
	_AirDataRate = _Speed & 0X07;

	_OptionTrans = (_Options & 0X80) >> 7;
	_OptionPullup = (_Options & 0X40) >> 6;
	_OptionWakeup = (_Options & 0X38) >> 3;
	_OptionFEC = (_Options & 0X07) >> 2;
	_OptionPower = (_Options & 0X03);
	
	// return to normal operating mode
	SetMode(MODE_NORMAL);

	// data is only valid, if first parameter is 0xC0
	if (0xC0 != _Params[0]){
		return false;
	}

	return true;
}

/*
Read model information
*/
bool EBYTE::ReadModelData() {
	// clear stored parameters
	_Params[0] = 0;
	_Params[1] = 0;
	_Params[2] = 0;
	_Params[3] = 0;

	// switch into programming mode
	SetMode(MODE_PROGRAM);

	// discard serial input buffer
	ClearBuffer();

	// request model information
	_s->write(0xC3);
	_s->write(0xC3);
	_s->write(0xC3);
	delay(5);

	// read model information
	_s->readBytes((uint8_t*)& _Params, (uint8_t) sizeof(_Params));
	delay(5);
	
	// unpack parameters
	_Save = _Params[0];	
	_Model = _Params[1];
	_Version = _Params[2];
	_Features = _Params[3];

	// return to normal operating mode
	SetMode(MODE_NORMAL);

	// data is only valid if first parameter is 0xC3
	if (0xC3 != _Params[0]) {
		return false;
	}

	return true;
}

/*
Get module model
NOTE: E50-TTL-100 will return 50
*/
uint8_t EBYTE::GetModel() {
	return _Model;
}

/*
Get module version
(undocumented as to the value)
*/
uint8_t EBYTE::GetVersion() {
	return _Version;
}

/*
Get module features
(undocumented as to the value)
*/
uint8_t EBYTE::GetFeatures() {
	return _Features;
}

/*
Clear serial interface input buffer
Will discard any bytes not yet read
*/
void EBYTE::ClearBuffer(){
	// discard byte
	byte b;

	// read all bytes of the input buffer into the discard byte
	while(_s->available()) {
		b = _s->read();
	}
}
