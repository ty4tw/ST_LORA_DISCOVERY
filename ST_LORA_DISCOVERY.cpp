/*
 * ST_LORA_DISCOVERY.cpp
 *
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
#include <KashiwaGeeks.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <stdarg.h>
#include <LoRaWANDef.h>
#include <ST_LORA_DISCOVERY.h>

using namespace tomyApplication;
int getFreeMemory(void);
extern PortList_t thePortList[];

#define JOIN_RETRY_CNT   3
//
//
//     Class ST_LORA_DISCOVERY
//
//

static String nullString = "";

ST_LORA_DISCOVERY::ST_LORA_DISCOVERY(uint8_t rxPin, uint8_t txPin)
{
    _serialPort = new SoftwareSerial(rxPin, txPin);
    pinMode(rxPin, INPUT);
    _txRetryCount = 1;
    _joinRetryCount = JOIN_RETRY_CNT;
    _maxPayloadSize = LoRa_DEFAULT_PAYLOAD_SIZE;
}

ST_LORA_DISCOVERY::~ST_LORA_DISCOVERY(void)
{
	// Intentionally blank
}

void ST_LORA_DISCOVERY::setDevEUI(String devEUI)
{
	String returnVal;
	int rc = send(F("DEUI"), F("="), devEUI, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);
	if ( rc != LORA_RC_SUCCESS)
	{
		DebugPrint(F("Can't set DEVEUI\n"));
	}
	while(true);
}

void ST_LORA_DISCOVERY::setAppEUI(String appEUI)
{
	String returnVal;
	int rc = send(F("DEUI"), F("="), appEUI, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);
	if ( rc != LORA_RC_SUCCESS)
	{
		DebugPrint(F("Can't set DEVEUI\n"));
	}
	while(true);
}

void ST_LORA_DISCOVERY::setAppKey(String appKey)
{
	String returnVal;
	int rc = send(F("APPKEY"), F("="), appKey, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);
	if ( rc != LORA_RC_SUCCESS)
	{
		DebugPrint(F("Can't set DEVEUI\n"));
	}
	while(true);
}


void ST_LORA_DISCOVERY::begin(LoRaDR dr)
{
    _txTimeoutValue = LoRa_RECEIVE_DELAY2;
    _serialPort->setTimeout(LoRa_SERIAL_WAIT_TIME);
    _minDR = dr;
    _joinRetryCount = JOIN_RETRY_CNT;
}


void ST_LORA_DISCOVERY::clearCmd(void)
{
	String returnVal;
    send(nullString, nullString, nullString, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);
}


void ST_LORA_DISCOVERY::sleep(void)
{
	// Intentionally blank
}

void ST_LORA_DISCOVERY::wakeup(void)
{
	// Intentionally blank
}

bool ST_LORA_DISCOVERY::isJoin(void)
{
    return ( _joinStatus == joined );
}

bool ST_LORA_DISCOVERY::join(void)
{
	clearCmd();

    for ( uint8_t i = 0; i < _joinRetryCount; i++ )
    {
        DebugPrint(F("try to join... "));
        if ( connect() )
        {
            DebugPrint(F("accepted.\n"));
            return true;
        }
        for ( uint8_t j = 0; j < 40 && i == _joinRetryCount; j++ )
        {
            delay(1000);  // Duty 1%
        }
    }
    DebugPrint(F("fail to join.\n"));
    return false;
}

bool ST_LORA_DISCOVERY::connect(void)
{
    String returnVal;

    clearCmd();
    send(F("NJM"), F("="), F("1"), &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);
	send(F("JOIN"), nullString, nullString, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);
	delay(LoRa_RECEIVE_DELAY2);

	send(F("NJS"), F("=?"), nullString, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);

	if ( returnVal == "1" )
	{
		_joinStatus = joined;
		return true;
	}
	else
	{
		_joinStatus = not_joined;
		return false;
	}
}

//
//
//    Getters
//
//
uint8_t ST_LORA_DISCOVERY::getMaxPayloadSize(void)
{
    return _maxPayloadSize;
}


//
//
//  sendCmd( )
//  Return value: LoRa_RV_SUCCESS, LoRa_RV_DATA_TOO_LONG, LoRa_RV_NOT_JOINED, LoRa_RV_ERROR
//
//
int ST_LORA_DISCOVERY::send(String cmdBase, String cmdParam, String cmdValue, String* returnVal, bool echo, uint32_t timeout)
{
    String  cmdStart = "AT";
    cmdStart += ( cmdBase == "" ? "" : "+" );
    String cmdEnd = "\n\r";

    _serialPort->listen();

    if ( echo )
    {
        ConsolePrint(F("\nSend  =>%s%s%s%s<=\n"), cmdStart.c_str(),cmdBase.c_str(),cmdParam.c_str(),cmdValue.c_str() );
    }

    _serialPort->print(cmdStart + cmdBase + cmdParam + cmdValue + cmdEnd);
    _serialPort->flush();
    delay(200);

    // Recive a response from the module.
    String resp;
    uint32_t  time = (timeout == 0 ? _txTimeoutValue : timeout );
    uint32_t tim = millis() + time + 100;
    uint8_t cnt = ( cmdParam == "=?" ? 3 : 2 );
    int pos = 0;

    while (millis() < tim)
    {
        if (_serialPort->available() > 0)
        {
          char ch = _serialPort->read();
          resp += String(ch);
          LoRaDebug(F("%02x "), ch);
          pos = resp.indexOf(cmdEnd, pos);
          if ( pos >= 0 )
          {
        	  cnt--;
			  if ( cnt == 0 )
			  {
				  break;
			  }
          }
        }
    }

    if ( echo )
    {
        ConsolePrint(F("\nRecv  =>%s<=\n"), resp.c_str());
    }

    String status = "";
    *returnVal = "";

    if ( cnt == 0 )
    {
    	int pos = 0;
    	pos = resp.indexOf(cmdEnd, pos);

		if ( pos > 0)
		{
			// value <CR><LF><CR><LF>status<CR><LF>
			*returnVal = resp.substring(0, pos);
			pos += 2;
		}

		if ( pos > -1 )
		{
			// <CR><LF>status<CR><LF>
			pos += 2;
			int end = resp.indexOf(cmdEnd, pos);
			status = resp.substring(pos, end);
		}
    }
    else
    {
    	resp = F("AT_ERROR");
    }


    // Check the response with specified response string.

    int rc;

    if ( status == "OK" )
    {
        rc = LORA_RC_SUCCESS;
    }
    else if (status == "AT_NO_NETWORK_JOINED" )
    {
        _joinStatus = not_joined;
        rc = LORA_RC_NOT_JOINED;
    }
    else if (status == "AT_TEST_PARAM_OVERFLOW" )
    {
        rc = LORA_RC_DATA_TOO_LONG;
    }
    else if ( status == "AT_BUSY_ERROR" )
    {
        rc = LORA_RC_BUSY;
    }
    else
    {
        rc = LORA_RC_ERROR;
    }
#ifdef FREE_MEMORY_CHECK
    ConsolePrint(F("Free RAM is %d bytes\n"), getFreeMemory());
#endif
    return rc;
}

//
//
//  sendData( )
//  Return value: LoRa_RV_SUCCESS, LoRa_RV_DATA_TOO_LONG, LoRa_RV_NOT_JOINED, LoRa_RV_ERROR
//
//

int ST_LORA_DISCOVERY::sendPayload(uint8_t port, bool echo, Payload* payload)
{
    return transmitBinaryData(port, echo, false, payload);
}


int ST_LORA_DISCOVERY::sendPayloadConfirm(uint8_t port, bool echo, Payload* payload)
{
    return transmitBinaryData(port, echo, true, payload);
}


int ST_LORA_DISCOVERY::transmitBinaryData(uint8_t port, bool echo, bool confirm, Payload* payload)
{
	String returnVal = "";
	String cmdParam;

    if ( _joinStatus == not_joined )
    {
        LoRaDebug(F("Module is not joined.  try to join.\n"));
        if ( !join() )
        {
            _stat = LORA_RC_NOT_JOINED;
            return _stat;
        }
    }

    if ( confirm )
    {
    	cmdParam = "1";
    }
    else
    {
    	cmdParam = "0";
    }

    send(F("CFM"), cmdParam, nullString, &returnVal, ECHOFLAG, LoRa_INIT_WAIT_TIME);

    char buf[4];
    sprintf(buf, "%d", port);
    String cmdValue = buf + ':';
    for ( int i = 0; i < payload->getLen(); i++ )
    {
    	uint8_t* bd = payload->getRowData() + i;
    	sprintf(buf, "%02X", *bd);
    	cmdValue += buf;
    }

    _stat = send(F("SENDB"), F("="), cmdValue, &returnVal, echo, _txTimeoutValue);
    return _stat;
}

Payload* ST_LORA_DISCOVERY::getDownLinkPayload(void)
{
    _payload.create(_maxPayloadSize);
    getDownLinkBinaryData(_payload.getRowData());
    return &_payload;
}

uint8_t ST_LORA_DISCOVERY::getDownLinkPort( void)
{
    int epos = _downLinkData.lastIndexOf(F(":"));
    if ( epos >= 0 )
    {
        String port =  _downLinkData.substring(0, epos);
        return atoi(port.c_str());
    }
    return 0;
}


uint8_t ST_LORA_DISCOVERY::getDownLinkBinaryData(uint8_t* data)
{
    int bpos = _downLinkData.indexOf(F(":"));
	if ( bpos >= 0 )
	{
	   uint8_t i = 0;
	   for (bpos++ ; bpos < (uint8_t)_downLinkData.length(); bpos += 2 )
	   {
		   data[i] = ctoh(_downLinkData[bpos]);
		   data[i] = data[i] << 4;
		   data[i++]  |= ctoh(_downLinkData[bpos + 1]);
	   }
	   return i;
	}
	return 0;
}

uint8_t ST_LORA_DISCOVERY::ctoh(uint8_t ch)
{
    if ( ch >= 0x30 && ch <=0x39 )
    {
        return ch - 0x30;
    }
    else if ( ch >= 0x41 && ch <= 0x46 )
    {
        return ch - 0x37;
    }
    return 0;
}


void ST_LORA_DISCOVERY::checkDownLink(void)
{
	_stat = send(F("RECVB"), F("=?"), nullString, &_downLinkData, ECHOFLAG, _txTimeoutValue);

    if ( _stat == LORA_RC_SUCCESS )
    {
        uint8_t port = getDownLinkPort();
        if ( port )
        {
            for ( uint8_t i = 0; thePortList[i].port != 0;  i++ )
            {
                if ( port == thePortList[i].port  )
                {
                    thePortList[i].callback();
                    break;
                }
            }
        }
    }
}

int ST_LORA_DISCOVERY::reset(void)
{
	String nullString = "";
	String returnVal;
    return send(F("ATZ"), nullString, nullString, &returnVal, ECHOFLAG, 0);
}



