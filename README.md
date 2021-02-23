# WaterControl

## CHDTECH
S1(BUTTON) P0_1 pull-up R10KOM  
S2(BUTTON) P2_0 pull-up R10KOM  

D1(LED) - Green; P1_0  
D2(LED) - Red;   P1_1  
D3(LED) - Green; P1_4  

---------------------------------  
RELAY 1          P0_7   
RELAY 2          P0_6  

COUNTER HOT      P1_2  
COUNTER COLD     P1_3  
RESET BUTTON     P0_1  



76543210 - numberss
10000100   132
00001100   12

P0_1    00000010 2
P0_2    00000100 4
        00001100 12


P2_0 - P2_4 - неполноценный порт

P0_0 - P0_7 - полноценный порт
P1_0 - P1_7 - полноценный порт


P2_1 - DC (Debug Clock)  
P2_2 - DD (Debug Data)  


Свободные порты:
//Ну что бы мозга не ебать
P2_0 - Свободный порт
P2_3 - Свободный порт
P2_4 - Свободный порт

E18-MS1PA1-PCB (свободные пины)
  0     1    2     3     4      5    6     7  
P2_0  
            P1_2  P1_3  P1_4  P1_5  P1_6  P1_7  
P0_0  P0_1  P0_2  P0_3  P0_4  P0_5  P0_6  


---------------------------------
P2_0 - MAIN BUTTON

0 порт занять данными

P0_0 - COLD   COUNTER
P0_1 - HOT    COUNTER

P0_2 - OTHER  COUNTER
P0_3 - OTHER  COUNTER

D3(LED) - Green; P1_4

P1_2 - RELAY COLD
P1_3 - RELAY HOT

free ports
  0     1    2     3     4      5    6     7  
                              P1_5  P1_6  P1_7  
                        P0_4  P0_5  P0_6  
