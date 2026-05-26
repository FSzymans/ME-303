// Base robot MSP432
// EPFL 2020-2026, Pierre-Yves Rochat, pyr@pyr.ch

#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <msp432p401r.h>
#include <Clock.h>
#include <gui.h>

#include "lcd.h"
#include "Carte2026-432.h"

// ========== PWM moteurs ===================

#define LIMITE_MIN_MOTEUR 1000

volatile int16_t CommandeDroite; // gestion par TA1 CCR2
volatile int16_t CommandeGauche; // gestion par TA1 CCR3

volatile int16_t debPwmMot;

void InitPwmMoteur(){
  MoteurGaucheInit;
  MoteurDroiteInit;
  CommandeDroite = CommandeGauche = 0;

  TA1CTL = TASSEL_2 | MC_2 | ID_1; // TA1 en mode UP

  NVIC->ISER[0] = 1 << ((TA1_N_IRQn) & 31);
  TA1CCTL1 = CCIE; TA1CCR1 = 0; // PWM debut cycle
  TA1CCTL2 = CCIE; TA1CCR2 = 0; // fin cycle droite
  TA1CCTL3 = CCIE; TA1CCR3 = 0; // fin cycle gauche
}


void TA1_N_IRQHandler(void){
  switch (TA1IV) {
  case 2 : // TA1 CCR1 : debut du cycle suivant
      TA1CCTL1 &=~CCIFG;
      TA1CCR2 = TA1CCR3 = 0;
      MoteurDroiteOff; MoteurGaucheOff; MoteurDroiteOff; MoteurGaucheOff;
      if (CommandeDroite>LIMITE_MIN_MOTEUR) { TA1CCR2 = CommandeDroite*2; MoteurDroiteAvance; MoteurDroiteOn; }
      if (CommandeDroite<-LIMITE_MIN_MOTEUR) { TA1CCR2 = -CommandeDroite*2; MoteurDroiteRecule; MoteurDroiteOn; }
      if (CommandeGauche>LIMITE_MIN_MOTEUR) { TA1CCR3 = CommandeGauche*2; MoteurGaucheAvance; MoteurGaucheOn; }
      if (CommandeGauche<-LIMITE_MIN_MOTEUR) { TA1CCR3 = -CommandeGauche*2; MoteurGaucheRecule; MoteurGaucheOn; }
      debPwmMot = 1; break;
  case 4 : TA1CCTL2 &=~CCIFG;
      MoteurDroiteOff; break; // TA1 CCR2 : fin cycle moteur droite
  case 6 : TA1CCTL3 &=~CCIFG;
      MoteurGaucheOff; break; // TA1 CCR3 : fin cycle moteur gauche
  default : break;
  }
}

// ========== Capteurs route ===================

#define PERIODE_CAPTEURS 180
#define PERIODE_CHARGE 1000
#define MAX_CAPT 254

volatile int16_t ValCapteurs[8];
int32_t cptCapt;
uint8_t captValide;

void InitCapteurs(){
  CaptIrInit;
  cptCapt = -1; // pour initialiser la charge des condensateurs
  CaptIrOn;

  ValCapteurs[0]=ValCapteurs[1]=ValCapteurs[2]=ValCapteurs[3]=255;
  ValCapteurs[4]=ValCapteurs[5]=ValCapteurs[6]=ValCapteurs[7]=255;

  TA1CTL = TASSEL_2 | MC_2 | ID_0 ;
  NVIC->ISER[0] = 1 << ((TA1_0_IRQn) & 31);
  TA1CCTL0 = CCIE;
  TA1CCR0 = PERIODE_CHARGE;
}

