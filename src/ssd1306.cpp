#include <Arduino.h>
#include <ssd1306.h>

// display, collegato sui pin predefiniti di Wire0, senza linea di reset:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);
// Create a U8g2log object
U8G2LOG u8g2log;
// assume 6x8 font, define width and height
#define U8LOG_WIDTH 21
#define U8LOG_HEIGHT 6

// coda per i messaggi sul display SSD1306
QueueHandle_t xQueueMsgU8G2;

// allocate memory
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

// handle del task di stampa display SSD1306
TaskHandle_t u8logTaskHandle;
// task di stampa display SSD1306
void u8logTask(void *pvParameters);

// display setup
void ssd1306_log_setup()
{
    // setup SSD1306 display
    u8g2.begin();
    // set the font for the terminal window
    u8g2.setFont(u8g2_font_t0_11_mf);
    // connect to u8g2, assign buffer
    u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
    // set extra space between lines in pixel, this can be negative
    u8g2log.setLineHeightOffset(0);
    // 0: Update screen with newline, 1: Update screen for every char
    u8g2log.setRedrawMode(0);

    // creazione coda per pubblicazione messaggi SSD1306
    xQueueMsgU8G2 = xQueueCreate(10, (U8LOG_WIDTH + 1) * sizeof(uint8_t));

  // crea il task di pubblicazione dei messaggi su display SSD1306
   BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      u8logTask,         // function that implements the task
      "u8logTask",       // name for the task
      4096,               // task size
      NULL,               // parameter passed into the task
      1,                  // task priority
      &u8logTaskHandle, // the task's handle
      1                   // pinned to core 1
  );
  if (xReturned != pdPASS)
  {
    Serial.println(F("Errore nella creazione del task u8logTask"));
    while (1)
    {
      yield();
    }
  }    
}

//-----------------------------------------------------------------------------
// task di stampa display SSD1306
void u8logTask(void *pvParameters)
{
    char temp[U8LOG_WIDTH + 1]="";
    while (1)
    {
        while (uxQueueMessagesWaiting(xQueueMsgU8G2) > 0)
        {
            xQueueReceive(xQueueMsgU8G2, &(temp[0]), portMAX_DELAY);
            //Serial.printf("%s", temp);
            u8g2log.print(temp);
        }
        delay(1);
    }
}

// publish message
void ssd1306_publish(const char *s) {
    uint8_t l = strlen(s);
    char t[U8LOG_WIDTH+2] = "";
    
    strncpy(t, s, (l>U8LOG_WIDTH ? U8LOG_WIDTH : l));

    if(l > U8LOG_WIDTH) {
        t[U8LOG_WIDTH+1] = '\0';
    }
      // inserimento in coda
    if (uxQueueSpacesAvailable(xQueueMsgU8G2) > 0)
    {
      xQueueSendToBack(xQueueMsgU8G2, (void *)t, portMAX_DELAY);
    }
}