/* --------------------------------------------------------------------------
  FILE        : device.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Sandeep Paulraj
  DESC        : This file provides low-level, device-specific init functions
                for use the DM36x device.
-------------------------------------------------------------------------- */ 


/************************************************************
* Include Files                                             *
************************************************************/

// General type include
#include "tistdtypes.h"

// Utility functions
#include "util.h"

// This module's header file
#include "device.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/

extern __FAR__ Uint32 EXTERNAL_RAM_START;


/************************************************************
* Local Macro Declarations                                  *
************************************************************/

#define GPINT_GPEN    (VUint32 *)(0x01C21C08)    // WDT special function
#define GPTDAT_GPDIR  (VUint32 *)(0x01C21C0c)    // WDT special function

#define TMPBUF          (unsigned int *)(0x17ff8)
#define TMPSTATUS		(unsigned int *)(0x17ff0)
#define FLAG_PORRST		0x00000001
#define FLAG_WDTRST		0x00000002
#define FLAG_FLGON		0x00000004
#define FLAG_FLGOFF		0x0000001


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

#ifndef SKIP_LOW_LEVEL_INIT
  static void LOCAL_porReset();
  static void LOCAL_vpssSyncReset();
#endif


/************************************************************
* Local Variable Definitions                                *
\***********************************************************/
#if defined(DM36x_ARM216_DDR173_OSC19P2)
  // 19.2MHz input
  // PLL1 VCO=432 MHz
  static const Uint32 PLL1_Mult     = 45;
  static const Uint32 PLL1_PreDiv   = 3;
  static const Uint32 PLL1_Div1     = 0;    // ARM          = 432/2   = 216 MHz
  static const Uint32 PLL1_Div2     = 1;    // ARM          = 432/2   = 216 MHz
  static const Uint32 PLL1_Div3     = 1;    // MJCP/HDVICP  = 432/2   = 216 MHz
  static const Uint32 PLL1_Div4     = 3;    // EDMA         = 432/4   = 108 MHz
  static const Uint32 PLL1_Div5     = 3;    // VPSS         = 432/2   = 216 MHz
  static const Uint32 PLL1_Div6     = 15;   // VENC         = 432/16  = 27 MHz
  static const Uint32 PLL1_Div7     = 0;    //              = 432/1   = 432 MHz
  static const Uint32 PLL1_Div8     = 3;    // SDMMC        = 432/4   = 108 MHz
  static const Uint32 PLL1_Div9     = 1;    // CLKOUT       = 432/2   = 216 MHz  
  
  // PLL2 VCO=344.064 MHz
  static const Uint32 PLL2_Mult     = 224;
  static const Uint32 PLL2_PreDiv   = 24;
  static const Uint32 PLL2_Div1     = 0;    //              = 344.064 = 344.064 MHz
  static const Uint32 PLL2_Div2     = 0;    //              = 344.064 = 344.064 MHz
  static const Uint32 PLL2_Div3     = 0;    // DDR          = 344.064 = 344.064 MHz ==> DDR172.032
  static const Uint32 PLL2_Div4     = 6;    // Voice        = 594/29  = 20.4872 MHz
  static const Uint32 PLL2_Div5     = 0;    // VideoHD      = 594/8   = 74.25 MHz

  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x0BFF05FC;
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  static const Uint32 DDR_SDBCR     = 0x08534832;
  static const Uint32 DDR_SDTIMR    = 0x2C923251;
  static const Uint32 DDR_SDTIMR2   = 0x4217C722;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x00000546;
#elif defined(DM36x_ARM270_DDR216_OSC27)
  // PLL1 VCO=432 MHz
  static const Uint32 PLL1_Mult     = 9;
  static const Uint32 PLL1_PreDiv   = 0;
  static const Uint32 PLL1_Div1     = 0;    //              = 432/2   = 216 MHz
  static const Uint32 PLL1_Div2     = 1;    //              = 432/2   = 216 MHz
  static const Uint32 PLL1_Div3     = 1;    // MJCP/HDVICP  = 432/2   = 216 MHz
  static const Uint32 PLL1_Div4     = 3;    // EDMA         = 432/4   = 108 MHz
  static const Uint32 PLL1_Div5     = 3;    // VPSS         = 432/2   = 216 MHz
  static const Uint32 PLL1_Div6     = 15;   // VENC         = 432/16  = 27 MHz
  static const Uint32 PLL1_Div7     = 0;    // DDR          = 432/1   = 432 MHz ==> DDR216
  static const Uint32 PLL1_Div8     = 3;    // SDMMC        = 432/4   = 108 MHz
  static const Uint32 PLL1_Div9     = 1;    // CLKOUT       = 432/2   = 216 MHz  
  
  // PLL2 VCO=270 MHz
  static const Uint32 PLL2_Mult     = 45;
  static const Uint32 PLL2_PreDiv   = 7;
  static const Uint32 PLL2_Div1     = 0;    // ARM          = 270/1   = 270 MHz
  static const Uint32 PLL2_Div2     = 0;    // ARM          = 270/1   = 270 MHz
  static const Uint32 PLL2_Div3     = 0;    //              = 270/1   = 270 MHz
  static const Uint32 PLL2_Div4     = 5;    // Voice        = 270/6   = 45 MHz
  static const Uint32 PLL2_Div5     = 9;    // VENC         = 270/10  = 27 MHz
  
  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x243F04FC;
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  static const Uint32 DDR_SDBCR     = 0x08534832;
  static const Uint32 DDR_SDTIMR    = 0x369342D1;
  static const Uint32 DDR_SDTIMR2   = 0x421DC702;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x00000695;  
