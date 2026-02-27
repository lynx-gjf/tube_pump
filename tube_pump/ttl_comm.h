#pragma once
#ifndef TTL_COMM_H
#define TTL_COMM_H

#include <stdint.h>
#include <Arduino.h>

constexpr unsigned long TTL_BAUD = 9600;
constexpr unsigned long RX_WAIT_MS = 100; // default ms to wait when reading

// Configure a TTL channel on specified RX/TX pins at given baud (AVR: SoftwareSerial)
void configureTTL(uint8_t txPin, uint8_t rxPin, unsigned long baud = TTL_BAUD);

// Send a String to a channel (txPin, rxPin order kept for compatibility with callers)
void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data);

// Read from a channel (txPin, rxPin order kept for compatibility with callers)
String readFromChannel(uint8_t txPin, uint8_t rxPin, unsigned long timeoutMs = RX_WAIT_MS);


#endif // TTL_COMM_H