// Timer A1 CCR0 :echantillonnage des capteurs de route
void TA1_0_IRQHandler(void){
  TA1CCTL0 &=~CCIFG; // indispensable sur MSP432 ?
  if (cptCapt == -1) { // charge condensateurs
    Capt0Charge; Capt1Charge; Capt2Charge; Capt3Charge;
    Capt4Charge; Capt5Charge; Capt6Charge; Capt7Charge;
    cptCapt = 0;
    TA1CCR0 += PERIODE_CHARGE;
  } else if (cptCapt == 0) {
    Capt0In; Capt1In; Capt2In; Capt3In; Capt4In; Capt5In; Capt6In; Capt7In;

    captValide = 0;
    TA1CCR0 += PERIODE_CAPTEURS;
    cptCapt = 1;
  } else if (cptCapt < MAX_CAPT) {
    // Detecte les fronts descendants des capteurs :
    if (!(captValide&(1<<0)) && Capt0Low) { ValCapteurs[0] = cptCapt; captValide|=(1<<0); }
    if (!(captValide&(1<<1)) && Capt1Low) { ValCapteurs[1] = cptCapt; captValide|=(1<<1); }
    if (!(captValide&(1<<2)) && Capt2Low) { ValCapteurs[2] = cptCapt; captValide|=(1<<2); }
    if (!(captValide&(1<<3)) && Capt3Low) { ValCapteurs[3] = cptCapt; captValide|=(1<<3); }
    if (!(captValide&(1<<4)) && Capt4Low) { ValCapteurs[4] = cptCapt; captValide|=(1<<4); }
    if (!(captValide&(1<<5)) && Capt5Low) { ValCapteurs[5] = cptCapt; captValide|=(1<<5); }
    if (!(captValide&(1<<6)) && Capt6Low) { ValCapteurs[6] = cptCapt; captValide|=(1<<6); }
    if (!(captValide&(1<<7)) && Capt7Low) { ValCapteurs[7] = cptCapt; captValide|=(1<<7); }
    cptCapt++;
    TA1CCR0 += PERIODE_CAPTEURS;
  } else if (cptCapt == MAX_CAPT) {
    cptCapt = -1;
    TA1CCR0 += PERIODE_CAPTEURS;
  }
}

//================ Barres de visualisation des catpeurs de route ==============

#define BAR_X0 20
#define BAR_Y0 50
#define BAR_X_LG 200
#define BAR_Y_EPAIS 4
#define BAR_Y_DY 20
#define VAL_MAX 120

void Barre(uint16_t val, uint32_t id, uint16_t coul){
    uint32_t i;
    if (val > VAL_MAX) { val = VAL_MAX; }
    uint32_t v = val * BAR_X_LG / VAL_MAX;

    LCD_SetWindows(BAR_X0, LCD_W-(BAR_Y0+(BAR_Y_DY*id)),
                   BAR_X0+v, LCD_W-(BAR_Y0+(BAR_Y_DY*id)+BAR_Y_EPAIS));
    for(i=0; i<(v+1)*(BAR_Y_EPAIS+0); i++){ Lcd_WriteData_16Bit(coul); }

    LCD_SetWindows(BAR_X0+v, LCD_W-(BAR_Y0+(BAR_Y_DY*id)),
                   BAR_X0+BAR_X_LG, LCD_W-(BAR_Y0+(BAR_Y_DY*id)+BAR_Y_EPAIS));
    for(i=0; i<(BAR_X_LG-v+1)*(BAR_Y_EPAIS+0); i++){ Lcd_WriteData_16Bit(BLACK); }
}

//================== Convertisseurs AD =======================

void InitAdc(){
    P8->SEL0 |= (BIT6); P8->SEL1 |= (BIT6);
    P8->SEL0 |= (BIT7); P8->SEL1 |= (BIT7);
    P9->SEL0 |= (BIT1); P9->SEL1 |= (BIT1);
    P8->SEL0 |= (BIT3); P8->SEL1 |= (BIT3);

    P8->SEL0 |= (BIT5); P8->SEL1 |= (BIT5);
    P9->SEL0 |= (BIT0); P9->SEL1 |= (BIT0);

    P5->SEL0 |= (BIT3); P5->SEL1 |= (BIT3);
    P4->SEL0 |= (BIT7); P4->SEL1 |= (BIT7);

    REF_A->CTL0 = REF_A_CTL0_VSEL_3; // reference interne ï¿½ 2.5 V
    REF_A->CTL0 |= REF_A_CTL0_ON; // REF ON
    while (!(REF_A->CTL0 & REF_A_CTL0_GENACT)) {} // stabilisation
}