#elif defined(DM36x_ARM297_DDR243_OSC24)
  // 24MHz input
  // PLL1 VCO=486 MHz
  static const Uint32 PLL1_Mult     = 81;
  static const Uint32 PLL1_PreDiv   = 7;
  static const Uint32 PLL1_Div1     = 0;    
  static const Uint32 PLL1_Div2     = 1;    
  static const Uint32 PLL1_Div3     = 1;    // MJCP/HDVICP  = 486/2   = 243 MHz
  static const Uint32 PLL1_Div4     = 3;    // EDMA         = 486/4   = 121.5 MHz
  static const Uint32 PLL1_Div5     = 1;    // VPSS         = 486/2   = 243 MHz
  static const Uint32 PLL1_Div6     = 17;   // VENC         = 486/18  = 27 MHz
  static const Uint32 PLL1_Div7     = 0;    // DDR          = 486/1   = 486 MHz ==> DDR243
  static const Uint32 PLL1_Div8     = 3;    // SDMMC        = 486/4   = 121.5 MHz
  static const Uint32 PLL1_Div9     = 1;    // CLKOUT       = 486/2   = 243 MHz  
  
  // PLL2 VCO=594 MHz
  static const Uint32 PLL2_Mult     = 99;
  static const Uint32 PLL2_PreDiv   = 7;
  static const Uint32 PLL2_Div1     = 0;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div2     = 1;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div3     = 0;    //              = 594/1   = 594 MHz
  static const Uint32 PLL2_Div4     = 28;   // Voice        = 594/29  = 20.4872 MHz
  static const Uint32 PLL2_Div5     = 7;    // VideoHD      = 594/8   = 74.25 MHz
  
  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x243F04FC;  
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  static const Uint32 DDR_SDBCR     = 0x08534832;
  static const Uint32 DDR_SDTIMR    = 0x3C934B51;
  static const Uint32 DDR_SDTIMR2   = 0x4221C702;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x00000768;
#elif defined(DM36x_ARM297_DDR270_OSC24)
  // 24MHz input
  // PLL1 VCO=486 MHz
  static const Uint32 PLL1_Mult     = 45;
  static const Uint32 PLL1_PreDiv   = 3;
  static const Uint32 PLL1_Div1     = 0;    
  static const Uint32 PLL1_Div2     = 1;    
  static const Uint32 PLL1_Div3     = 1;    // MJCP/HDVICP  = 540/2   = 270 MHz
  static const Uint32 PLL1_Div4     = 3;    // EDMA         = 540/4   = 135 MHz
  static const Uint32 PLL1_Div5     = 1;    // VPSS         = 540/2   = 270 MHz
  static const Uint32 PLL1_Div6     = 19;   // VENC         = 540/18  = 27 MHz
  static const Uint32 PLL1_Div7     = 0;    // DDR          = 540/1   = 540 MHz ==> DDR270
  static const Uint32 PLL1_Div8     = 3;    // SDMMC        = 540/4   = 135 MHz
  static const Uint32 PLL1_Div9     = 1;    // CLKOUT       = 540/2   = 270 MHz  
  
  // PLL2 VCO=594 MHz
  static const Uint32 PLL2_Mult     = 99;
  static const Uint32 PLL2_PreDiv   = 7;
  static const Uint32 PLL2_Div1     = 0;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div2     = 1;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div3     = 0;    //              = 594/1   = 594 MHz
  static const Uint32 PLL2_Div4     = 28;   // Voice        = 594/29  = 20.4872 MHz
  static const Uint32 PLL2_Div5     = 7;    // VideoHD      = 594/8   = 74.25 MHz  
  
  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x243F04FC;
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C6;
  static const Uint32 DDR_SDBCR     = 0x00534A32;
  static const Uint32 DDR_SDTIMR    = 0x45245392;
  static const Uint32 DDR_SDTIMR2   = 0x4225C742;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x0000083A;    
#elif defined(DM36x_ARM297_DDR277_OSC27)
  // 27MHz input
  // PLL1 VCO=594 MHz
  static const Uint32 PLL1_Mult     = 44;
  static const Uint32 PLL1_PreDiv   = 3;
  static const Uint32 PLL1_Div1     = 0;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL1_Div2     = 1;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL1_Div3     = 1;    // MJCP/HDVICP  = 594/2   = 297 MHz
  static const Uint32 PLL1_Div4     = 3;    // EDMA         = 594/4   = 148.5 MHz
  static const Uint32 PLL1_Div5     = 3;    // VPSS         = 594/4   = 148.5 MHz
  static const Uint32 PLL1_Div6     = 21;   // VENC         = 594/18  = 27 MHz
  static const Uint32 PLL1_Div7     = 0;    //              = 594/1   = 594 MHz
  static const Uint32 PLL1_Div8     = 3;    // SDMMC        = 594/4   = 148.5 MHz
  static const Uint32 PLL1_Div9     = 1;    // CLKOUT       = 594/2   = 294 MHz  
  
  // PLL2 VCO=552.96 MHz
  static const Uint32 PLL2_Mult     = 256;
  static const Uint32 PLL2_PreDiv   = 24;
  static const Uint32 PLL2_Div1     = 0;    //              = 552.96/1   = 552.96 MHz   
  static const Uint32 PLL2_Div2     = 0;    //              = 552.96/1   = 552.96 MHz   
  static const Uint32 PLL2_Div3     = 0;    // DDR          = 552.96/2   = 552.96 MHz ==> DDR276.48
  static const Uint32 PLL2_Div4     = 8;    //              = 552.96/9   = 61.44 MHz
  static const Uint32 PLL2_Div5     = 0;    //              = 552.96/1   = 552.96 MHz
  
  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x0BFF077C;  
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  static const Uint32 DDR_SDBCR     = 0x08534832;
  static const Uint32 DDR_SDTIMR    = 0x4ADC5C1A;
  static const Uint32 DDR_SDTIMR2   = 0x4ADC5C1A;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x0000090D; 

