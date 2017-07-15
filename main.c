//--------------------------------------------------------------------------------------------------
//                                  _            _     
//                                 | |          | |    
//      ___ _ __ ___  ___ _   _ ___| |_ ___  ___| |__  
//     / _ \ '_ ` _ \/ __| | | / __| __/ _ \/ __| '_ \. 
//    |  __/ | | | | \__ \ |_| \__ \ ||  __/ (__| | | |
//     \___|_| |_| |_|___/\__, |___/\__\___|\___|_| |_|
//                         __/ |                       
//                        |___/    Engineering (www.emsystech.de)
//
// Filename:    main.c
// Description: Demonstration for RaspiLCD
//    
// Open Source Licensing 
//
// This program is free software: you can redistribute it and/or modify it under the terms of 
// the GNU General Public License as published by the Free Software Foundation, either version 3 
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.  
// If not, see <http://www.gnu.org/licenses/>.
//
// Dieses Programm ist Freie Software: Sie können es unter den Bedingungen der GNU General Public
// License, wie von der Free Software Foundation, Version 3 der Lizenz oder (nach Ihrer Option) 
// jeder späteren veröffentlichten Version, weiterverbreiten und/oder modifizieren.
//
// Dieses Programm wird in der Hoffnung, dass es nützlich sein wird, aber OHNE JEDE GEWÄHRLEISTUNG,
// bereitgestellt; sogar ohne die implizite Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR 
// EINEN BESTIMMTEN ZWECK. Siehe die GNU General Public License für weitere Details.
//
// Sie sollten eine Kopie der GNU General Public License zusammen mit diesem Programm erhalten 
// haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
//                       
// Author:      Martin Steppuhn
// History:     05.09.2012 Initial version
//				18.10.2012 Update vor erster Auslieferung
//				05.11.2012 Version 0.9.0
//				29.07.2013 Version 1.0.0 for new Hardwareversion (6x Buttons)
//--------------------------------------------------------------------------------------------------

//=== Includes =====================================================================================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>
#include <stdio.h>
#include "std_c.h"
#include "bcm2835.h"
#include "raspilcd.h"
#include "lcd.h"

//=== Preprocessing directives (#define) ===========================================================

//=== Type definitions (typedef) ===================================================================

//=== Global constants =============================================================================

//=== Global variables =============================================================================

//=== Local constants  =============================================================================

#include "bmp_raspi.inc"
#include "bmp_men.inc"

//=== Local variables ==============================================================================

uint16  DemoCount;
char	FunctionView;
char	TempString[32];

uint16	DemoMem[256];
uint16	CpuUsageMem[3][128];

//=== Local function prototypes ====================================================================

//--------------------------------------------------------------------------------------------------
// Name:      Demo Functions
// Function:  
//            
// Parameter: 
// Return:    
//--------------------------------------------------------------------------------------------------
void DemoLogo(void)
{
	LCD_ClearScreen();
	LCD_DrawBitmap(0,0,bmp_raspi);		
	LCD_SetFont(0);	
	LCD_PrintXY(70,4 ,"Raspi-LCD");		
	LCD_PrintXY(75,14,"Project");
	LCD_PrintXY(68,32,"powered by");
	LCD_PrintXY(70,42,"Emsystech");
	LCD_PrintXY(62,52,"Engineering");
}

void DemoText(void)
{	
	LCD_ClearScreen();
	LCD_SetFont(1);	
	LCD_PrintXY(30,0,"Raspi-LCD");
	LCD_PrintXY(0,12,"www.emsystech.de");
	LCD_SetFont(0);	
	LCD_PrintXY(8,29,"128 x 64 Pixel (BW)");
	LCD_PrintXY(6,38,"White LED Backlight");
	LCD_PrintXY(4,47,"8 Lines with 21 Char ");
	LCD_PrintXY(4,56,"in the smallest Font ");
}

void DemoVector(void)
{
	LCD_ClearScreen();
	LCD_SetFillColor(1);	
	LCD_DrawRect(110,20,120,60,1);
	LCD_DrawLine(0,0,127,0);
	LCD_DrawLine(0,0,127,16);
	LCD_DrawLine(0,0,127,32);
	LCD_DrawLine(0,0,127,48);
	LCD_DrawLine(0,0,127,63);
	LCD_SetPenColor(1);
	LCD_DrawCircle(63,31,31);
	LCD_DrawCircle(8,50,5);
	LCD_DrawEllipse(80,40, 30,10);
	LCD_SetFillColor(0);	
	LCD_DrawRect(12,20,40,40,3);
	LCD_SetFillColor(-1);	
	LCD_DrawRect(30,50,60,60,1);
}