uint16_t AdcRead(uint16_t ch) {
  ADC14->CTL0 = ADC14_CTL0_SHP | ADC14_CTL0_CONSEQ_1 | ADC14_CTL0_MSC | ADC14_CTL0_ON;
  ADC14->CTL0 |= ADC14_CTL0_SHT1__16;
  switch (ch) {
    case 0 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_19; break; // Select P8.6
    case 1 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_18; break; // Select P8.7
    case 2 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_16; break; // Select P9.1
    case 3 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_22; break; // Select P8.3

    case 4 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_20; break; // Select P8.5
    case 5 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_17; break; // Select P9.0

    case 8 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_2; break; // Select P5.3
    case 9 : ADC14->MCTL[0] = ADC14_MCTLN_INCH_6; break; // Select P4.7
  default : break;
  }
  ADC14->MCTL[0] |= ADC14_MCTLN_VRSEL_14;
  ADC14->MCTL[1] = ADC14_MCTLN_EOS;

  ADC14->CTL1 |= ADC14_CTL1_RES__14BIT; // Select bit resolution
  ADC14->CTL0 |= ADC14_CTL0_ENC;

  ADC14->CTL0 |= ADC14_CTL0_SC; // start conversion
  while (!(ADC14->IFGR0 & BIT0)) {} // wait end conversion
  return ADC14->MEM[0];
}

//================ Lecture et affichage de la tension des batteries ==============

#define REP_ADC 200
#define DIV_ADC ((10.0f+4.7f)/4.7f) // Diviseur resistif en entree

#define X0_VOLT 260
#define Y0_VOLT 25

void AfficheTension() {
    uint32_t i, val;
    float v;
    char ch[50] = "XXX";

    val = 0;
    for (i=0; i<REP_ADC; i++){
      val += AdcRead(4); // lecture du +5 V avec diviseur 10k - 4.7k
    }

    v = val * 2.5f * DIV_ADC / (REP_ADC  * 16383.0f);
    snprintf(ch, sizeof(ch), "%.2f V   ", v);
    Show_Str(X0_VOLT, Y0_VOLT, WHITE, BLACK, (u8*)&ch, 16, 0);
}

// =============== Programme principal =================

// =============== States =================
enum State {IDLE, INFINITY, ZEBRA, DANCE, DANCE_T};
enum State state;

// =============== Global variables =================

//================ Affichage of crossing counter ==============
#define X_CC 235
#define Y_CC 50
uint8_t cross_counter = 0;

void AfficheCC(){
    char cc[50] = "XX";
    snprintf(cc, sizeof(cc), "%d COUNTER   ", cross_counter);
    Show_Str(X_CC, Y_CC, WHITE, BLACK, (u8*)&cc, 16, 0);
}

//================ Affichage of mode ==============
#define X_TXT 260
#define Y_TXT 175

#define X_MODE 240
#define Y_MODE 190

char * mode[] = {"IDLE", "INFINITY", "ZEBRA", "DANCE", "DANCE_T"};


void AfficheMode(){
	char txt[50] = "MODE";
	char buffer[50]="IDLE";
	snprintf(buffer, sizeof(buffer), "%-8s", mode[state]);
	Show_Str(X_TXT, Y_TXT, RED, BLACK,(u8*)&txt, 16, 0);
	Show_Str(X_MODE, Y_MODE, WHITE, BLACK,(u8*)&buffer, 16, 0);
}

//================ Affichage of DANCE_T ==============
#define X_HEAD 155
#define Y_HEAD 140

#define X_TORSO 145
#define Y_TORSO 160

#define X_LEGS 140
#define Y_LEGS 185

volatile uint8_t rythm = 0;

char head[10] = "o";
char torso[10] = "/|\\" ;
char legs[10] = "_||_";