#elif defined(DM36x_ARM445_DDR351_OSC24)
  // 27MHz input
  // PLL1 VCO=594 MHz
  static const Uint32 PLL1_Mult     = 0x75;
  static const Uint32 PLL1_PreDiv   = 0x7;
  static const Uint32 PLL1_Div1     = 0x1b;
  static const Uint32 PLL1_Div2     = 0x1;
  static const Uint32 PLL1_Div3     = 0x1;
  static const Uint32 PLL1_Div4     = 0x3;
  static const Uint32 PLL1_Div5     = 0x1;
  static const Uint32 PLL1_Div6     = 0x19;
  static const Uint32 PLL1_Div7     = 0x0;
  static const Uint32 PLL1_Div8     = 0x6;
  static const Uint32 PLL1_Div9     = 0x1b;
  
  // PLL2 VCO=552.96 MHz
  static const Uint32 PLL2_Mult     = 0xe8;
  static const Uint32 PLL2_PreDiv   = 0x18;
  static const Uint32 PLL2_Div1     = 0x11;
  static const Uint32 PLL2_Div2     = 0x0;
  static const Uint32 PLL2_Div3     = 0x1;
  static const Uint32 PLL2_Div4     = 0x5;
  static const Uint32 PLL2_Div5     = 0x16;
  
  // Clock Control (mux settings)
  //static const Uint32 ClockControl  = 0x0BFF077C;  
  static const Uint32 ClockControl  = 0x243F04FC;
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  //static const Uint32 DDR_SDBCR     = 0x08534832;
  static const Uint32 DDR_SDBCR     = 0x08534A32;
  //static const Uint32 DDR_SDTIMR    = 0x4ADC5C1A;
  static const Uint32 DDR_SDTIMR    = 0x57256C9A;
  //static const Uint32 DDR_SDTIMR2   = 0x4ADC5C1A;
  static const Uint32 DDR_SDTIMR2   = 0x442EC742;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x0000090D; 

#elif defined (DM36x_ARM432_DDR340_OSC24)
   //24MHZ input

  static Uint32 PLL1_Mult     = 85;
  static Uint32 PLL1_PreDiv   = 5;   
  static Uint32 PLL1_Div1     = 0x1B;            
  static Uint32 PLL1_Div2     = 1;            
  static Uint32 PLL1_Div3     = 1;   
  static Uint32 PLL1_Div4     = 3;   
  static Uint32 PLL1_Div5     = 1;   
  static Uint32 PLL1_Div6     = 8;   

  static Uint32 PLL1_Div8     = 6;  
  static Uint32 PLL1_Div9     = 0x1B;

  // PLL2 VCO=594 MHz
  static Uint32 PLL2_Mult     = 0x9;

  static Uint32 PLL2_Div1     = 0x11; 

  static Uint32 PLL2_Div3     = 1;    
  static Uint32 PLL2_Div4     = 0x14; 
  static Uint32 PLL2_Div5     = 0x0f; 
  #ifdef GNU
  static Uint32 PLL1_Div7     = 0;  
  static Uint32 PLL2_PreDiv   = 0x0;
  static Uint32 PLL2_Div2     = 0;    
  #else
  static VUint32 PLL1_Div7     = 0;  
  static VUint32 PLL2_PreDiv   = 0x0;
  static VUint32 PLL2_Div2     = 0;    
  #endif
 
  // Clock Control (mux settings)
  static Uint32 ClockControl  = 0x343F04FC;
//  SYSTEM->PERI_CLKCTRL = 0x36ED04FC;

/*
//oritail date

  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  //static const Uint32 DDR_SDBCR     = 0x008534A32;        //0x08534A32;
  static const Uint32 DDR_SDBCR     = 0x08534832;        //0x08534A32;
  static const Uint32 DDR_SDTIMR    = 1471511963;
  static const Uint32 DDR_SDTIMR2   = 3559835459;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 2652;
*/

  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  //static const Uint32 DDR_SDBCR     = 0x008534A32;        //0x08534A32;
  static const Uint32 DDR_SDBCR     = 0x08534832;        //0x08534A32;
//  static const Uint32 DDR_SDTIMR    = 1471511963;
  static const Uint32 DDR_SDTIMR    = 0x57256C9A;
 // static const Uint32 DDR_SDTIMR2   = 3559835459;
static const Uint32 DDR_SDTIMR2   = 0x442EC742;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 2652;

/*
  static const Uint32 DDR_PHYCR     = 0x000000C5;
 // static const Uint32 DDR_SDBCR     = 0x008534A32;        //0x08534A32;
  static const Uint32 DDR_SDBCR     = 0x08534832;        //0x08534A32;
  static const Uint32 DDR_SDTIMR    = 0x05256C9A; //1471511963;
// static const Uint32 DDR_SDTIMR2   = 3559835459;
  static const Uint32 DDR_SDTIMR2   = 0xD42EC743;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 2652;
*/