void LogCpuTemperature(void)
{
	FILE *fp;
	unsigned int temp;
	uint16 i;
			
	TempString[0] = 0;
	
	fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");		// open as file
	if(fp != NULL)
	{	
		fgets(TempString,32,fp);			// get line
		fclose(fp);
	}
	
	temp = 0;
	if(TempString[0])
	{
		TempString[3]=0;	// end at 1/10 °C
		sscanf(TempString,"%u",&temp);
		printf("%u\r\n",temp);
		TempString[3]=TempString[2]; TempString[2]='.'; TempString[4]='°'; TempString[5]='C'; TempString[6]=0;
	}
	
	for(i=126;i>0;i--)  DemoMem[i+1] = DemoMem[i];
	DemoMem[1] = DemoMem[0];
	DemoMem[0] = temp;
}	

void LogCpuUsage(void)
{
	uint16 i;
	int AvgType;
	double loadavg[3];
	getloadavg(loadavg,3);
	
	for(AvgType = 0; AvgType < 3; AvgType++) {
		for(i=126;i>0;i--) { 
			CpuUsageMem[AvgType][i+1] = CpuUsageMem[AvgType][i];
		}
		CpuUsageMem[AvgType][1] = CpuUsageMem[AvgType][0];
		CpuUsageMem[AvgType][0] = loadavg[AvgType]*100;
		if (CpuUsageMem[AvgType][0] > 100) {
			CpuUsageMem[AvgType][0] = 100;
		}
	}
}	

void CpuUssage(int AvgType)
{
	uint16	x,y,oldY=0;
	char	OutputString[32];
	char	AvgString[10];
	const int MAX_Y = 60;
	const int MIN_Y = 3;
	const int MAX_X = 128;
	const int MIN_X = 21;	
	LCD_ClearScreen();
	LCD_SetPenColor(1);
	LCD_SetFont(0);
	if(AvgType == 0) { strncpy(AvgString,"1m avg",10); }
	else if (AvgType == 1) { strncpy(AvgString,"5m avg",10); }
	else if (AvgType == 2) { strncpy(AvgString,"15m avg",10); }
	else { printf("invalid AvgType\n");
	   exit(1); 
   }
	snprintf(OutputString,32,"CPU %s:%d%%\n",AvgString, CpuUsageMem[AvgType][0]);
	LCD_PrintXY(30,0,OutputString);
		
	LCD_PrintXY(0,0, "100-");
	LCD_PrintXY(0,MAX_Y/2,"50 -");
	LCD_PrintXY(0,MAX_Y-3,"0  -");
	LCD_DrawLine(MIN_X,0,MIN_X,63);
	for(x=MAX_X;x>MIN_X;x--)
	{
		//start drawing from the right side MAX_X-x
		//convert the percentage to the drawing area (MAX_Y-MIN_Y)/100*CpuUsageMem
		y = (float)(MAX_Y-MIN_Y)/100*CpuUsageMem[AvgType][MAX_X-x];
		//invert as 0 is top of the display
		y = MAX_Y-y;
		LCD_DrawLine(x,y,x+1,oldY);
		oldY=y;
	}
}
void DemoCpuTemperature(void)
{
	uint16	i,y;
	
	LCD_ClearScreen();
	LCD_SetPenColor(1);
	LCD_SetFont(1);
	LCD_PrintXY(40,0,"CPU:");
	LCD_PrintXY(80,0,TempString);
		
	LCD_SetFont(0);
	LCD_PrintXY(0,0, "60-");
	LCD_PrintXY(0,18,"50-");
	LCD_PrintXY(0,37,"40-");
	LCD_PrintXY(0,56,"30-");
	LCD_DrawLine(15,0,15,63);

	for(i=16;i<128;i++)
	{
		y = DemoMem[127-i];
		
		if(y > 290) 
		{
			y = ((y - 290) / 5);
			y = 64 - y;
			LCD_PutPixel(i,y,1);
			LCD_PutPixel(i,y+1,1);
		}
	}
}

void DemoCpuTemperatureInit(void)
{
	uint8 i;
	for(i=0;i<128;i++)  DemoMem[i] = 0;
}

void DemoBubbles(void)
{
	LCD_ClearScreen();
	LCD_SetPenColor(1);
	
	if(DemoMem[200])	{ if(DemoMem[201] > 16)	DemoMem[201]--; else	DemoMem[200] = 0;	}
		else		{ if(DemoMem[201] <48)	DemoMem[201]++; else	DemoMem[200] = 1;	}
	DemoMem[202] = ((63 - DemoMem[201]) < DemoMem[201]) ? (63 - DemoMem[201]) : DemoMem[201];	
	DemoMem[202] = (DemoMem[202] > 20) ? 20 : DemoMem[202];	
	LCD_DrawEllipse(28,DemoMem[201],20+20-DemoMem[202],DemoMem[202]);	

	if(DemoMem[203])	{ if(DemoMem[204] > 14)	DemoMem[204]--; else	DemoMem[203] = 0;	}
		else		{ if(DemoMem[204] <50)	DemoMem[204]++; else	DemoMem[203] = 1;	}
	DemoMem[205] = ((63 - DemoMem[204]) < DemoMem[204]) ? (63 - DemoMem[204]) : DemoMem[204];	
	DemoMem[205] = (DemoMem[205] > 20) ? 20 : DemoMem[205];	
	LCD_DrawEllipse(65,DemoMem[204],22+10-DemoMem[205],DemoMem[205]);	

	if(DemoMem[206])	{ if(DemoMem[207] > 10)	DemoMem[207]--; else	DemoMem[206] = 0;	}
		else		{ if(DemoMem[207] <54)	DemoMem[207]++; else	DemoMem[206] = 1;	}
	DemoMem[208] = ((63 - DemoMem[207]) < DemoMem[207]) ? (63 - DemoMem[207]) : DemoMem[207];	
	DemoMem[208] = (DemoMem[208] > 15) ? 15 : DemoMem[208];	
	LCD_DrawEllipse(102,DemoMem[207],15+20-DemoMem[208],DemoMem[208]);	
}

