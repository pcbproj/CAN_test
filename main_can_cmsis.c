/* 
project for STM32F103C8 MCU


*/

#include "main.h"

#define DATA_LENGTH_CODE 8

void RCC_Init(void);

uint8_t can_init(void);
uint8_t can_send(uint8_t * pData, uint8_t dataLength);
int main(void){
    volatile uint16_t counter = 0;
    uint8_t data[] = "ABCDEFGHIJ9";
    RCC_Init();
    can_init();
    while(1){
        can_send(data, sizeof(data));
        while(counter<0xFFFF) counter++;
		    counter = 0;
    }
}



uint8_t can_init(void){
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN; /* turn on clocking for CAN */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; /* turn on clocking for GPIOA */
    /* PA11 - CAN_RX */
    GPIOA->CRH	&= ~GPIO_CRH_CNF11;     /* CNF11 = 00 */
    GPIOA->CRH	|= GPIO_CRH_CNF11_1;	  /* CNF11 = 10 -> AF Out | Push-pull (CAN_RX) */
    GPIOA->CRH 	|= GPIO_CRH_MODE11;     /* MODE11 = 11 -> Maximum output speed 50 MHz */
    /* PA12 - CAN_TX */
    GPIOA->CRH	&= ~GPIO_CRH_CNF12;	    /* CNF12 = 00 */
    GPIOA->CRH	|= GPIO_CRH_CNF12_1;	  /* CNF12 = 10 -> AF Out | Push-pull (CAN_TX) */
    GPIOA->CRH 	|= GPIO_CRH_MODE12;     /* MODE12 = 11 -> Maximum output speed 50 MHz */
    CAN1->MCR |= CAN_MCR_INRQ;          /* Initialization Request */
    CAN1->MCR |= CAN_MCR_NART;          /* Not autoretranslate transmission */
    CAN1->MCR |= CAN_MCR_AWUM;          /* Automatic Wakeup Mode */
    /* clean and set Prescaler = 9 */
    CAN1->BTR &= ~CAN_BTR_BRP;          
    CAN1->BTR |= 8U;			// << CAN_BTR_BRP;
    /* clean and set T_1s = 13, T_2s = 2 */
    CAN1->BTR &= ~CAN_BTR_TS1;
    CAN1->BTR |= 12U << 16;		// CAN_BTR_TS1_Pos;
    CAN1->BTR &= ~CAN_BTR_TS2;
    CAN1->BTR |= 1U << 20;		// CAN_BTR_TS2_Pos;

    CAN1->sTxMailBox[0].TIR &= ~CAN_TI0R_RTR;                    /* data frame */
    CAN1->sTxMailBox[0].TIR &= ~CAN_TI0R_IDE;                    /* standart ID */ 
    CAN1->sTxMailBox[0].TIR &= ~CAN_TI0R_STID;
    CAN1->sTxMailBox[0].TIR |= (0x556U << 21);	// CAN_TI0R_STID_Pos);
    CAN1->sTxMailBox[0].TDTR &= ~CAN_TDT0R_DLC;                  /* length of data in frame */
    CAN1->sTxMailBox[0].TDTR |= (DATA_LENGTH_CODE);	// CAN_TDT0R_DLC_Pos);
    CAN1->MCR &= ~CAN_MCR_INRQ;                                  /* go to normal mode */ 
    return 0;	
}

uint8_t can_send(uint8_t * pData, uint8_t dataLength){
    uint16_t i = 0;
    uint8_t j = 0;
    while (!(CAN1->TSR & CAN_TSR_TME0)){
        i++;
        if (i>0xEFFF) return 1;
    }
    i = 0;
    CAN1->sTxMailBox[0].TDLR = 0;
    CAN1->sTxMailBox[0].TDHR = 0;
    while (i<dataLength){
        if (i>(DATA_LENGTH_CODE-1)){
            CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;                 /* Transmit Mailbox Request */
            dataLength -= i;
            j++;
            while (!(CAN1->TSR & CAN_TSR_TME0)){                      /* Transmit mailbox 0 empty? */
                i++;
                if (i>0xEFFF) return 1;
            }
        if (CAN1->TSR & CAN_TSR_TXOK0){}                          /* Tx OK? */
        //else return ((CAN1->ESR & CAN_ESR_LEC)>>CAN_ESR_LEC_Pos); / return Last error code /
        i = 0;
        CAN1->sTxMailBox[0].TDLR = 0;
        CAN1->sTxMailBox[0].TDHR = 0;
        }
        if (i<4)
	          CAN1->sTxMailBox[0].TDLR |= (pData[i+j*DATA_LENGTH_CODE]*1U << (i*8));
        else
	          CAN1->sTxMailBox[0].TDHR |= (pData[i+j*DATA_LENGTH_CODE]*1U << (i*8-32));
        i++;
    }
    CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ; /* Transmit Mailbox Request */
    if (CAN1->TSR & CAN_TSR_TXOK0) return 0;
    else return ((CAN1->ESR & CAN_ESR_LEC) >> 4);	//CAN_ESR_LEC_Pos); /* return Last error code */
}