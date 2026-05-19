// Procedures utiles pour la carte MSP-EXP430F5529
// Pierre-Yves Rochat, 2020-26, EPFL, pyr@pyr.ch

#ifndef CARTE2026_432_H_
#define CARTE2026_432_H_

#include <msp432p401r.h>
#include <stdint.h>

// Definition LED et poussoirs pour la carte 2026

#define Led1On P2OUT|=(1<<5)
#define Led1Off P2OUT&=~(1<<5)
#define Led1Toggle P2OUT^=(1<<5)
#define InitLed1 P2DIR|=(1<<5);Led1Off

#define Led2On P3OUT|=(1<<0)
#define Led2Off P3OUT&=~(1<<0)
#define Led2Toggle P3OUT^=(1<<0)
#define InitLed2 P3DIR|=(1<<0);Led2Off

#define Led3On P5OUT|=(1<<1)
#define Led3Off P5OUT&=~(1<<1)
#define Led3Toggle P5OUT^=(1<<1)
#define InitLed3 P5DIR|=(1<<1);Led3Off

#define Led4On P6OUT|=(1<<1)
#define Led4Off P6OUT&=~(1<<1)
#define Led4Toggle P6OUT^=(1<<1)
#define InitLed4 P6DIR|=(1<<1);Led4Off

#define Led5On P4OUT|=(1<<0)
#define Led5Off P4OUT&=~(1<<0)
#define Led5Toggle P4OUT^=(1<<0)
#define InitLed5 P4DIR|=(1<<0);Led5Off

#define Led6On P4OUT|=(1<<2)
#define Led6Off P4OUT&=~(1<<2)
#define Led6Toggle P4OUT^=(1<<2)
#define InitLed6 P4DIR|=(1<<2);Led6Off

#define Led7On P4OUT|=(1<<4)
#define Led7Off P4OUT&=~(1<<4)
#define Led7Toggle P4OUT^=(1<<4)
#define InitLed7 P4DIR|=(1<<4);Led7Off

#define Led8On P4OUT|=(1<<5)
#define Led8Off P4OUT&=~(1<<5)
#define Led8Toggle P4OUT^=(1<<5)
#define InitLed8 P4DIR|=(1<<5);Led8Off

#define Pous1On (!(P5IN&(1<<2)))
#define InitPous1 P5DIR&=~(1<<2);P5REN|=(1<<2);P5OUT|=(1<<2)

#define Pous2On (!(P2IN&(1<<6)))
#define InitPous2 P2DIR&=~(1<<6);P2REN|=(1<<6);P2OUT|=(1<<6)

#define Pous3On (!(P2IN&(1<<7)))
#define InitPous3 P2DIR&=~(1<<7);P2REN|=(1<<7);P2OUT|=(1<<7)

#define Pous4On (!(P5IN&(1<<0)))
#define InitPous4 P5DIR&=~(1<<0);P5REN|=(1<<0);P5OUT|=(1<<0)

#define Pous5On (!(P4IN&(1<<6)))
#define InitPous5 P4DIR&=~(1<<6);P4REN|=(1<<6);P4OUT|=(1<<6)

// LED du Launchpad MSP432 :
#define LedRougeInit P1DIR|=(1<<0);P1OUT&=~(1<<0)
#define LedRougeOn P1OUT|=(1<<0)
#define LedRougeOff P1OUT&=~(1<<0)
#define LedRougeToggle P1OUT^=(1<<0)

#define LedCoulInit P2DIR|=(1<<0)|(1<<1)|(1<<2);P2OUT&=~((1<<0)|(1<<1)|(1<<2))
#define LedCoulRougeOn P2OUT|=(1<<0)
#define LedCoulRougeOff P2OUT&=~(1<<0)
#define LedCoulRougeToggle P2OUT^=(1<<0)
#define LedCoulVertOn P2OUT|=(1<<1)
#define LedCoulVertOff P2OUT&=~(1<<1)
#define LedCoulVertToggle P2OUT^=(1<<1)
#define LedCoulBleuOn P2OUT|=(1<<2)
#define LedCoulBleuOff P2OUT&=~(1<<2)
#define LedCoulBleuToggle P2OUT^=(1<<2)

// Moteurs du robot :

#define MoteurDroiteInit P9DIR|=(1<<2);P9OUT&=~(1<<2);P6DIR|=(1<<2);P6OUT&=~(1<<2)
#define MoteurDroiteOn P6OUT|=(1<<2)
#define MoteurDroiteOff P6OUT&=~(1<<2)
#define MoteurDroiteRecule P9OUT|=(1<<2)
#define MoteurDroiteAvance P9OUT&=~(1<<2)

#define MoteurGaucheInit P8DIR|=(1<<4);P8OUT&=~(1<<4);P8DIR|=(1<<2);P8OUT&=~(1<<2)
#define MoteurGaucheOn P8OUT|=(1<<2)
#define MoteurGaucheOff P8OUT&=~(1<<2)
#define MoteurGaucheRecule P8OUT|=(1<<4)
#define MoteurGaucheAvance P8OUT&=~(1<<4)

// Capteurs de route

#define CaptIrInit P10DIR |= (1<<4)
#define CaptIrOn P10OUT |= (1<<4)
#define CaptIrOff P10OUT &=~(1<<4);

#define Capt7Low !(P7IN & (1<<1))
#define Capt7In P7DIR &=~(1<<1)
#define Capt7Charge P7DIR |=(1<<1); P7OUT |=(1<<1)

#define Capt6Low !(P9IN & (1<<4))
#define Capt6In P9DIR &=~(1<<4)
#define Capt6Charge P9DIR |=(1<<4); P9OUT |=(1<<4)

#define Capt5Low !(P9IN & (1<<6))
#define Capt5In P9DIR &=~(1<<6)
#define Capt5Charge P9DIR |=(1<<6); P9OUT |=(1<<6)

#define Capt4Low !(P8IN & (1<<0))
#define Capt4In P8DIR &=~(1<<0)
#define Capt4Charge P8DIR |=(1<<0); P8OUT |=(1<<0)

#define Capt3Low !(P7IN & (1<<4))
#define Capt3In P7DIR &=~(1<<4)
#define Capt3Charge P7DIR |=(1<<4); P7OUT |=(1<<4)

#define Capt2Low !(P7IN & (1<<6))
#define Capt2In P7DIR &=~(1<<6)
#define Capt2Charge P7DIR |=(1<<6); P7OUT |=(1<<6)

#define Capt1Low !(P10IN & (1<<0))
#define Capt1In P10DIR &=~(1<<0)
#define Capt1Charge P10DIR |=(1<<0); P10OUT |=(1<<0)

#define Capt0Low !(P10IN & (1<<2))
#define Capt0In P10DIR &=~(1<<2)
#define Capt0Charge P10DIR |=(1<<2); P10OUT |=(1<<2)

void InitCarte2026();

void AfficheLedBleues(uint16_t val) ;

void delay_ms(unsigned int ms);

#endif /* PYR_CARTEBLANCHE_H_ */
