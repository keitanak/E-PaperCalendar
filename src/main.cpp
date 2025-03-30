#include <Arduino.h>

#include <STM32RTC.h>

#include "STM32LowPower.h"

#include <JJYReceiver.h>

#include <holidays.h>

/* Change these values to set the initial time after powerloss */
const byte seconds = 0;
const byte minutes = 58;
const byte hours = 23;

/* Change these values to set the initial date after powerloss */
/* Saturday 1st March 2025 */
// week day: 1-7 (Monday first)
const byte weekDay = 7;
const byte day =1;
const byte month = 3;
const byte year = 25;

// To preserve the current dates. Reset these valuse to default.
int curyear=0,curmonth=0,curday=0;


// define pin for JJY module
#define DATA PB3
#define PON PB4
#define SEL PB5

// define pin for JJY status LED
#define GLED PA8
#define RLED PA9

// TIMEout for JJY recieve (ms)
#define JJYTIMEOUT 300000

//const int pin = USER_BTN;
const int pin = PA10;
const int pin2 = PA12;

//For power control of epaper
#define EPDPWR PA11

// 10msec Timer for clock ticktock (Mandatory)


#if defined(TIM1)
  TIM_TypeDef *Instance = TIM1;
#else
  TIM_TypeDef *Instance = TIM2;
#endif

HardwareTimer *MyTim = new HardwareTimer(Instance);

JJYReceiver jjy(DATA,SEL,PON); // JJYReceiver lib set up.
bool jjyreceiving = false;
unsigned long receievestarttime;

int currentcalmode=0,calmode = 0;
// 0= single month,  1 = 9 months calendar.

/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

// uncomment next line to use HSPI for EPD (and e.g VSPI for SD), e.g. with Waveshare ESP32 Driver Board
//#define USE_HSPI_FOR_EPD

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>

//#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
//#include <GxEPD2_4C.h>
//#include <GxEPD2_7C.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// select the display constructor line in one of the following files (old style):
//#include "GxEPD2_display_selection.h"
//#include "GxEPD2_display_selection_added.h"

// or select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

#if !defined(__AVR) && !defined(STM32F1xx)

// note 16.11.2019: the compiler may exclude code based on constant if statements (display.epd2.panel == constant),
//                  therefore bitmaps may get optimized out by the linker

// comment out unused bitmaps to reduce code space used
//#include "bitmaps/Bitmaps80x128.h"  // 1.02" b/w
//#include "bitmaps/Bitmaps152x152.h" // 1.54" b/w
//#include "bitmaps/Bitmaps200x200.h" // 1.54" b/w
//#include "bitmaps/Bitmaps104x212.h" // 2.13" b/w flexible GDEW0213I5F
//#include "bitmaps/Bitmaps128x250.h" // 2.13" b/w
//#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w
//#include "bitmaps/Bitmaps168x384.h" // 2.9"  b/w
//#include "bitmaps/Bitmaps152x296.h" // 2.6"  b/w
//#include "bitmaps/Bitmaps176x264.h" // 2.7"  b/w
//#include "bitmaps/Bitmaps240x360.h" // 3.1" b/w
//#include "bitmaps/Bitmaps240x416.h" // 3.71"  b/w
//#include "bitmaps/Bitmaps400x300.h" // 4.2"  b/w
//#include "bitmaps/Bitmaps648x480.h" // 5.38"  b/w
//#include "bitmaps/Bitmaps640x384.h" // 7.5"  b/w
//#include "bitmaps/Bitmaps800x480.h" // 7.5"  b/w
//#include "bitmaps/Bitmaps960x640.h" // 10.2"  b/w
//#include "bitmaps/Bitmaps960x680.h" // 13.3"  b/w
// 3-color
//#include "bitmaps/Bitmaps3c200x200.h" // 1.54" b/w/r
//#include "bitmaps/Bitmaps3c104x212.h" // 2.13" b/w/r
//#include "bitmaps/Bitmaps3c128x250.h" // 2.13" b/w/r
//#include "bitmaps/Bitmaps3c128x296.h" // 2.9"  b/w/r
//#include "bitmaps/Bitmaps3c152x296.h" // 2.66" b/w/r
//#include "bitmaps/Bitmaps3c176x264.h" // 2.7"  b/w/r
//#include "bitmaps/Bitmaps3c400x300.h" // 4.2"  b/w/r
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#include "bitmaps/Bitmaps3c648x480.h" // 5.83" b/w/r
//#include "bitmaps/Bitmaps3c800x480.h" // 7.5"  b/w/r
//#include "bitmaps/Bitmaps3c880x528.h" // 7.5"  b/w/r
//#include "bitmaps/Bitmaps3c960x640.h" // 11.6" b/w/r
//#include "bitmaps/Bitmaps3c960x680.h" // 13.3" b/w/r
//#include "bitmaps/WS_Bitmaps800x600.h" // 6.0"  grey
// 4-color
//#include "bitmaps/Bitmaps4c128x250.h" // 2.13" 4-color
//#include "bitmaps/Bitmaps4c184x360.h" // 2.66" 4-color
//#include "bitmaps/Bitmaps4c168x384.h" // 2.9" 4-color
//#include "bitmaps/WS_Bitmaps4c168x168.h" // 4.37" 4-color
//#include "bitmaps/WS_Bitmaps4c168x400.h" // 3.00" 4-color
//#include "bitmaps/Bitmaps4c400x300.h" // 4.2" 4-color
//#include "bitmaps/Bitmaps4c960x640.h" // 11.6" 4-color
// 7-color
//#include "bitmaps/Bitmaps7c600x448.h" // 5.65" 7-color
//#include "bitmaps/WS_Bitmaps7c192x143.h" // 5.65" 7-color
//#include "bitmaps/WS_Bitmaps7c300x180.h" // 7.3" 7-color
#endif
#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
//#include "bitmaps/Bitmaps1304x984.h" // 12.48" b/w
//#include "bitmaps/Bitmaps1360x480.h" // 10.85" b/w
//#include "bitmaps/Bitmaps3c1304x984.h" // 12.48" b/w/r
//#include "bitmaps/Bitmaps7c800x480.h" // 7.3" 7-color
#endif