void DemoBubblesInit(void)
{
	DemoMem[200] = 1;
	DemoMem[201] = 10;
	DemoMem[203] = 0;
	DemoMem[204] = 40;
	DemoMem[206] = 1;
	DemoMem[207] = 40;
}

void DemoFont(void)
{
	LCD_ClearScreen();
	LCD_SetFont(0);		LCD_PrintXY(0,0, "Font 0");
	LCD_SetFont(1);		LCD_PrintXY(0,8, "Font 1");
	LCD_SetFont(2);		LCD_PrintXY(0,23,"Font 2");
	LCD_SetFont(3);		LCD_PrintXY(0,39,"Font 3");
}

void DemoBitmap(void)
{
	LCD_DrawBitmap(0,0,bmp_men);	
	LCD_SetFont(1);		LCD_PrintXY(6,0, "Bitmap");
}
 
int getch() {
    static int ch = -1, fd = 0;
    struct termios neu, alt;
    fd = fileno(stdin);
    tcgetattr(fd, &alt);
    neu = alt;
    neu.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(fd, TCSANOW, &neu);
    ch = getchar();
    tcsetattr(fd, TCSANOW, &alt);
    return ch;
}	
 
int kbhit(void) {
        struct termios term, oterm;
        int fd = 0;
        int c = 0;
        tcgetattr(fd, &oterm);
        memcpy(&term, &oterm, sizeof(term));
        term.c_lflag = term.c_lflag & (!ICANON);
        term.c_cc[VMIN] = 0;
        term.c_cc[VTIME] = 1;
        tcsetattr(fd, TCSANOW, &term);
        c = getchar();
        tcsetattr(fd, TCSANOW, &oterm);
        if (c != -1)
        ungetc(c, stdin);
        return ((c != -1) ? 1 : 0);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{ 
	int Contrast,Backlight;
	char c;

	Contrast = 9;
	Backlight = 1;
			
	printf("RaspiLCD Demo V1.0.0 by Martin Steppuhn [" __DATE__ " " __TIME__"]\n");
	printf("RaspberryHwRevision=%i\r\n",GetRaspberryHwRevision());
		
	if(!RaspiLcdHwInit()) { printf("RaspiLcdHwInit() failed!\r\n"); return 1; }
	LCD_Init();			// Init Display
	SetBacklight(1);	// Turn Backlight on
	
	DemoCpuTemperatureInit();
	FunctionView = 0;
	while(1)
	{
		DemoCount++;
		SleepMs(50);
		UpdateButtons();
		
		if(Button) printf("Buttons: %02X (%02X) Contrast=%i Backlight=%u\r\n",Button,ButtonPressed,Contrast,Backlight);

		if((DemoCount & 3) == 0) {
			LogCpuTemperature();
			LogCpuUsage();
		}			
			
		if(BUTTON_PRESSED_UP || BUTTON_PRESSED_DOWN)
		{
			if(BUTTON_PRESSED_UP   && (Contrast < 20))  Contrast++; 
			if(BUTTON_PRESSED_DOWN && (Contrast > 0))   Contrast--; 
			LCD_SetContrast(Contrast); 
		}
		if(BUTTON_PRESSED_D)
		{
			Backlight = (Backlight) ? 0 : 1;	// Toggle Backlight
			SetBacklight(Backlight);			// Write to Hardware
		}		
		
		if(BUTTON_PRESSED_A) { FunctionView--; }		
		if(BUTTON_PRESSED_B) {FunctionView++; }
		if (FunctionView >  5) { FunctionView = 0; }
		if (FunctionView <  0) { FunctionView = 5; }
	
		if(     FunctionView == 0)	 { DemoLogo(); }
		else if(FunctionView == 1)	 { if((DemoCount & 3) == 0) DemoCpuTemperature(); }
		else if(FunctionView == 2)  { if((DemoCount & 3) == 0) CpuUssage(0); }
		else if(FunctionView == 3)  { if((DemoCount & 3) == 0) CpuUssage(1); }
		else if(FunctionView == 4)	 { if((DemoCount & 3) == 0) CpuUssage(2); }
		 
					
		LCD_WriteFramebuffer();
		
		if(kbhit())
		{
			c=getch();
			printf("\r\nINIT !!!\r\n\r\n");
			LCD_Init();			// Init Display
			SetBacklight(1);	// Turn Backlight on
			FunctionView = 0;
		}		
	}
	return(0);
}