/*
   //calculate by myself

  static const Uint32 DDR_PHYCR     = 0x000000C5;
  static const Uint32 DDR_SDBCR     = 0x08534832;        
  static const Uint32 DDR_SDTIMR    = 0x05256C9A;
  static const Uint32 DDR_SDTIMR2   = 0x442EC742;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x00000A5C;
*/
#elif defined(DM36x_ARM216_DDR173_OSC24)

  //24Mhz input
  static const Uint32 PLL1_Mult     = 0x73;
  static const Uint32 PLL1_PreDiv   = 0x0f;  
  //static const Uint32 PLL1_Div1     = 0;
  static const Uint32 PLL1_Div1     = 0xD;
  static const Uint32 PLL1_Div2     = 1;
  static const Uint32 PLL1_Div3     = 1;    
  static const Uint32 PLL1_Div4     = 3;    
  static const Uint32 PLL1_Div5     = 1;    
  static const Uint32 PLL1_Div6     = 0xc;  
  static const Uint32 PLL1_Div7     = 0;    
  static const Uint32 PLL1_Div8     = 3;    
  //static const Uint32 PLL1_Div9     = 0x1B;  
  static const Uint32 PLL1_Div9     = 0x1;  

  // PLL2 VCO=594 MHz
  //static const Uint32 PLL2_Mult     = 0xE0;
  static const Uint32 PLL2_Mult     = 0x12;
  //static const Uint32 PLL2_PreDiv   = 0x18;
  static const Uint32 PLL2_PreDiv   = 0x1;
  //static const Uint32 PLL2_Div1     = 0;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div1     = 0x11;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div2     = 0x1;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div3     = 1;    //              = 594/1   = 594 MHz
  //static const Uint32 PLL2_Div4     = 6;    // Voice        = 594/29  = 20.4872 MHz
  static const Uint32 PLL2_Div4     = 0x14;    // Voice        = 594/29  = 20.4872 MHz
  //static const Uint32 PLL2_Div5     = 0x0;  // VideoHD      = 594/8   = 74.25 MHz
  static const Uint32 PLL2_Div5     = 0xf;  // VideoHD      = 594/8   = 74.25 MHz

  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x343F04FC;
  //SYSTEM->PERI_CLKCTRL = 0x0BFFO5FC;

  // DDR timings
 static const Uint32 DDR_PHYCR     = 0x000000C6;
  static const Uint32 DDR_SDBCR     = 0x08534A32;
  static const Uint32 DDR_SDTIMR    = 0x2CDA3AC9;
  static const Uint32 DDR_SDTIMR2   = 0x9C17C723;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  //static const Uint32 DDR_SDRCR     = 0x0000083A;
  static const Uint32 DDR_SDRCR     = 0x545;

#else
  // Use DM365_297_DDR243 settings by default
  // 24MHz input
  // PLL1 VCO=486 MHz
  static const Uint32 PLL1_Mult     = 81;
  static const Uint32 PLL1_PreDiv   = 7;
  static const Uint32 PLL1_Div1     = 0;    
  static const Uint32 PLL1_Div2     = 1;    
  static const Uint32 PLL1_Div3     = 1;    // MJCP/HDVICP  = 486/2   = 243 MHz
  static const Uint32 PLL1_Div4     = 3;    // EDMA         = 486/4   = 121.5 MHz
  static const Uint32 PLL1_Div5     = 1;    // VPSS         = 486/2   = 243 MHz
  static const Uint32 PLL1_Div6     = 11;   // VENC         = 486/18  = 27 MHz
  static const Uint32 PLL1_Div7     = 0;    // DDR          = 486/1   = 486 MHz ==> DDR243
  static const Uint32 PLL1_Div8     = 4;    // SDMMC        = 486/4   = 121.5 MHz
  static const Uint32 PLL1_Div9     = 1;    // CLKOUT       = 486/2   = 243 MHz  
  
  // PLL2 VCO=594 MHz
  static const Uint32 PLL2_Mult     = 99;
  static const Uint32 PLL2_PreDiv   = 7;
  static const Uint32 PLL2_Div1     = 0;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div2     = 1;    // ARM          = 594/2   = 297 MHz
  static const Uint32 PLL2_Div3     = 1;    //              = 594/1   = 594 MHz
  static const Uint32 PLL2_Div4     = 28;   // Voice        = 594/29  = 20.4872 MHz
  static const Uint32 PLL2_Div5     = 7;    // VideoHD      = 594/8   = 74.25 MHz
  
  // Clock Control (mux settings)
  static const Uint32 ClockControl  = 0x243F04FC;
  
  // DDR timings
  static const Uint32 DDR_PHYCR     = 0x000000C5;
  static const Uint32 DDR_SDBCR     = 0x08534832;
  static const Uint32 DDR_SDTIMR    = 0x3C934B51;
  static const Uint32 DDR_SDTIMR2   = 0x4221C702;
  static const Uint32 DDR_PBBPR     = 0x000000FE;
  static const Uint32 DDR_SDRCR     = 0x00000768;
#endif


/************************************************************
* Global Variable Definitions                               *
************************************************************/

#if defined(DM36x_ARM216_DDR173_OSC19P2)
  const char devString[] = "DM365_216";  // 19.2 MHz oscillator
#elif defined(DM36x_ARM270_DDR216_OSC27)
  const char devString[] = "DM365_270";  // 24 MHz oscillator
#elif defined(DM36x_ARM297_DDR243_OSC24)
  const char devString[] = "DM365_297";  // 24 MHz Oscillator
#elif defined(DM36x_ARM297_DDR270_OSC24)
  const char devString[] = "DM365_297";  // 24 MHz Oscillator
#elif defined(DM36x_ARM297_DDR277_OSC27)
  const char devString[] = "DM365_297";  // 27 MHz Oscillator
#elif defined(DM36x_ARM216_DDR173_OSC24)
  const char devString[] = "DM365_297_OSC24";  // 24 MHz Oscillator
#elif defined(DM36x_ARM432_DDR340_OSC24)
  const char devString[] = "DM365_432 DDR 340";  // 24 MHz Oscillator
#elif defined(DM36x_ARM445_DDR351_OSC24)
  const char devString[] = "DM365_445 DDR 351";  // 24 MHz Oscillator
#else
  const char devString[] = "DM365_297"; // Default is DM365_297_DDR243
#endif     


/************************************************************
* Global Function Definitions                               *
************************************************************/