#else

// select only one to fit in code space
//#include "bitmaps/Bitmaps80x128.h"  // 1.02" b/w
//#include "bitmaps/Bitmaps200x200.h" // 1.54" b/w
//#include "bitmaps/Bitmaps104x212.h" // 2.13" b/w flexible GDEW0213I5F
//#include "bitmaps/Bitmaps128x250.h" // 2.13" b/w
//#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w
//#include "bitmaps/Bitmaps168x384.h" // 2.9"  b/w
//#include "bitmaps/Bitmaps176x264.h" // 2.7"  b/w
////#include "bitmaps/Bitmaps400x300.h" // 4.2"  b/w // not enough code space
////#include "bitmaps/Bitmaps640x384.h" // 7.5"  b/w // not enough code space
////#include "bitmaps/Bitmaps800x480.h" // 7.5"  b/w // not enough code space
// 3-color
//#include "bitmaps/Bitmaps3c200x200.h" // 1.54" b/w/r
//#include "bitmaps/Bitmaps3c104x212.h" // 2.13" b/w/r
//#include "bitmaps/Bitmaps3c128x250.h" // 2.13" b/w/r
//#include "bitmaps/Bitmaps3c128x296.h" // 2.9"  b/w/r
//#include "bitmaps/Bitmaps3c176x264.h" // 2.7"  b/w/r
////#include "bitmaps/Bitmaps3c400x300.h" // 4.2"  b/w/r // not enough code space

#endif

#if defined(ARDUINO_ARCH_RP2040) && (defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_RASPBERRY_PI_PICO_W))
#if defined(__MBED__)
// SPI pins used by GoodDisplay DESPI-PICO. note: steals standard I2C pins PIN_WIRE_SDA (6), PIN_WIRE_SCL (7)
// uncomment next line for use with GoodDisplay DESPI-PICO. // MbedSPI(int miso, int mosi, int sck);
arduino::MbedSPI SPIn(4, 7, 6); // need be valid pins for same SPI channel, else fails blinking 4 long 4 short
// uncomment next line for use with my proto board. // MbedSPI(int miso, int mosi, int sck);
//arduino::MbedSPI SPIn(4, 3, 2); // need be valid pins for same SPI channel, else fails blinking 4 long 4 short
// uncomment next line for use with Waveshare Pico-ePaper-2.9. // MbedSPI(int miso, int mosi, int sck);
// note: doesn't work with Waveshare PhotoPainter, conflict on pin 12. use philhower package instead.
//arduino::MbedSPI SPIn(12, 11, 10); // need be valid pins for same SPI channel, else fails blinking 4 long 4 short
#else // package https://github.com/earlephilhower/arduino-pico
// SPIClassRP2040(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx);
// uncomment next line for use with my proto board.
//SPIClassRP2040 SPIn(spi0, 4, 5, 2, 3); // need be valid pins for same SPI channel, else fails blinking 4 long 4 short
// uncomment next line for use with Waveshare Pico-ePaper-2.9 or Waveshare PhotoPainter module
SPIClassRP2040 SPIn(spi1, 12, 13, 10, 11); // need be valid pins for same SPI channel, else fails blinking 4 long 4 short
#endif
#endif

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif

// Change clock speed to default value.
// This is used before drawing e-paper for optimal performance.
void SystemClock_Config_org(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }
}

// Change clock speed for power saving.
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

// Return day of the week for a given date
int dayOfWeek(int year, int month, int day)
{
    if (month < 3)
    {
        year--;
        month += 12;
    }
    int  i =(year + year / 4 - year / 100 + year / 400 + (13 * month + 8) / 5 + day) % 7;
    // Sun = 0 Sat = 6
    return (i);
}

// Return number of days in a month
int numberOfDaysInMonth(int year, int month)
{
    int numberOfDaysInMonthArray[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0)
    {
        numberOfDaysInMonthArray[1] = 29;
    }
    return numberOfDaysInMonthArray[month - 1];
}

// Check if the date is public holiday
bool checkPublicHoliday(int year, int month,int day){
  // check if public holiday
  bool ph= false;
  for ( int i = 0; i < sizeof(holidaylist); i++ )
  {
    if(holidaylist[i] == (year*10000+month*100+day) )
      {
        ph=true;
      }
  }

  return ph;
}

// Draw main YEAR/Month title
void drawMainTitle(int year,int month, int size, const GFXfont* f,int x,int y)
{
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(x,y+size*1.5);
  display.print(year);
  display.setCursor(x + size * 4,y+size*1.5);
  display.print(" /");
  display.setCursor(x + size * 5,y+size*1.5);
  display.print(month);
}

// Draw small YEAR/Month title
void drawSmallTitle(int year,int month, int size, const GFXfont* f,int x,int y)
{
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(x,y+size*1.5);
  display.print(year);
  display.setCursor(x + size * 4,y+size*1.5);
  display.print(" /");
  display.setCursor(x + size * 5,y+size*1.5);
  display.print(month);
}

// Draw main month calendar
void drawMainCalendar(int year,int month, const GFXfont* fweekday,const GFXfont* fdays,int x,int y,int w, int h)
{

  int i,dow,ndm;
  // Draw Weekday bar & ruled line
  display.setFont(fweekday);
  display.setCursor( x , y);

  display.fillRect( x,y, w,h, GxEPD_RED);
  display.fillRect( x+w,y, w*6,h, GxEPD_BLACK);
  for (i=0; i <= 7; i++){
    display.drawLine(x+i*w,y,x+i*w,y+h*6, GxEPD_BLACK);
  }
  for (i=0; i <= 4; i++){
    display.drawLine(x,y+2*h+i*h,x+w*7,y+2*h+i*h, GxEPD_BLACK);
  }

  // Check weekday of this month
  dow = dayOfWeek(year,month,1);

  // Check number of days in this month
  ndm = numberOfDaysInMonth(year,month);

  // in case of 6 week in a month, draw additional lines
  if ((dow + ndm) == 36 ) {
    display.drawLine(x,y+2*h+5*h,x+w*1,y+2*h+5*h, GxEPD_BLACK);
    display.drawLine(x+0*w,y,x+0*w,y+h*7, GxEPD_BLACK);
    display.drawLine(x+1*w,y,x+1*w,y+h*7, GxEPD_BLACK);
  } else if ((dow + ndm) == 37 ) {
    display.drawLine(x,y+2*h+5*h,x+w*2,y+2*h+5*h, GxEPD_BLACK);
    display.drawLine(x+0*w,y,x+0*w,y+h*7, GxEPD_BLACK);
    display.drawLine(x+1*w,y,x+1*w,y+h*7, GxEPD_BLACK);
    display.drawLine(x+2*w,y,x+2*w,y+h*7, GxEPD_BLACK);
  }

  display.setTextColor(GxEPD_WHITE);
  display.setFont(fweekday);
  display.setCursor( x+w/4,y+h*2/3 );
  display.println("Sun");
  display.setCursor( x+w/4+w,y+h*2/3 );
  display.println("Mon");
  display.setCursor( x+w/4+w*2,y+h*2/3 );
  display.println("Tue");
  display.setCursor( x+w/4+w*3,y+h*2/3 );
  display.println("Wed");
  display.setCursor( x+w/4+w*4,y+h*2/3 );
  display.println("Thr");
  display.setCursor( x+w/4+w*5,y+h*2/3 );
  display.println("Fri");
  display.setCursor( x+w/4+w*6,y+h*2/3 );
  display.println("Sat");

  // Start draw days
  // *if today found, revent the string
  display.setFont(fdays);
  int line = 1;
  for (i=1;i<=ndm;i++){
    if (i == rtc.getDay())
    {
      display.setTextColor(GxEPD_WHITE);
      if ((dow == 0) or checkPublicHoliday(year,month,i))
      {
        display.fillRect(x + dow * w , y + line * h , w, h, GxEPD_RED);
      } else {
        display.fillRect(x + dow * w , y + line * h , w, h, GxEPD_BLACK);
      }
    } else if ((dow == 0) or checkPublicHoliday(year,month,i)) {
      display.setTextColor(GxEPD_RED);
    } else {
      display.setTextColor(GxEPD_BLACK);
    }
    display.setCursor( x + dow * w + w/3,  y + line * h + h*4/5);
    if(i <=9) {display.print(" ");}
    display.print(i);
    if (dow == 6) { line++; }
    dow++;
    dow = dow % 7;
  }
}

// Draw small month calendar
void drawSmallCalendar(int year,int month, const GFXfont* fdays,int x,int y,int w, int h)
{

  int i,dow,ndm;
  // Check weekday of this month
  dow = dayOfWeek(year,month,1);

  // Check number of days in this month
  ndm = numberOfDaysInMonth(year,month);

  // Start draw days
  // *if today found, revent the string
  display.setFont(fdays);
  int line = 0;
  for (i=1;i<=ndm;i++){
    if ((dow == 0) or checkPublicHoliday(year,month,i) ){
      display.setTextColor(GxEPD_RED);
    } else {
      display.setTextColor(GxEPD_BLACK);
    }
    display.setCursor( x + dow * w + w/3,  y + line * h + h*4/5);
    if(i <=9) {display.print(" ");}
    display.print(i);
    if (dow == 6) { line++; }
    dow++;
    dow = dow % 7;
  }

}

// Get battery voltage and draw it on the screen
void drawBatteryvoltage(const GFXfont* fdays,int x,int y)
{
  display.setFont(fdays);
  display.setCursor( x ,  y );

	uint16_t vrefint = analogRead(AVREF);
	//Serial.print("Vref int (1.21v):");
	//Serial.print(vrefint);
	//Serial.println();

	uint16_t vtemp = analogRead(PA6);
  float vbat;
	//Serial.print("PA6 value:");
	//Serial.println(vtemp);
  // 小数点2桁で丸める
  vbat = round(100*1.21*vtemp/vrefint);
  vbat = vbat/100;
  //Serial.print(vbat);
	//Serial.println();
  display.print("BAT:");
  display.print(vbat);
  display.print(" V");
}

// Draw 1 month calendar with previous/next month.
void drawcalendar()
{
  int y,m,y1,m1,y2,m2;
  y = rtc.getYear() + 2000;
  m = rtc.getMonth();
  if (m == 1) {
    y1 = y -1 ;
    m1 = 12;
    y2 = y;
    m2 = m + 1;
    } else if (m == 12) {
      y1 = y ;
      m1 = 11;
      y2 = y+1;
      m2 = 1;
    } else {
      y1 = y;
      m1 = m -1;
      y2 = y;
      m2 = m +1;
    }
  display.setFullWindow();
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    // FreeMonoBold24pt7b   FreeSerifBold24pt7b FreeSansBold24pt7b
    drawMainTitle(y,m,24,&FreeSansBold24pt7b,20,20);
    drawMainCalendar(y,m,&FreeSansBold12pt7b,&FreeSansBold18pt7b,0,144,92,48);
    drawSmallTitle(y1,m1,9,&FreeSansBold9pt7b,276,5);
    drawSmallCalendar(y1,m1,&FreeSansBold9pt7b,276,25,25,20);
    drawSmallTitle(y2,m2,9,&FreeSansBold9pt7b,460,5);
    drawSmallCalendar(y2,m2,&FreeSansBold9pt7b,460,25,25,20);
    drawBatteryvoltage(&FreeSansBold9pt7b,552,460);
  }
  while (display.nextPage());
}

