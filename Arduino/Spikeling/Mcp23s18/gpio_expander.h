// -----------------------------------------------------------------------------     
// Adapted from https://github.com/sumotoy/gpio_expander
// by Thomas Euler (2018-08-18)
//
// -----------------------------------------------------------------------------     
/*
	gpio_expander - An attemp to create a fast and universal library for drive many GPIO chips
	
	Features:
	
	- Can drive many type of GPIO's chip from different makers.
	- Use almost same commands for all chips so they can be easily exchanged.
	- Can access special registers of some chip.

    Copyright (c) 2013-2014, sumotoy, coded by Max MC Costa.    

    gpio_expander Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gpio_expander Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

	
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Version:0.8b3 - Fixed an error that blocks I/O 16 in many functions
	(thanks Thorsten to point me this)
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
// -----------------------------------------------------------------------------     
#ifndef _GPIO_EXPANDER_H_
#define _GPIO_EXPANDER_H_

#include <inttypes.h>
#include <Arduino.h>

#if defined(ESP32) || defined(ESP8622)
#else
    #include <avr/interrupt.h>
#endif  
#if defined(__MK20DX128__) || defined(__MK20DX256__) //Teensy 3.0 or 3.1
	#define __FASTWRITE
#endif

// -----------------------------------------------------------------------------     
class gpio_expander {

public:
	gpio_expander( );

	virtual void 	begin(bool protocolInitOverride=false) = 0;
	
	//void 			command();								

	
protected:
	inline uint16_t		byte2word(byte high_byte,byte low_byte){return (word)high_byte << 8 | (word)low_byte;};
	inline byte			word2highByte(uint16_t data){return (byte)(data >> 8);};
	inline byte			word2lowByte(uint16_t data){return (byte)(data & 0x00FF);};

private:

};
#endif
// -----------------------------------------------------------------------------     