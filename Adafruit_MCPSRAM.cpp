﻿#include "Adafruit_MCPSRAM.h"

#include <Arduino.h>

#include <SPI.h>

Adafruit_MCPSRAM::Adafruit_MCPSRAM(int8_t mosi, int8_t miso, int8_t sck, int8_t cs)
{
	_mosi = mosi;
	_miso = miso;
	_sck = sck;
	_cs = cs;
	hwSPI = false;
}

Adafruit_MCPSRAM::Adafruit_MCPSRAM(int8_t cs)
{
	_cs = cs;
	hwSPI = true;
}

void Adafruit_MCPSRAM::begin()
{
	 pinMode(_cs, OUTPUT);
#ifdef HAVE_PORTREG
    csport      = portOutputRegister(digitalPinToPort(_cs));
    cspinmask   = digitalPinToBitMask(_cs);
#endif
	csHigh();

    if (!hwSPI){
      // set pins for software-SPI
      pinMode(_mosi, OUTPUT);
      pinMode(_sck, OUTPUT);
#ifdef HAVE_PORTREG
      clkport     = portOutputRegister(digitalPinToPort(_sck));
      clkpinmask  = digitalPinToBitMask(_sck);
      mosiport    = portOutputRegister(digitalPinToPort(_mosi));
      mosipinmask = digitalPinToBitMask(_mosi);
	  misoport    = portOutputRegister(digitalPinToPort(_miso));
      misopinmask = digitalPinToBitMask(_miso);
#endif
      }
    if (hwSPI){
      SPI.begin();
#ifndef SPI_HAS_TRANSACTION
      SPI.setClockDivider (4);
#endif
	}
	
csLow();

for(int i=0; i<3; i++){
	  if(hwSPI) {
		(void)SPI.transfer(0xFF);
	  } else {
		for(uint8_t bit = 0x80; bit; bit >>= 1) {
	#ifdef HAVE_PORTREG
		  *clkport &= ~clkpinmask;
		  if(0xFF & bit) *mosiport |=  mosipinmask;
		  else        *mosiport &= ~mosipinmask;
		  *clkport |=  clkpinmask;
	#else
		  digitalWrite(_sck, LOW);
		  if(0xFF & bit) digitalWrite(_mosi, HIGH);
		  else        digitalWrite(_mosi, LOW);
		  digitalWrite(_sck, HIGH);
	#endif
		}
	  }
  }
csHigh();

}

void Adafruit_MCPSRAM::write(uint16_t addr, uint8_t *buf, uint16_t num, uint8_t reg)
{
csLow();

//write command and address
uint8_t cmdbuf[3];
cmdbuf[0] = reg;
cmdbuf[1] = (addr >> 8);
cmdbuf[2] = addr & 0xFF;
	
for(int i=0; i<3; i++){

  uint8_t d = cmdbuf[i];
	
  if(hwSPI) {
    (void)SPI.transfer(d);
  } else {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#ifdef HAVE_PORTREG
      *clkport &= ~clkpinmask;
      if(d & bit) *mosiport |=  mosipinmask;
      else        *mosiport &= ~mosipinmask;
      *clkport |=  clkpinmask;
#else
      digitalWrite(_sck, LOW);
      if(d & bit) digitalWrite(_mosi, HIGH);
      else        digitalWrite(_mosi, LOW);
      digitalWrite(_sck, HIGH);
#endif
    }
  }
  if(reg != K640_WRITE) break;
}

//write buffer of data
for(int i=0; i<num; i++){

  uint8_t d = buf[i];
	
  if(hwSPI) {
    (void)SPI.transfer(d);
  } else {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#ifdef HAVE_PORTREG
      *clkport &= ~clkpinmask;
      if(d & bit) *mosiport |=  mosipinmask;
      else        *mosiport &= ~mosipinmask;
      *clkport |=  clkpinmask;
#else
      digitalWrite(_sck, LOW);
      if(d & bit) digitalWrite(_mosi, HIGH);
      else        digitalWrite(_mosi, LOW);
      digitalWrite(_sck, HIGH);
#endif
    }
  }
  
}
  
csHigh();
}