// THis is not used.
void drawClock(int x, int y, int size,const GFXfont* f,int hour, int minute)
{
  //display.setRotation(r);
  display.setPartialWindow(x,y, x+size*5,y+size);
  display.fillScreen(GxEPD_WHITE);
  display.setFont(f);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {

    display.setCursor( x, y+size*1.5);
    if (hour < 10) {display.print(" ");}
    display.print(hour);
    display.print(":");
    if (minute < 10) {display.print(" ");}
    display.print(minute);
  }
  while (display.nextPage());
}

// Draw 9 month calendar
void draw9monthcalendar()
{
  int d,i,j,y,m,yy[3][3],mm[3][3];
  y = rtc.getYear() + 2000;
  m = rtc.getMonth();
  // Caclulate month/year for 9 months.
  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      // d will be -4 to 4 indicates offset from center month
      d = (i-1)*3 + j -1;
      if ((m + d) < 1) {
        yy[i][j] = y - 1;
        }
        else if ((m + d) > 12)
        {
          yy[i][j] = y + 1;
        }
        else {
          yy[i][j] = y;
        }
      mm[i][j] = ( m - 1 + d + 12 ) % 12 +1;
      Serial.println(yy[i][j]);
      Serial.println(mm[i][j]);
    }
  }

  display.setFullWindow();
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    for(i=0;i<3;i++){
      for(j=0;j<3;j++){
        drawSmallTitle(yy[i][j],mm[i][j],9,&FreeSansBold9pt7b,200*j+20,15+160*i);
        drawSmallCalendar(yy[i][j],mm[i][j],&FreeSansBold9pt7b,200*j+20,40+160*i,25,20);
      }
    }
  }
  while (display.nextPage());

}

// pin change interrupt service routine for JJY
void isr_routine() {
  jjy.jjy_receive();
}

// 10 msec interrupt service routine for JJY
void ticktock() {
  jjy.delta_tick();
}

// ISR for pin button push. Start JJY receive.
void repetitionsIncrease() {

  // Wait to prevent chataling.
  delay(100);

  // Start timer
  MyTim->resume();
  jjy.power(true);

  // Set Receiveing flag.
  jjyreceiving = true;
  // Rrcord recieving start time
  receievestarttime = millis();
}

// ISR for pin2 button push. Change calendar mode.
void modechange() {

    // Wait to prevent chataling.
    delay(100);

    if(calmode == 0){
      calmode = 1;
    } else {
      calmode = 0;
    }

}

// Print current date and time to serial.
void printdatetime(){

  time_t now = jjy.get_time();
  time_t lastreceived = jjy.getTime();
  tm tm_info;

  localtime_r(&now, &tm_info);
  const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  String str0 = String(tm_info.tm_year + 1900);
  String str1 = String(tm_info.tm_mon + 1);
  String str2 = String(tm_info.tm_mday);
  String str3 = String(tm_info.tm_hour);
  char buf1[3];
  sprintf(buf1, "%02d", tm_info.tm_min);
  char buf2[3];
  sprintf(buf2, "%02d", tm_info.tm_sec);
  String strm = String(days[tm_info.tm_wday]);
  Serial.print(str0+"/"+str1+"/"+str2+" "+strm+" "+str3+":"+buf1+" "+buf2);  // Print current date time.
  Serial.print(" Last received:");
  String str = String(ctime(&lastreceived));
  Serial.println(str);  // Print last received time

  // 受信にかかった時間をmsで表示する
  Serial.print("Elapsed time = ");
  Serial.println(millis() - receievestarttime);

}


