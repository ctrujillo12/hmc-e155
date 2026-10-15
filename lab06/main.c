/*
main.c
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

char* webpageStart = "<!DOCTYPE html><html><head><title>DS1722 Temperature Sensor</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>DS1722 Temperature Sensor</h1>";

char* resStr = "<h2>Resolution Control:</h2>\
	<form action=\"12bit\"><input type=\"submit\" value=\"12 bits\"></form>\
	<form action=\"11bit\"><input type=\"submit\" value=\"11 bits\"></form>\
	<form action=\"10bit\"><input type=\"submit\" value=\"10 bits\"></form>\
	<form action=\"9bit\"><input type=\"submit\" value=\"9 bits\"></form>\
	<form action=\"8bit\"><input type=\"submit\" value=\"8 bits\"></form>";

char* ledStr = "<h2>LED Control:</h2><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";

char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

// led state stays same between requests, only ledon/ledoff change it
int updateLEDStatus(char request[])
{
	static int led_status = 0;

	if (inString(request, "ledoff") == 1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon") == 1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

// Returns the DS1722 config byte for a resolution button, or 0 if the request isn't one
uint8_t updateResStatus(char request[])
{
	if (inString(request, "12bit") == 1) return DS1722_CFG_12BIT;
	if (inString(request, "11bit") == 1) return DS1722_CFG_11BIT;
	if (inString(request, "10bit") == 1) return DS1722_CFG_10BIT;
	if (inString(request, "9bit")  == 1) return DS1722_CFG_9BIT;
	if (inString(request, "8bit")  == 1) return DS1722_CFG_8BIT;
	return 0;
}

// Number of resolution bits for a config byte (R2 -> 12, else 8 + R1:R0)
int resBitsFromCfg(uint8_t cfg)
{
	if (cfg & 0x08) return 12;
	return 8 + ((cfg >> 1) & 0x3);
}

// Writes value / 10^decimals as text (e.g. 235625 with decimals=4 -> "23.5625") at p.
// Handles negatives and zero-padded fractions without using printf. Returns pointer past the end.
char* fmtFixed(char *p, int value, int decimals)
{
	int scale = 1;
	for (int i = 0; i < decimals; i++) scale *= 10;

	if (value < 0) { *p++ = '-'; value = -value; }
	int whole = value / scale;
	int frac  = value % scale;

	char tmp[12];
	int n = 0;
	if (whole == 0) tmp[n++] = '0';
	while (whole > 0) { tmp[n++] = '0' + (whole % 10); whole /= 10; }
	while (n > 0) *p++ = tmp[--n];

	if (decimals > 0) {
		*p++ = '.';
		int div = scale / 10;
		for (int i = 0; i < decimals; i++) { *p++ = '0' + (frac / div) % 10; div /= 10; }
	}
	*p = '\0';
	return p;
}

void configurePins(void)
{
	pinMode(PA5, GPIO_ALT);    // SPI1_SCK
	pinMode(PA6, GPIO_ALT);    // SPI1_MISO
	pinMode(PB5, GPIO_ALT);    // SPI1_MOSI
	pinMode(DS1722_CE_PIN, GPIO_OUTPUT); // DS1722 CE (active high)
	digitalWrite(DS1722_CE_PIN, 0);      // idle low

	// Very high output speed for SCK, MISO, MOSI (2 bits per pin: PA5 -> [11:10], PA6 -> [13:12], PB5 -> [11:10])
	GPIOA->OSPEEDR |= ((3 << 10) | (3 << 12));
	GPIOB->OSPEEDR |= (3 << 10);

	// AF5 = SPI1 on PA5 ([23:20]), PA6 ([27:24]) and PB5 ([23:20]) (AFRL, 4 bits per pin)
	GPIOA->AFR[0] &= ~((0xFU << 20) | (0xFU << 24));
	GPIOB->AFR[0] &= ~(0xFU << 20);
	GPIOA->AFR[0] |=  ((5U << 20) | (5U << 24));
	GPIOB->AFR[0] |=  (5U << 20);
}

int main(void) {
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(LED_PIN, GPIO_OUTPUT);

  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);

  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  // SPI: pins first, then the peripheral (initSPI also enables the SPI1 clock)
  configurePins();
  initSPI(0b111, 0, 1);   // slowest clock (PCLK/256), CPOL=0, CPHA=1 as the DS1722 requires

  initTempSensor();       // 12-bit continuous; waits for the first conversion
  int resBits = 12;



  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;

    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      char c = readChar(USART);
      if (charIndex < BUFF_LEN - 1) request[charIndex++] = c;  // don't overflow the buffer
    }

    // Change resolution only if a resolution button was pressed
    uint8_t cfg = updateResStatus(request);
    if (cfg) {
      writeToTempSensor(cfg);        // waits one full conversion at the new resolution
      resBits = resBitsFromCfg(cfg);
    }

    // Read temperature and format it without needing printf float support
    double temp = readFromTempSensor();
    int tenThousandths = (int)(temp * 10000.0 + (temp >= 0 ? 0.5 : -0.5));
    double tempF = temp * 9.0 / 5.0 + 32.0;
    int hundredthsF = (int)(tempF * 100.0 + (tempF >= 0 ? 0.5 : -0.5));
    char tempStatusStr[96];
    char *p = tempStatusStr;
    strcpy(p, "Temperature: ");  p += strlen(p);
    p = fmtFixed(p, tenThousandths, 4);
    strcpy(p, " C / ");          p += strlen(p);
    p = fmtFixed(p, hundredthsF, 2);
    strcpy(p, " F (");           p += strlen(p);
    p = fmtFixed(p, resBits, 0);
    strcpy(p, "-bit)");

    // Update string with current LED state
    int led_status = updateLEDStatus(request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr, "LED is on!");
    else
      sprintf(ledStatusStr, "LED is off!");

    // transmit the webpage over UART
    sendString(USART, webpageStart);

    sendString(USART, "<h2>Temperature</h2><p>");
    sendString(USART, tempStatusStr);
    sendString(USART, "</p>");
    sendString(USART, resStr);

    sendString(USART, ledStr);
    sendString(USART, "<h2>LED Status</h2><p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, webpageEnd);
  }
}



