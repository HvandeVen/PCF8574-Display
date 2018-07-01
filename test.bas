'Simple test program for PCF8574A I2C Display with 20x4 lines
'Using a PIC16F628A without hardware I2C
'By Harm van de Ven (https://www.hvandeven.com) & (https://github.com/hvandeven)

Device 16F628A
Config INTRC_OSC_NOCLKOUT, WDT_OFF, PWRTE_ON, LVP_OFF, MCLRE_OFF, BODEN_OFF, BOREN_OFF, DATA_CP_OFF
Xtal 4
All_Digital TRUE

Clear
DelayMS 500 'Short delay to stabilize the controller

Declare SCL_Pin = PORTB.4
Declare SDA_Pin = PORTB.5
Declare Slow_Bus On

Include "I2CPrintSoftCommand.inc" 'Include the software I2C file since the 16F628A has no hardware I2C

Dim lights As Word
lights = 0

Main:   ' Main loop
              '12345678901234567890
Print At 1,1, "  DCF8574A DISPLAY  "
Print At 2,1, "TEST TEST  TEST TEST"
Print At 3,1, "20 x 4 lines display"
Print At 4,1, "  Harm van de Ven   "
DelayMS 250

' toggle backlight
If lights = 0 Then
    Print $FE, 3 'Backlight on
    lights = 1
    GoTo Main
EndIf 

If lights = 1 Then
    Print $FE, 4 'Backlight off
    lights = 0
    GoTo Main
EndIf 

GoTo Main
End