Uint32 DEVICE_init()
{
  Uint32 status = E_PASS;
  VUint32 temp;

  // Mask all interrupts
  AINTC->INTCTL = 0x4;
  AINTC->EABASE = 0x0;
  AINTC->EINT0  = 0x0;
  AINTC->EINT1  = 0x0;    
   
  // Clear all interrupts
  AINTC->FIQ0 = 0xFFFFFFFF;
  AINTC->FIQ1 = 0xFFFFFFFF;
  AINTC->IRQ0 = 0xFFFFFFFF;
  AINTC->IRQ1 = 0xFFFFFFFF;

#ifndef SKIP_LOW_LEVEL_INIT
  LOCAL_porReset();
  WDT_RESET();

  // System PSC setup - enable all
  DEVICE_PSCInit();
  
  DEVICE_pinmuxControl(0,0xFFFFFFFF,0x00FD0000);  // All Video Inputs
  DEVICE_pinmuxControl(1,0xFFFFFFFF,0x00145555);  // All Video Outputs
  DEVICE_pinmuxControl(2,0xFFFFFFFF,0x000000DA);  // EMIFA
  DEVICE_pinmuxControl(3,0xFFFFFFFF,0x00180000);  // SPI0, SPI1, UART1, I2C, SD0, SD1, McBSP0, CLKOUTs
  DEVICE_pinmuxControl(4,0xFFFFFFFF,0x55555555);  // MMC/SD0 instead of MS, SPI0

  GPIO->DIR02 &= 0xfeffffff;
  GPIO->CLRDATA02 = 0x01000000;

  // System PLL setup
  if (status == E_PASS) status |= DEVICE_PLL1Init();
  
  WDT_FLAG_ON();
  // DDR PLL setup
  if (status == E_PASS) status |= DEVICE_PLL2Init();
  
  // Do the clock mux setup after all plls are set
  SYSTEM->PERI_CLKCTRL = ClockControl;

  // DDR2 module setup
  if (status == E_PASS) status |= DEVICE_ExternalMemInit();
  #ifdef UBL_NAND
  if (status == E_PASS) status |= DEVICE_EMIFInit();
  #endif
  if (status == E_PASS) status |= DEVICE_TIMER0Init();
  // I2C0 Setup
  if (status == E_PASS) status |= DEVICE_I2C0Init();
#endif

  return status;
}

void WDT_FLAG_ON()
{

  SYSTEM->VPSS_CLKCTL &= 0xffffff7f;      // VPSS_CLKMD 1:2
  *TMPBUF = 0x591b3ed7; 
  *TMPSTATUS |= FLAG_FLGON;  
}

void DEVICE_LPSCTransition(Uint8 module, Uint8 domain, Uint8 state)
{
  // Wait for any outstanding transition to complete
  while ( (PSC->PTSTAT) & (0x00000001 << domain) );
  
  // If we are already in that state, just return
  if (((PSC->MDSTAT[module]) & 0x1F) == state) return;
    
  // Perform transition
  PSC->MDCTL[module] = ((PSC->MDCTL[module]) & (0xFFFFFFE0)) | (state);
  PSC->PTCMD |= (0x00000001 << domain);

  // Wait for transition to complete
  while ( (PSC->PTSTAT) & (0x00000001 << domain) );
  
  // Wait and verify the state
  while (((PSC->MDSTAT[module]) & 0x1F) != state);  
}


void DEVICE_pinmuxControl(Uint32 regOffset, Uint32 mask, Uint32 value)
{
  SYSTEM->PINMUX[regOffset] = (SYSTEM->PINMUX[regOffset] & ~mask) | (mask & value);
}


DEVICE_BootPeripheral DEVICE_bootPeripheral(void)
{
  DEVICE_BootMode bm = (DEVICE_BootMode) ((SYSTEM->BOOTCFG & DEVICE_BOOTCFG_BOOTMODE_MASK) >> DEVICE_BOOTCFG_BOOTMODE_SHIFT); 
  if (bm == DEVICE_BOOTMODE_NAND)
  {
    return DEVICE_BOOTPERIPHERAL_NAND;
  }
  else if (bm == DEVICE_BOOTMODE_NOR)
  {
    return DEVICE_BOOTPERIPHERAL_NOR;
  }
  else if (bm == DEVICE_BOOTMODE_SDMMC)
  {
    return DEVICE_BOOTPERIPHERAL_SDMMC;
  }
  else if (bm == DEVICE_BOOTMODE_UART)
  {
    return DEVICE_BOOTPERIPHERAL_UART;
  }
  else if (bm == DEVICE_BOOTMODE_USB)
  {
    return DEVICE_BOOTPERIPHERAL_USB;
  }
  else if (bm == DEVICE_BOOTMODE_SPI_MEM)
  {
    return DEVICE_BOOTPERIPHERAL_SPI_MEM;
  }
  else if (bm == DEVICE_BOOTMODE_EMAC)
  {
    return DEVICE_BOOTPERIPHERAL_EMAC;
  }
  else if (bm == DEVICE_BOOTMODE_HPI)
  {
    return DEVICE_BOOTPERIPHERAL_HPI;
  }
  else  
  {
    return DEVICE_BOOTPERIPHERAL_NONE;
  }
}


DEVICE_BootMode DEVICE_bootMode( void )
{
  return (DEVICE_BootMode) ((SYSTEM->BOOTCFG & DEVICE_BOOTCFG_BOOTMODE_MASK) >> DEVICE_BOOTCFG_BOOTMODE_SHIFT);
}


DEVICE_BusWidth DEVICE_emifBusWidth( void )
{
  if ( ( (SYSTEM->BOOTCFG & DEVICE_BOOTCFG_EMIFWIDTH_MASK) >> DEVICE_BOOTCFG_EMIFWIDTH_SHIFT) & 0x1 )
  {
    return DEVICE_BUSWIDTH_16BIT;
  }
  else
  {
    return DEVICE_BUSWIDTH_8BIT;
  }
}


