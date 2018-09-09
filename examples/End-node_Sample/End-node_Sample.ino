#include <KashiwaGeeks.h>
#include <ST_LORA_DISCOVERY.h>
#include "Commissioning.h"

#define ECHO true

ST_LORA_DISCOVERY LoRa(8,9);

//================================
//    Initialize Device Function
//================================

void start()
{
    ConsoleBegin(57600);
    //DisableConsole();
    //DisableDebug();

    /*
     * Enable Interrupt 0 & 1  Uncomment the following two  lines.
     */
    //EnableInt0();
    //EnableInt1();  // For ADB922S, CUT the pin3 of the Sheild.


    ConsolePrint(F("\n**** End-node_Sample *****\n"));

    /*  setup Power save Devices */
    //power_adc_disable();          // ADC converter
    //power_spi_disable();          // SPI
    //power_timer1_disable();   // Timer1
    //power_timer2_disable();   // Timer2, tone()
    //power_twi_disable();         // I2C

    /*  setup ST_LORA_DISCOVERY  */
    LoRa.begin(DR3);

    /* setup EUI & Key */
    LoRa.setDevEUI(LORAWAN_DEVICE_EUI);
    LoRa.setAppEUI(LORAWAN_APPLICATION_EUI);
    LoRa.setAppKey(LORAWAN_APPLICATION_KEY);

    /*  join LoRaWAN */
    LoRa.join();
}

//================================
//     Power save functions
//================================
void sleep(void)
{
    DebugPrint(F("LoRa sleep.\n"));

    //
    //  ToDo: Set Device to Power saving mode
    //
}

void wakeup(void)
{
    DebugPrint(F("LoRa wakeup.\n"));
   //
   //  ToDo: Set Device to Power On mode
   //
}

//================================
//     INT0, INT2 callbaks
//================================
void int0D2(void)
{
  ConsolePrint(F("\nINT0 !!!\n"));
  task1();
}

void int1D3(void)
{
  ConsolePrint(F("\nINT1 !!!\n"));
  task2();
}

//================================
//    Functions to be executed periodically
//================================
#define LoRa_Port_NORM  12
#define LoRa_Port_COMP  13

float bme_temp = 10.2;
float bme_humi = 20.2;
float bme_press = 50.05;

/*-------------------------------------------------------------*/
void task1(void)
{
    char buf[16];
    ConsolePrint(F("\n  Task1 invoked\n\n"));
    ConsolePrint(F("Temperature:  %s degrees C\n"), dtostrf(bme_temp, 6, 2, buf));
    ConsolePrint(F("%%RH: %s %%\n"), dtostrf(bme_humi, 6, 2, buf));
    ConsolePrint(F("Pressure: %s Pa\n"), dtostrf(bme_press, 6, 2, buf));

    Payload pl(LoRa.getMaxPayloadSize());
    pl.set_float(bme_temp);
    pl.set_float(bme_humi);
    pl.set_float(bme_press);

    int rc = LoRa.sendPayload(LoRa_Port_COMP, ECHO, &pl);
    checkResult(rc);
}

/*-------------------------------------------------------------*/
void task2(void)
{
    ConsolePrint(F("\n  Task2 invoked\n\n"));
    ConsolePrint(F("Temperature:  %d degrees C\n"), temp);
    ConsolePrint(F("%%RH: %d %%\n"), humi);
    ConsolePrint(F("Pressure: %d Pa\n"), press);
}


/*-------------------------------------------------------------*/
void checkResult(int rc )
{
    if ( rc == LORA_RC_SUCCESS )
    {
        ConsolePrint(F("\n SUCCESS\n"));
    }
    else if ( rc == LORA_RC_DATA_TOO_LONG )
    {
      ConsolePrint(F("\n !!!DATA_TOO_LONG\n"));
    }
    else if ( rc == LORA_RC_NO_FREE_CH )
    {
      ConsolePrint(F("\n !!!No free CH\n"));
    }
    else if ( rc == LORA_RC_BUSY )
    {
      ConsolePrint(F("\n !!!Busy\n"));
    }
    else if ( rc ==LORA_RC_NOT_JOINED )
    {
      ConsolePrint(F("\n !!!Not Joined\n"));
    }
    else if ( rc == LORA_RC_ERROR )
    {
     ConsolePrint(F("\n !!!UNSUCCESS\n"));
    }
}

//===============================
//            Execution interval
//     TASK( function, initial offset, interval by minute )
//===============================

TASK_LIST = {
        TASK(task1, 0, 1),
        TASK(task2, 1, 2),
        END_OF_TASK_LIST
};


/*   End of Program  */
