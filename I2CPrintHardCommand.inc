'*****************************************************************************************************************
'*  Name    : I2CPrintHardCommand.inc                                                                            *
'*  Purpose : allow Proton Print commands to access the standard 1602 LCD via I2C backpack board                 * 
'*  Author  : George Towler                                                                                      *
'*  Notice  : I claim no copyright but maybe Barak does                                                          *
'*  Date    : 18/01/2015                                                                                         *
'*  Compiler: 3.5.7.1                                                                                            *
'*  Version : 0.9.0                                                                                              *
'*  Notes   : see http://www.protonbasic.co.uk/showthread.php/62478-%93Print%94-command-for-SPI-Alfa-numeric-LCD *                                                  
'*          :     post #6   Barak shows how to replace Proton commands and how to deal with control (FE$) bytes  *                                                                                                                               
'*****************************************************************************************************************

  #Disable Print                          ;Disable the Proton Print command    

  ;***************************************************************************************************************
  ; Configure items to suit your LCD and I2C hardware, I2CPrint_DataOnHighBits remmed if data is on low bits     *
  ;                                                                                                              *
  Symbol I2CPrint_address         = %01110000   ' PCF8574 Slave Address 0,1,1,1,A2,A1,A0,RW  where W=0           *
  Symbol I2CPrint_InitForLCDLines = $28         ' $28 for two lines  ($36 for 4 lines ??)                        *
  Symbol I2CPrint_RS              = %00000001   '                                                                *
  Symbol I2CPrint_RW              = %00000010   '                                                                *
  Symbol I2CPrint_E               = %00000100   '                                                                *
  Symbol I2CPrint_BackLight       = %00001000   '                                                                *
  $define I2CPrint_DataOnHighBits               '                                                                *
  ;***************************************************************************************************************
 
  Symbol I2CPrint_RS_E           = I2CPrint_RS | I2CPrint_E
  
  $ifdef I2CPrint_DataOnHighBits
    Symbol I2CPrint_Init1          = %00110000 
    Symbol I2CPrint_Init1_E        = %00110000 | I2CPrint_E
    Symbol I2CPrint_Init2          = %00110000  
    Symbol I2CPrint_Init2_E        = %00110000 | I2CPrint_E
    Symbol I2CPrint_Init3          = %00100000 
    Symbol I2CPrint_Init3_E        = %00100000 | I2CPrint_E
  $else
    Symbol I2CPrint_Init1          = %00000011  
    Symbol I2CPrint_Init1_E        = %00000011 | I2CPrint_E
    Symbol I2CPrint_Init2          = %00000011  
    Symbol I2CPrint_Init2_E        = %00000011 | I2CPrint_E
    Symbol I2CPrint_Init3          = %00000010  
    Symbol I2CPrint_Init3_E        = %00000010 | I2CPrint_E
  $endif

  Dim BPF As Byte System
  Symbol I2CPrint_tComOrData = BPF.0
  Symbol I2CPrint_tInitialised = BPF.1  

  Dim I2CPrint_PrintStore As Byte
  Dim I2CPrint_PrintData As Byte
  Dim I2CPrint_HighData As Byte
  Dim I2CPrint_LowData As Byte
  Dim I2CPrint_dummy As Byte
  Dim I2CPrint_RSFlag As Byte  
  Dim Backlight_State As Byte

  GoTo I2CPrintCommand_End

  @Print                                              ; assembler Print redirect
  Wreg_Byte I2CPrint_PrintStore                       ; save the W Register

  If I2CPrint_tInitialised = 0 Then                   ; Has LCD been inititalised?
    DelayMS 100                                       ; Power-on delay for slow display types
    HBusOut I2CPrint_address, [I2CPrint_Init1_E]      ; E hi and send command nibble 0011 (3)
    HBusOut I2CPrint_address, [I2CPrint_Init1]        ; E lo
    DelayMS 20
    HBusOut I2CPrint_address, [I2CPrint_Init2_E]      ; E hi and send command nibble 0011 (3)    
    HBusOut I2CPrint_address, [I2CPrint_Init2]        ; E lo     
    DelayMS 20
    HBusOut I2CPrint_address, [I2CPrint_Init3_E]      ; E hi and send command nibble 0010 (2)    
    HBusOut I2CPrint_address, [I2CPrint_Init3]        ; E lo     
    DelayMS 20
    I2CPrint_PrintData = I2CPrint_InitForLCDLines     ; 4 bit mode and n lines 
    GoSub I2CPrint_SendControl
    I2CPrint_PrintData = $0C                          ; enable display, cursor and blink off
    GoSub I2CPrint_SendControl
    I2CPrint_PrintData = $06                          ; move cursor after each write 
    GoSub I2CPrint_SendControl                    
    I2CPrint_PrintData = $80                          ; goto column 1 line 1
    GoSub I2CPrint_SendControl
    Set I2CPrint_tInitialised                         ; Indicate that the LCD is initialised
  EndIf

  I2CPrint_PrintData = I2CPrint_PrintStore
  If I2CPrint_PrintData = 0xFE Then                   ; it is a command header
    I2CPrint_RSFlag = 0                               ; clear a flag so we know the next byte will be a command
  Else
    If I2CPrint_RSFlag = 0 Then                       ; this is a command byte
      If I2CPrint_PrintData = 03 Then
        Backlight_State = I2CPrint_BackLight  
      ElseIf I2CPrint_PrintData = 04 Then
        Backlight_State = 0  
      Else
        GoSub I2CPrint_SendControl
      End If
    Else
      GoSub I2CPrint_SendData
    EndIf
    I2CPrint_RSFlag = 1                               ; set the flag so the next byte will be send as Data
  EndIf                                     
  Byte_Wreg I2CPrint_PrintStore                       ; restore the W register
  Return