void DEVICE_PSCInit()
{
  unsigned char i=0;
  unsigned char lpsc_start;
  unsigned char lpsc_end,lpscgroup,lpscmin,lpscmax;
  unsigned int  PdNum = 0; 
 
  lpscmin  =0;
  lpscmax  =2;
  
  for(lpscgroup=lpscmin ; lpscgroup <=lpscmax; lpscgroup++)
  {
    if(lpscgroup==0)
    {
      lpsc_start = 0; // Enabling LPSC 3 to 28 SCR first
      lpsc_end   = 28;
    }
    else if (lpscgroup == 1)
    { /* Skip locked LPSCs [29-37] */
      lpsc_start = 38;
      lpsc_end   = 47;
    }
    else if (lpscgroup == 2)
    {
      lpsc_start = 50;
      lpsc_end   = 51;
    }
    else
    {
      break;
    }

    //NEXT=0x3, Enable LPSC's
    for(i=lpsc_start; i<=lpsc_end; i++)
    {
      
      PSC->MDCTL[i] = (PSC->MDCTL[i] & ~0x0000001F) | (0x3 & 0x0000001F); 
    }

    //Program goctl to start transition sequence for LPSCs
    PSC->PTCMD = (1<<PdNum); 
       
    //Wait for GOSTAT = NO TRANSITION from PSC for Powerdomain 0
    while(! (((PSC->PTSTAT >> PdNum) & 0x00000001) == 0)); 

    //Wait for MODSTAT = ENABLE from LPSC's
    for(i=lpsc_start; i<=lpsc_end; i++)
    {
      while(!((PSC->MDSTAT[i] & 0x0000001F) == 0x3));   
    }  
  }    
}


Uint32 DEVICE_PLL1Init()
{
  unsigned int CLKSRC=0x0;            
  
  // Power up the PLL
  PLL1->PLLCTL &= 0xFFFFFFFD;    

  PLL1->PLLCTL &= 0xFFFFFEFF;        
  PLL1->PLLCTL |= CLKSRC<<8;
  
  // Set PLLENSRC '0', PLL Enable(PLLEN) selection is controlled through MMR
  PLL1->PLLCTL &= 0xFFFFFFDF;  
  
  // Set PLLEN=0 => PLL BYPASS MODE
  PLL1->PLLCTL &= 0xFFFFFFFE;
  
  UTIL_waitLoop(150);         
  
  // PLLRST=1(reset assert)
  PLL1->PLLCTL |= 0x00000008; 
  
  UTIL_waitLoop(300); 
  
  // Bring PLL out of Reset
  PLL1->PLLCTL &= 0xFFFFFFF7;
  
  
  // Program the Multiper and Pre-Divider for PLL1
  PLL1->PLLM   =   PLL1_Mult;   // VCO will (24*2*81)/7+1 = 486 Mhz for DDR
  PLL1->PREDIV =   DEVICE_PLLDIV_EN_MASK | PLL1_PreDiv;
   
  PLL1->SECCTL = 0x00470000;   // Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 1 
  PLL1->SECCTL = 0x00460000;   // Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 0 
  PLL1->SECCTL = 0x00400000;   // Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 0 
  PLL1->SECCTL = 0x00410000;   // Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 1

  // Program the PostDiv for PLL1
  PLL1->POSTDIV = DEVICE_PLLDIV_EN_MASK | 0x00;

  PLL1->PLLDIV1 = DEVICE_PLLDIV_EN_MASK | PLL1_Div1;
  // Post divider setting for PLL1
  PLL1->PLLDIV2 = DEVICE_PLLDIV_EN_MASK | PLL1_Div2;   
  PLL1->PLLDIV3 = DEVICE_PLLDIV_EN_MASK | PLL1_Div3;   
  PLL1->PLLDIV4 = DEVICE_PLLDIV_EN_MASK | PLL1_Div4;   
  PLL1->PLLDIV5 = DEVICE_PLLDIV_EN_MASK | PLL1_Div5;   
  PLL1->PLLDIV6 = DEVICE_PLLDIV_EN_MASK | PLL1_Div6;   
  PLL1->PLLDIV7 = DEVICE_PLLDIV_EN_MASK | PLL1_Div7;   
  PLL1->PLLDIV8 = DEVICE_PLLDIV_EN_MASK | PLL1_Div8;   
  PLL1->PLLDIV9 = DEVICE_PLLDIV_EN_MASK | PLL1_Div9;   

  UTIL_waitLoop(300);

  // Set the GOSET bit
  PLL1->PLLCMD = 0x00000001;  // Go
  UTIL_waitLoop(300);

  // Wait for PLL to LOCK
  while(! (((SYSTEM->PLL0_CONFIG) & 0x07000000) == 0x07000000));

  // Enable the PLL Bit of PLLCTL
  PLL1->PLLCTL |= 0x00000001;   // PLLEN=0

  return E_PASS;
}

Uint32 DEVICE_PLL2Init()
{
  unsigned int CLKSRC=0x0;

  // Power up the PLL
  PLL2->PLLCTL &= 0xFFFFFFFD;  
             
  // Select the Clock Mode as Onchip Oscilator or External Clock on MXI pin
  PLL2->PLLCTL &= 0xFFFFFEFF;        
  PLL2->PLLCTL |= CLKSRC<<8;
  
  // Set PLLENSRC '0', PLL Enable(PLLEN) selection is controlled through MMR
  PLL2->PLLCTL &= 0xFFFFFFDF;  
  
  // Set PLLEN=0 => PLL BYPASS MODE
  PLL2->PLLCTL &= 0xFFFFFFFE;
  
  UTIL_waitLoop(50);         
  
  // PLLRST=1 (reset assert)
  PLL2->PLLCTL |= 0x00000008;  

  UTIL_waitLoop(300); 

  // Bring PLL out of Reset
  PLL2->PLLCTL &= 0xFFFFFFF7;    
       
  // Program the Multiper and Pre-Divider for PLL2
  PLL2->PLLM    = PLL2_Mult;
  PLL2->PREDIV  = DEVICE_PLLDIV_EN_MASK | PLL2_PreDiv;
    
  PLL2->SECCTL = 0x00470000;   // Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 1 
  PLL2->SECCTL = 0x00460000;   // Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 0 
  PLL2->SECCTL = 0x00400000;   // Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 0 
  PLL2->SECCTL = 0x00410000;   // Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 1

  // Program the PostDiv for PLL2
  PLL2->POSTDIV = DEVICE_PLLDIV_EN_MASK | 0x00;
  
  PLL2->PLLDIV1 = DEVICE_PLLDIV_EN_MASK | PLL2_Div1;
  // Post divider setting for PLL2 
  PLL2->PLLDIV2 = DEVICE_PLLDIV_EN_MASK | PLL2_Div2;
  PLL2->PLLDIV3 = DEVICE_PLLDIV_EN_MASK | PLL2_Div3;
  PLL2->PLLDIV4 = DEVICE_PLLDIV_EN_MASK | PLL2_Div4;
  PLL2->PLLDIV5 = DEVICE_PLLDIV_EN_MASK | PLL2_Div5;
         
  //GoCmd for PostDivider to take effect
  PLL2->PLLCMD = 0x00000001;  
     
  UTIL_waitLoop(150);
                
  // Wait for PLL to LOCK
  while(! (((SYSTEM->PLL1_CONFIG) & 0x07000000) == 0x07000000)); 

  UTIL_waitLoop(4100);
         
  //Enable the PLL2
  PLL2->PLLCTL |= 0x00000001;   // PLLEN=0

  return E_PASS; 
}


Uint32 DEVICE_ExternalMemInit()
{
  DEVICE_LPSCTransition(LPSC_DDR2,0,PSC_ENABLE);
  
  SYSTEM->VTPIOCR = (SYSTEM->VTPIOCR) & 0xFFFF9F3F;
  
  // Set bit CLRZ (bit 13)
  SYSTEM->VTPIOCR = (SYSTEM->VTPIOCR) | 0x00002000;
  
  // Check VTP READY Status
  while( !(SYSTEM->VTPIOCR & 0x8000));     
  
  // Set bit VTP_IOPWRDWN bit 14 for DDR input buffers)
  //SYSTEM->VTPIOCR = SYSTEM->VTPIOCR | 0x00004000;         
  
  // Set bit LOCK(bit7) and PWRSAVE (bit8)
  SYSTEM->VTPIOCR = SYSTEM->VTPIOCR | 0x00000080;     
    
  // Powerdown VTP as it is locked (bit 6)
  // Set bit VTP_IOPWRDWN bit 14 for DDR input buffers)
  SYSTEM->VTPIOCR = SYSTEM->VTPIOCR | 0x00004040;
  
  // Wait for calibration to complete 
  UTIL_waitLoop( 150 );
  
  // Set the DDR2 to synreset, then enable it again
  DEVICE_LPSCTransition(LPSC_DDR2,0,PSC_SYNCRESET);
  DEVICE_LPSCTransition(LPSC_DDR2,0,PSC_ENABLE);
    
  DDR->DDRPHYCR = DDR_PHYCR; 
  // Program SDRAM Bank Config Register (set BOOTUNLOCK, clear TIMUNLOCK)
  DDR->SDBCR    = (DDR_SDBCR | (0x1 << DEVICE_SDCR_BOOTUNLOCK_SHIFT)) & (~DEVICE_SDCR_TIMUNLOCK_MASK);
  // Program SDRAM Bank Config Register (clear BOOTUNLOCK, set TIMUNLOCK)
  DDR->SDBCR    = (DDR_SDBCR | (0x1 << DEVICE_SDCR_TIMUNLOCK_SHIFT)) & (~DEVICE_SDCR_BOOTUNLOCK_MASK);
  DDR->SDTIMR   = DDR_SDTIMR;    //Program SDRAM Timing Control Register1
  DDR->SDTIMR2  = DDR_SDTIMR2;    //Program SDRAM Timing Control Register2
  DDR->PBBPR    = DDR_PBBPR;
  // Program SDRAM Bank Config Register (clear BOOTUNLOCK, clear TIMUNLOCK)
  DDR->SDBCR    = (DDR_SDBCR & (~DEVICE_SDCR_BOOTUNLOCK_MASK)) & (~DEVICE_SDCR_TIMUNLOCK_MASK);
  // Program SDRAM Refresh Control Register
  DDR->SDRCR    = DDR_SDRCR;

  DEVICE_LPSCTransition(LPSC_DDR2,0,PSC_SYNCRESET);
  DEVICE_LPSCTransition(LPSC_DDR2,0,PSC_ENABLE);

  return E_PASS;
}

Uint32 DEVICE_AsyncMemInit( Uint8 interfaceNum )
{
  if (interfaceNum == 0)
  {
    // Turn on the EMIF LPSC
    DEVICE_LPSCTransition(LPSC_AEMIF,PD0,PSC_ENABLE);

    // Set PINMUX for EMIF use
    DEVICE_pinmuxControl(2,DEVICE_PINMUX_EMIF_MASK,DEVICE_PINMUX_EMIF_EN);
  }
  else
  {
    return E_FAIL;
  }
  
  return E_PASS;
}

Uint32 DEVICE_UARTInit(Uint8 peripheralNum)
{
  if (peripheralNum == 0)
  { 
    // Reset and then power on the UART0 via PSC
    DEVICE_LPSCTransition(LPSC_UART0,PD0,PSC_SYNCRESET);
    DEVICE_LPSCTransition(LPSC_UART0,PD0,PSC_ENABLE);

    // The pin muxing registers must be set for UART0 use
    DEVICE_pinmuxControl(1,DEVICE_PINMUX_UART0_MASK,DEVICE_PINMUX_UART0_EN);
    UART0->PWREMU_MGMT = 0;         // Reset UART TX & RX components

    UTIL_waitLoop( 100 );

    UART0->MDR = 0x0;
    UART0->DLL = 0xd;               // Set baud rate	
    UART0->DLH = 0;
 

    UART0->FCR = 0x0007;            // Clear UART TX & RX FIFOs
    UART0->FCR = 0x0000;            // Non-FIFO mode
    UART0->IER = 0x0007;            // Enable interrupts
  
    UART0->LCR = 0x0003;            // 8-bit words,
                                  // 1 STOP bit generated,
                                  // No Parity, No Stick paritiy,
                                  // No Break control
    UART0->MCR = 0x0000;            // RTS & CTS disabled,
                                  // Loopback mode disabled,
                                  // Autoflow disabled

    UART0->PWREMU_MGMT = 0xE001;
  }
  else if (peripheralNum == 1)
  {
  }
  else if (peripheralNum == 2)
  {
  }
  else
  {
    return E_FAIL;
  } 
  
  return E_PASS;
}