void setup()
{

  // Start serial monitor.
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // to control power of e-Paper
  pinMode(EPDPWR,OUTPUT);
  digitalWrite(EPDPWR,LOW);

  delay(100);
  #if defined(ARDUINO_ARCH_RP2040) && (defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_RASPBERRY_PI_PICO_W))
    // uncomment next line for use with GoodDisplay DESPI-PICO or my proto board, or Waveshare RPi boards
    display.epd2.selectSPI(SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    // uncomment next 2 lines to allow recovery from configuration failures
    pinMode(15, INPUT_PULLUP); // safety pin
    while (!digitalRead(15)) delay(100); // check safety pin for fail recovery
    // recovery can be done also by holding BOOTSEL during power-up.
    // uncomment next line for Waveshare PhotoPainter module
    pinMode(16, OUTPUT); digitalWrite(16, HIGH); // power to the paper
  #endif
  #if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
    hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  #elif (defined(ARDUINO_ARCH_ESP32) && defined(ARDUINO_LOLIN_S2_MINI))
    // SPI.begin(sck, miso, mosi, ss); // preset for remapped pins
    SPI.begin(18, -1, 16, 33); // my LOLIN ESP32 S2 mini connection
  #endif

  #if defined(ESP32) && defined(_GxBitmaps1304x984_H_)
    drawBitmaps1304x984();
    display.powerOff();
  #endif

  Serial.println("setup done");

  // Initialize RTC
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format
  // Check it RTC time is not set.
  if (!rtc.isTimeSet()) {
    Serial.printf("RTC time not set\n Set it.\n");
    // RTC time is not set, maybe powerloss, so Set the date and time
    rtc.setHours(hours);
    rtc.setMinutes(minutes);
    rtc.setSeconds(seconds);
    rtc.setWeekDay(weekDay);
    rtc.setDay(day);
    rtc.setMonth(month);
    rtc.setYear(year);
  }

  // Configure low power
  LowPower.begin();


  // Initialize User button pin.
  pinMode(pin, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);

  // Initialize LED pin for JJY status
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);
  digitalWrite( GLED, LOW );
  digitalWrite( RLED, LOW );

  // Attache ISR, these are for user button event.
  LowPower.attachInterruptWakeup(pin, repetitionsIncrease, RISING, DEEP_SLEEP_MODE);
  LowPower.attachInterruptWakeup(pin2, modechange, RISING, DEEP_SLEEP_MODE);

  // Initialize timer for JJY.
  MyTim->setOverflow(100, HERTZ_FORMAT); // 100 Hz
  MyTim->attachInterrupt(ticktock);
  MyTim->pause();

  // JJY Library initialize
  jjy.begin(); // Start JJY Receive
  jjy.freq(40); // Carrier frequency setting. Default:40

  Serial.println("JJY Initialized.");

  // Turn off JJY module power to save power.
  jjy.power(false);

  // DATA pin signal change edge detection. (Mandatory)
  // For JJY data input.
  attachInterrupt(digitalPinToInterrupt(DATA), isr_routine, CHANGE);

  // ADC initialize to check vcc voltge
  pinMode(PA6, INPUT_ANALOG);


}