' ****************************************************************************
' **                             S U B R O U T I N E S                      **
' ****************************************************************************

I2CPrint_SendData:
  $ifdef I2CPrint_DataOnHighBits
    I2CPrint_HighData = I2CPrint_PrintData & %11110000            ; mask LSBs
    I2CPrint_LowData = I2CPrint_PrintData << 4                    ; shift left
  $else
    I2CPrint_LowData = I2CPrint_PrintData & %00001111             ; mask MSBs
    I2CPrint_HighData = I2CPrint_PrintData >> 4                   ; shift Right
  $endif

  I2CPrint_dummy = I2CPrint_RS_E | I2CPrint_HighData              ; data and e hi
  I2CPrint_dummy = I2CPrint_dummy | Backlight_State               ; data and e hi and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]             
  I2CPrint_dummy = I2CPrint_RS | I2CPrint_HighData                ; data and e lo
  I2CPrint_dummy = I2CPrint_dummy | Backlight_State               ; data and e lo and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]              
  I2CPrint_dummy = I2CPrint_RS | Backlight_State                  ; data and e hi
  HBusOut I2CPrint_address, [I2CPrint_dummy]                      ; Delay, RS on throughout
  I2CPrint_dummy = I2CPrint_RS_E | I2CPrint_LowData               ; data and e hi
  I2CPrint_dummy = I2CPrint_dummy | Backlight_State               ; data and e hi and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]              
  I2CPrint_dummy = I2CPrint_RS | I2CPrint_LowData                 ; data and e lo
  I2CPrint_dummy = I2CPrint_dummy | Backlight_State               ; data and e lo and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy] 
  Return
  
I2CPrint_SendControl:
  $ifdef I2CPrint_DataOnHighBits
    I2CPrint_HighData = I2CPrint_PrintData & %11110000            ; mask LSBs
    I2CPrint_LowData = I2CPrint_PrintData << 4                    ; shift left
  $else
    I2CPrint_LowData = I2CPrint_PrintData & %00001111             ; mask MSBs
    I2CPrint_HighData = I2CPrint_PrintData >> 4                   ; shift Right
  $endif

  I2CPrint_dummy = I2CPrint_E | I2CPrint_HighData                 ; data and e hi
  I2CPrint_dummy = I2CPrint_dummy | Backlight_State               ; data and e hi and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]              
  I2CPrint_dummy = I2CPrint_HighData | Backlight_State            ; data and e lo and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]              
  I2CPrint_dummy = Backlight_State                                ; 
  HBusOut I2CPrint_address, [I2CPrint_dummy]                      ; Delay
  I2CPrint_dummy = I2CPrint_E | I2CPrint_LowData                  ; data and e hi
  I2CPrint_dummy = I2CPrint_dummy | Backlight_State               ; data and e hi and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]              
  I2CPrint_dummy = I2CPrint_LowData | Backlight_State             ; data and e lo and bls
  HBusOut I2CPrint_address, [I2CPrint_dummy]         
  Return

I2CPrintCommand_End:
