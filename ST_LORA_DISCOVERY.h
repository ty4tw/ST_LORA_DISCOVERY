/*
 * ST_LORA_DISCOVERY.h
 *
 *                       The MIT License
 *
 *   Copyright (c) 2018     Tomoaki Yamaguchi    tomoaki@tomy-tech.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ST_LORA_DISCOVERY_H_
#define ST_LORA_DISCOVERY_H_
#include <KashiwaGeeks.h>
#include <SoftwareSerial.h>
namespace tomyApplication
{

class ST_LORA_DISCOVERY
{
public:
	ST_LORA_DISCOVERY(uint8_t rxPin, uint8_t txPin);
    ~ST_LORA_DISCOVERY(void);

    void begin(LoRaDR dr = DR2);
    bool join(void);
    bool isJoin(void);

    int sendPayload(uint8_t port, bool echo, Payload* payload);
    int sendPayloadConfirm(uint8_t port, bool echo, Payload* payload);

    uint8_t getDownLinkPort( void);
    Payload* getDownLinkPayload(void);
    String getDownLinkData(void);
    void checkDownLink(void);

     int setDr(LoRaDR dr);

    uint8_t getMaxPayloadSize(void);

    int reset(void);
    void sleep(void);
    void wakeup(void);

    void setDevEUI(const char* devEUI);
    void setAppEUI(const char* appEUI);
    void setAppKey(const char* appKey);

private:
    uint8_t getDr(void);

    uint8_t getDownLinkBinaryData(uint8_t* data);
    void clearCmd(void);
    bool connect(void);
    bool isConnected(void);
    int transmitBinaryData( uint8_t port, bool echo, bool ack, Payload* payload);
    int send(String cmdBase, String cmdParam, String cmdValue, String* returnVal, bool echo, uint32_t timeout = LoRa_INIT_WAIT_TIME);
    uint8_t ctoh(uint8_t ch);
    void checkRecvData(char* buff, bool conf);

    SoftwareSerial*  _serialPort;
    uint32_t  _baudrate {9600};
    JoineStatus  _joinStatus {not_joined};
    uint8_t  _txRetryCount {0};
    uint8_t _joinRetryCount;
    uint8_t  _maxPayloadSize;
    uint32_t  _txTimeoutValue {LoRa_RECEIVE_DELAY2};
    String  _downLinkData;
    Payload _payload;
    int  _stat{0};
    LoRaDR _minDR{DR2};
};

}
#endif /* ST_LORA_DISCOVERY_H_ */