void AfficheDance(){

	switch(rythm){
		case 0:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "/|\\");
			snprintf(legs, sizeof(legs), "%-4s", "_||_");
			break;
		case 1:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "-|-");
			snprintf(legs, sizeof(legs), "%-4s", "_/\\_");
			break;
		case 2:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "-|~");
			snprintf(legs, sizeof(legs), "%-4s", "_||_");
			break;
		case 3:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "~|~");
			snprintf(legs, sizeof(legs), "%-4s", "_|\\_");
			break;
		case 4:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "~|~");
			snprintf(legs, sizeof(legs), "%-4s", "_/|_");
			break;
		case 5:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "~|-");
			snprintf(legs, sizeof(legs), "%-4s", "_/\\_");
			break;
		case 6:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "-|-");
			snprintf(legs, sizeof(legs), "%-4s", "_/\\_");
			break;
		case 7:
			snprintf(head, sizeof(head), "%-1s", "o");
			snprintf(torso, sizeof(torso), "%-3s", "/|\\");
			snprintf(legs, sizeof(legs), "%-4s", "_||_");
			break;
	}


	if(state==DANCE_T){
		Show_Str(X_HEAD, Y_HEAD, WHITE, BLACK,(u8*)&head, 16, 0);
		Show_Str(X_TORSO, Y_TORSO, WHITE, BLACK,(u8*)&torso, 16, 0);
		Show_Str(X_LEGS, Y_LEGS, WHITE, BLACK,(u8*)&legs, 16, 0);
	}
}

void cleanDance(){

	snprintf(head, sizeof(head), "%-1s", "");
	snprintf(torso, sizeof(torso), "%-3s", "");
	snprintf(legs, sizeof(legs), "%-4s", "");

	Show_Str(X_HEAD, Y_HEAD, WHITE, BLACK,(u8*)&head, 16, 0);
	Show_Str(X_TORSO, Y_TORSO, WHITE, BLACK,(u8*)&torso, 16, 0);
	Show_Str(X_LEGS, Y_LEGS, WHITE, BLACK,(u8*)&legs, 16, 0);

}

//================ Affichage of crossing counter ==============
#define X_r 235
#define Y_r 100

void AfficheR(){
    char r[50] = "XX";
    snprintf(r, sizeof(r), "%d RYTHM   ", rythm);
    Show_Str(X_r, Y_r, WHITE, BLACK, (u8*)&r, 16, 0);
}

// Timer
void TA2_N_IRQHandler(void){
	switch (TA1IV) {
	  case 2 : // TA1 CCR1 : debut du cycle suivant

		  TA2CCR1 += 65534;
		  TA2CCTL1 &=~CCIFG;
		 	      if (state == DANCE_T){
		 			  switch(rythm){
		 				  case 0:
		 					  CommandeDroite=15000;
		 					  CommandeGauche=0;

		 					  break;
		 				  case 1:
		 					  CommandeDroite=-15000;
		 					  CommandeGauche=0;

		 							  break;
		 				  case 2:
		 					  CommandeDroite=10000;
		 					  CommandeGauche=10000;
		 							  break;
		 				  case 3:
		 					  CommandeDroite=-10000;
		 					  CommandeGauche=-10000;
		 							  break;
		 				  case 4:
		 					  CommandeDroite=0;
		 					  CommandeGauche=15000;
		 							  break;
		 				  case 5:
		 					  CommandeDroite=0;
		 					  CommandeGauche=-15000;
		 							  break;
		 				  case 6:
		 					  CommandeDroite=-10000;
		 					  CommandeGauche=-10000;
		 							  break;
		 				  case 7:
		 					  CommandeDroite=10000;
		 					  CommandeGauche=10000;
		 							  break;
		 			  }
		 			  rythm++;
		 			  if(rythm>8){
		 				  state=IDLE;
		 				  cleanDance();
		 				  TA2CCR1 += 65534;
		 			  }
		 	      }
	  case 4 : break;
	  case 6 :  break;
	  default : break;
	}
}

// =============== Start ===============
int32_t Centre;