Uint32 DEVICE_SPIInit(Uint8 periphNum)
{
  if ( periphNum == 0)
  {
    // Make sure SPI0 is powered up
    DEVICE_LPSCTransition(LPSC_SPI0, PD0, PSC_ENABLE);
    // Set the PINMUX3 for enabling SPI0     
    DEVICE_pinmuxControl(3, 0xFF000000, 0x36000000);
  }
  else
  {
    return E_FAIL;
  }
  
  return E_PASS;
}


Uint32 DEVICE_I2CInit(Uint8 peripheralNum)
{
    return E_PASS;
}

Uint32 DEVICE_SDMMCInit(Uint8 peripheralNum)
{
  if (peripheralNum == 0)
  {
    // Make sure SDMMC0 is powered up
    DEVICE_LPSCTransition(LPSC_SDMMC0, PD0, PSC_ENABLE);
    
    // Set the PINMUX3 for enabling SDMMC
    // FIXME - what pinmux settings?
    DEVICE_pinmuxControl(3, 0xFF000000, 0x36000000);
  }
  else
  {
    return E_FAIL;
  }
  
  return E_PASS; 
}


Uint32 DEVICE_TIMER0Init()
{
  // Put timer into reset
  TIMER0->EMUMGT_CLKSPD = 0x00000003;
  TIMER0->TCR           = 0x00000000;
  
  // Enable TINT0, TINT1 interrupt
  TIMER0->INTCTL_STAT   = 0x00000001;
  
  // // Set to 64-bit GP Timer mode, enable TIMER12 & TIMER34
  TIMER0->TGCR = 0x00000003;

  // Reset timers to zero 
  TIMER0->TIM12 = 0x00000000;
  TIMER0->TIM34 = 0x00000000;
  
  // Set timer period (5 second timeout = (24000000 * 5) cycles = 0x07270E00) 
  TIMER0->PRD34 = 0x00000000;
  TIMER0->PRD12 = 0x07270E00;

  return E_PASS;
}


void DEVICE_TIMER0Start(void)
{
  // Clear interrupt
  AINTC->IRQ1   |=  0x00000001;

  // Put timer in reset
  TIMER0->TGCR  =   0x00000000;

  // Reset timer count to zero 
  TIMER0->TIM12 =   0x00000000;

  // Setup for one-shot mode
  TIMER0->TCR   =   0x00000040;

  // Start TIMER12
  TIMER0->TGCR  = 0x00000005;
}


void DEVICE_TIMER0Stop(void)
{
  // Clear interrupt
  AINTC->IRQ1   |=  0x00000001;

  // Put timer in reset
  TIMER0->TCR   = 0x00000000;
  TIMER0->TGCR  = 0x00000000;

  // Reset timer count to zero 
  TIMER0->TIM12 = 0x00000000;
}


Uint32 DEVICE_TIMER0Status(void)
{
  return ((AINTC->IRQ1)&0x1);
}

Uint32 DEVICE_I2C0Init(void)
{
  I2C0->ICMDR   = 0;                // Reset I2C
  I2C0->ICPSC   = 26;               // Config prescaler for 27MHz
  I2C0->ICCLKL  = 20;               // Config clk LOW for 20kHz
  I2C0->ICCLKH  = 20;               // Config clk HIGH for 20kHz
  I2C0->ICMDR   |= I2C_ICMDR_IRS;   // Release I2C from reset

  return E_PASS;
}


Uint32 DEVICE_EMIFInit(void)
{
  AEMIF->AWCCR = 0xff;

  //AEMIF->A1CR = 0x40400204;
  AEMIF->A1CR = 0x00400204;

  AEMIF->NANDFCR |= 1;

  AEMIF->A2CR = 0x00a00505;

  return E_PASS;
}

/************************************************************
* Local Function Definitions                                *
************************************************************/
#ifndef SKIP_LOW_LEVEL_INIT

static void LOCAL_porReset()
{
  if ((PLL1->RSTYPE)&3)
  {
    LOCAL_vpssSyncReset();  // VPSS sync reset
    
    *TMPBUF = 0;
    *TMPSTATUS |= FLAG_PORRST;

    *GPINT_GPEN = 0x00020000;
    
    *GPTDAT_GPDIR = 0x00020002;
    
    while(1);
  }
}

static void LOCAL_vpssSyncReset()
{
  unsigned int PdNum = 0;

  SYSTEM->VPSS_CLKCTL |= 0x00000080;            // VPSS_CLKMD 1:1

  //LPSC SyncReset DDR Clock Enable
  PSC->MDCTL[47] = ((PSC->MDCTL[47] & 0xffffffe0) | 0x00000001);        
            
  PSC->PTCMD = (1<<PdNum);

  while(! (((PSC->PTSTAT >> PdNum) & 0x00000001) == 0));      

  while(!((PSC->MDSTAT[47] &  0x0000001F) == 0x1));           
}

void WDT_RESET()
{
  volatile unsigned int s;

  if((*TMPBUF == 0x591b3ed7)){
    *TMPBUF = 0;
    *TMPSTATUS |= FLAG_PORRST;
    *TMPSTATUS |= FLAG_FLGOFF;  

    for (s=0;s<0x100;s++) {}

    LOCAL_vpssSyncReset();
    *GPINT_GPEN = 0x00020000;                                   // WDT
    *GPTDAT_GPDIR = 0x00020002;                                 // execute >
    while(1);
  }
}

#endif

/***********************************************************
* End file                                                 *
***********************************************************/