void loop()
{
  // Check if the date is changed.
  if ( (curyear != rtc.getYear()) or (curmonth != rtc.getMonth()) or (curday != rtc.getDay()) )
  {
    // if changed, draw calendar.
    curyear = rtc.getYear();
    curmonth = rtc.getMonth();
    curday = rtc.getDay();
    Serial.println("Date change detected, draw Calendar");
    // power on e-paper
    digitalWrite(EPDPWR,HIGH);
    delay(100);

    // Before changing clock, serial needs to be disabled.
    Serial.end();
    // Change clock speed to default value.
    SystemClock_Config_org();
    // Serial needs to be initialized again.
    Serial.begin(115200);

    display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    drawcalendar();
    display.hibernate();

    // power off e-paper
    digitalWrite(EPDPWR,LOW);

    delay(100);

    // to reduce the power consumption, set pins to input mode.
    pinMode(PA4,INPUT);
    pinMode(PA5,INPUT);
    pinMode(PA6,INPUT);
    pinMode(PA7,INPUT);
    pinMode(PA2,INPUT);
    pinMode(PB0,INPUT);
    pinMode(PB1,INPUT);

    // Before changing clock, serial needs to be disabled.
    Serial.end();
    // Change clock speed for power saving.
    SystemClock_Config();
    // Serial needs to be initialized again.
    Serial.begin(115200);
  }

  // Check if the calendar mode is changed.
  if (currentcalmode != calmode) {
    currentcalmode = calmode;

    Serial.println("Mode change detected, draw Calendar");
    // power on e-paper
    digitalWrite(EPDPWR,HIGH);
    delay(100);

    Serial.end();
    SystemClock_Config_org();
    Serial.begin(115200);

    display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    if (calmode == 0) {
      drawcalendar();
    } else {
      draw9monthcalendar();
    }
    display.hibernate();

    // power off e-paper
    digitalWrite(EPDPWR,LOW);

    delay(100);

    // to reduce the power consumption, set pins to input mode.
    pinMode(PA4,INPUT);
    pinMode(PA5,INPUT);
    pinMode(PA6,INPUT);
    pinMode(PA7,INPUT);
    pinMode(PA2,INPUT);
    pinMode(PB0,INPUT);
    pinMode(PB1,INPUT);

    Serial.end();
    SystemClock_Config();
    Serial.begin(115200);
  }

  // To save power, will enter into deep sleep mode for 10s.
  LowPower.deepSleep(10000);
  //delay(1000);

  // If JJY flag is set, then start receiving JJY signal.
  if (jjyreceiving) {
    time_t lastreceived = jjy.getTime();
    unsigned long cycletime = millis();
    int loopcount = 0;
    // Wait until receive success or time out
    while ((lastreceived == -1) and ((millis() - receievestarttime) < JJYTIMEOUT)) {
      // if it is -1, meaning JJY receving.

      // Show JJY receiveing quality as LED.
      // Green  >= 60%
      // Yeallow 30 - 60 %
      // Red   <=30%

      if (millis() - cycletime > 1000) {

        String str0 = "Receiving quality:";
        String str1 = String(jjy.quality);
        Serial.println(str0 + str1);
        str0 = "Count:";
        str1 = String(loopcount);
        Serial.println(str0 + str1);
        loopcount++;

        cycletime = millis();
        if (jjy.quality < 31) {
          digitalWrite( GLED, LOW );
          digitalWrite( RLED, HIGH );
        } else if (jjy.quality < 61) {
          digitalWrite( GLED, HIGH );
          digitalWrite( RLED, HIGH );
        } else {
          digitalWrite( GLED, HIGH );
          digitalWrite( RLED, LOW );
        }
        delay(100);
        digitalWrite( GLED, LOW );
        digitalWrite( RLED, LOW );
      }

      lastreceived = jjy.getTime();

    }
    if (lastreceived != -1){
      // If receive successful, set date time to RTC.
      time_t now = jjy.get_time();
      tm tm_info;
      localtime_r(&now, &tm_info);

      rtc.setTime(tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
      int wdayforrtc;
      if (tm_info.tm_wday == 0) { wdayforrtc = 7; }
      else { wdayforrtc = tm_info.tm_wday; }
      rtc.setDate(wdayforrtc, tm_info.tm_mday, tm_info.tm_mon + 1, tm_info.tm_year + 1900 - 2000);

      // Print date/time to serial
      printdatetime();

      // LED flash Green to show the success.
      digitalWrite( GLED, HIGH );
      delay(100);
      digitalWrite( GLED, LOW );
      delay(500);
      digitalWrite( GLED, HIGH );
      delay(100);
      digitalWrite( GLED, LOW );
      delay(500);
      digitalWrite( GLED, HIGH );
      delay(100);
      digitalWrite( GLED, LOW );
      delay(500);
    } else {
      // LED flash Red to show the failure.
      Serial.println("Timed Out");
      digitalWrite( RLED, HIGH );
      delay(100);
      digitalWrite( RLED, LOW );
      delay(500);
      digitalWrite( RLED, HIGH );
      delay(100);
      digitalWrite( RLED, LOW );
      delay(500);
      digitalWrite( RLED, HIGH );
      delay(100);
      digitalWrite( RLED, LOW );
      delay(500);

    }

    // Stop the timer
    MyTim->pause();
    // Clear receiving flag.
    jjyreceiving = false;
    // Power off JJY module.
    jjy.power(false);
  }

}