void FindCentre() {
  Centre = 0;
  Centre += ValCapteurs[0]*7;
  Centre += ValCapteurs[1]*5;
  Centre += ValCapteurs[2]*4;
  Centre += ValCapteurs[3]*1;
  Centre -= ValCapteurs[4]*1;
  Centre -= ValCapteurs[5]*4;
  Centre -= ValCapteurs[6]*5;
  Centre -= ValCapteurs[7]*7;
}
void FollowLine(){
    int16_t Kp = 4;
    if(Centre >= 0){
        CommandeDroite = 7000 - ( Kp * Centre );
        CommandeGauche = 7000;
    }
    if(Centre < 0){
        CommandeDroite = 7000;
        CommandeGauche = 7000 + ( Kp * Centre );
    }
}
int8_t black = 0;
volatile uint16_t c = 0;
void CrossLine(){
    int16_t Sum = 0;
    uint8_t i;

    for (i=0; i<8; i++) {
                    Sum += ValCapteurs[i];
    }


     if(Sum < 800){

         if((ValCapteurs[0] <120) && (ValCapteurs[7] < 120)){
             CommandeDroite = 4000;
             CommandeGauche = 4000;
             delay_ms(50);
             c++;
         }
         if((ValCapteurs[0] > 120) && (ValCapteurs[7] < 120)){
              CommandeDroite = 2000;
              CommandeGauche = 4000;
              delay_ms(50);
              c++;
              //black = 1;
         }
         if((ValCapteurs[0] < 120) && (ValCapteurs[7] > 120)){
              CommandeDroite = 4000;
              CommandeGauche = 3000;
              delay_ms(50);
              c++;
              //black = 1;
         }


     }
     else{
         CommandeDroite = 4000;
         CommandeGauche = 4000;
         delay_ms(100);
         c = 0;

     }
     if(c > 20){
         state=IDLE;
         c = 0;
     }
}

void InfinityRoute(){
	uint8_t crossing = 0;
	uint8_t f = 0;
	uint8_t n_max_capt = 0;
	for (f= 0; f < 8; f++){
		if(ValCapteurs[f] > 60) n_max_capt++;
	}

	if(n_max_capt > 4){
		crossing++;
		delay(20);
	}
	if (crossing) cross_counter++;
	if (cross_counter >= 2) {
		CommandeDroite = 0;
		CommandeGauche = 0;
	} else FollowLine();

}

void Dance(){
	uint8_t i = 0;
	for(i=0; i<4;i++){
		CommandeDroite=15000;
		CommandeGauche=0;
		delay(8000*1000);
		CommandeDroite=0;
		CommandeGauche=15000;
		delay(8000*1000);
		CommandeDroite=0;
		CommandeGauche=15000;
		delay(8000*1000);
	}


}
void StopBot(){
	CommandeDroite=0;
	CommandeGauche=0;
}

// ============== End ============
void main(void) {
  WDTCTL = WDTPW | WDTHOLD;
  Clock_Init48MHz();
  InitCarte2026();

  InitPwmMoteur();
  InitCapteurs();
  LCD_Init();
  InitAdc();

  POINT_COLOR=WHITE;
  LCD_Fill(0, 00, lcddev.width, 20, RED);
  Gui_StrCenter(0, 2, WHITE, BLUE, "CHAOS BOT", 16, 1);
  LCD_Fill(0, lcddev.height-20, lcddev.width, lcddev.height, BLUE);
  Gui_StrCenter(0, lcddev.height-18, WHITE, BLUE, "EPFL", 16, 1);//


  // Timer dance t
  TA2CTL = TASSEL_1 | MC_2 | ID_0; // TA2 en mode UP

  NVIC->ISER[0] |= 1 << ((TA2_N_IRQn) & 31);
  TA2CCTL1 = CCIE;
  TA2CCR1 += 65534;
  //Timer dance t

  while(1) {
    while(!debPwmMot) {} debPwmMot = 0; // synchronisation avec le PWM

    FindCentre();
    AfficheLedBleues(Centre);


    if(state!=DANCE_T){
    	uint32_t i; // capteurs 0 - 7
    	for(i=0; i<8; i++) { Barre(ValCapteurs[i], i, YELLOW); }
    }


    switch(state) {
    	case IDLE://instead do in interrupt part?
    		//cross_counter=0;
    		StopBot();
    		if (Pous1On && Pous2On){
    			state=DANCE;
    		}else if(Pous1On){
    			state=INFINITY;
    			cross_counter=0;
    		}else if(Pous2On){
    			state=ZEBRA;
    		}else if(Pous3On){
    			state=DANCE_T;
    			rythm = 0;
    		}else state = IDLE;
    		break;
    	case ZEBRA:
    		CrossLine();
    		break;
    	case DANCE:
    		Dance();
    		state=IDLE;
    		break;
    	case INFINITY:
    		InfinityRoute();
    		if(cross_counter >= 2) state = IDLE;
    		break;
    	case DANCE_T:
    		break;
    }

    AfficheTension();
    AfficheCC();
    AfficheMode();
    AfficheDance();
    AfficheR();
  }
}