void Adafruit_MCPSRAM::read(uint16_t addr, uint8_t *buf, uint16_t num, uint8_t reg)
{

csLow();

//write command and address
uint8_t cmdbuf[3];
cmdbuf[0] = reg;
cmdbuf[1] = (addr >> 8);
cmdbuf[2] = addr & 0xFF;
for(int i=0; i<3; i++){

  uint8_t d = cmdbuf[i];
	
  if(hwSPI) {
    (void)SPI.transfer(d);
  } else {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#ifdef HAVE_PORTREG
      *clkport &= ~clkpinmask;
      if(d & bit) *mosiport |=  mosipinmask;
      else        *mosiport &= ~mosipinmask;
      *clkport |=  clkpinmask;
#else
      digitalWrite(_sck, LOW);
      if(d & bit) digitalWrite(_mosi, HIGH);
      else        digitalWrite(_mosi, LOW);
      digitalWrite(_sck, HIGH);
#endif
    }
  }
  if(reg != K640_READ) break;
}

//read data into buffer
for(int i=0; i<num; i++){
	
  if(hwSPI) {
    buf[i] = SPI.transfer(0x00);
  } else {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#ifdef HAVE_PORTREG
      *clkport &= ~clkpinmask;
      *clkport |=  clkpinmask;
	   buf[i] = (buf[i] << 1) | = *misoport & misoport;
#else
      digitalWrite(_sck, LOW);
      digitalWrite(_sck, HIGH);
	  
	  buf[i] = (buf[i] << 1) | digitalRead(_miso);
#endif
    }
  }
  
}
csHigh();
}
	
uint8_t Adafruit_MCPSRAM::read8(uint16_t addr, uint8_t reg)
{
	uint8_t c;
	this->read(addr, &c, 1, reg);
	return c;
}

uint16_t Adafruit_MCPSRAM::read16(uint16_t addr)
{
	uint8_t b[2];
	this->read(addr, b, 2);
	return ((uint16_t)b[0] << 8) | b[1];
}

void Adafruit_MCPSRAM::write8(uint16_t addr, uint8_t val, uint8_t reg)
{
	this->write(addr, &val, 1, reg);
}

void Adafruit_MCPSRAM::write16(uint16_t addr, uint16_t val)
{
	uint8_t b[2];
	b[0] = (val >> 8);
	b[1] = (val);
	this->write(addr, b, 2);
}

void Adafruit_MCPSRAM::erase(uint16_t addr, uint16_t length, uint8_t val)
{
	csLow();
	//write command and address
	uint8_t cmdbuf[3];
	cmdbuf[0] = K640_WRITE;
	cmdbuf[1] = (addr >> 8);
	cmdbuf[2] = addr & 0xFF;
	
	for(int i=0; i<3; i++){

		uint8_t d = cmdbuf[i];
		
		if(hwSPI) {
			(void)SPI.transfer(d);
			} else {
			for(uint8_t bit = 0x80; bit; bit >>= 1) {
				#ifdef HAVE_PORTREG
				*clkport &= ~clkpinmask;
				if(d & bit) *mosiport |=  mosipinmask;
				else        *mosiport &= ~mosipinmask;
				*clkport |=  clkpinmask;
				#else
				digitalWrite(_sck, LOW);
				if(d & bit) digitalWrite(_mosi, HIGH);
				else        digitalWrite(_mosi, LOW);
				digitalWrite(_sck, HIGH);
				#endif
			}
		}
	}

	//write buffer of data
	for(uint16_t i=0; i<length; i++){

		uint8_t d = val;
		
		if(hwSPI) {
			(void)SPI.transfer(d);
			} else {
			for(uint8_t bit = 0x80; bit; bit >>= 1) {
				#ifdef HAVE_PORTREG
				*clkport &= ~clkpinmask;
				if(d & bit) *mosiport |=  mosipinmask;
				else        *mosiport &= ~mosipinmask;
				*clkport |=  clkpinmask;
				#else
				digitalWrite(_sck, LOW);
				if(d & bit) digitalWrite(_mosi, HIGH);
				else        digitalWrite(_mosi, LOW);
				digitalWrite(_sck, HIGH);
				#endif
			}
		}
		
	}
	
	csHigh();
}

void Adafruit_MCPSRAM::csHigh()
{
#ifdef SPI_HAS_TRANSACTION
      SPI.endTransaction();
#endif
#ifdef HAVE_PORTREG
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif
}

void Adafruit_MCPSRAM::csLow()
{
#ifdef SPI_HAS_TRANSACTION
      SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif	
#ifdef HAVE_PORTREG
	*csport &= ~cspinmask;
#else
	digitalWrite(_cs, LOW);
#endif
}