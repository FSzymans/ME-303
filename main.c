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

    REF_A->CTL0 = REF_A_CTL0_VSEL_3; // reference interne � 2.5 V
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
// =============== Start ===============
int32_t Centre;

void FindCentre() {
  Centre = 0;
  Centre += ValCapteurs[0]*7;
  Centre += ValCapteurs[1]*5;
  Centre += ValCapteurs[2]*3;
  Centre += ValCapteurs[3]*1;
  Centre -= ValCapteurs[4]*1;
  Centre -= ValCapteurs[5]*3;
  Centre -= ValCapteurs[6]*5;
  Centre -= ValCapteurs[7]*7;
}
void FollowLine(){
    int16_t Kp = 10;
    if(Centre >= 0){
        CommandeDroite = 10000 - ( Kp * Centre );
        CommandeGauche = 10000;
    }
    if(Centre < 0){
        CommandeDroite = 10000;
        CommandeGauche = 10000 + ( Kp * Centre );
    }
}
void CrossLine(){
    int16_t Kp = 3;
    if(Centre >= 0){
        CommandeDroite = 7000;
        CommandeGauche = 7000 + ( Kp * Centre );
    }
    if(Centre < 0){
        CommandeDroite = 7000 - ( Kp * Centre );
        CommandeGauche = 7000;
    }
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
  Gui_StrCenter(0, 2, WHITE, BLUE, "ROBOT 2026", 16, 1);
  LCD_Fill(0, lcddev.height-20, lcddev.width, lcddev.height, BLUE);
  Gui_StrCenter(0, lcddev.height-18, WHITE, BLUE, "EPFL", 16, 1);//

  while(1) {
    while(!debPwmMot) {} debPwmMot = 0; // synchronisation avec le PWM

    /*CommandeGauche = CommandeDroite = 0;
    if(Pous1On) { CommandeGauche = 20000; Led8On; } else { Led8Off; }
    if(Pous2On) { CommandeGauche = -10000; Led7On; } else { Led7Off; }
    if(Pous3On) { CommandeDroite = 20000; Led6On; } else { Led6Off; }
    if(Pous4On) { CommandeDroite = -10000; Led5On; } else { Led5Off; }

    if(Pous5On) { CommandeGauche = CommandeDroite = 30000; Led4On; } else { Led4Off; }
    */
    FindCentre();
    AfficheLedBleues(Centre);
    CrossLine();

    uint32_t i; // capteurs 0 - 7
    for(i=0; i<8; i++) { Barre(ValCapteurs[i], i, YELLOW); }

    AfficheTension();
  }
}


