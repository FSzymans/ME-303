// Procedures utiles pour la carte 2026
// Version pour MSP432
// Pierre-Yves Rochat, 2020-26, EPFL, pyr@pyr.ch

#include <Carte2026-432.h>

void InitMoteurs(){

}

void InitCarte2026() {
  InitLed1;
  InitLed2;
  InitLed3;
  InitLed4;
  InitLed5;
  InitLed6;
  InitLed7;
  InitLed8;
  InitPous1;
  InitPous2;
  InitPous3;
  InitPous4;
  InitPous5;

  InitMoteurs();
}

void AfficheLedBleues(uint16_t val) {
  if (val & (1<<0)) { Led8On; } else { Led8Off; }
  if (val & (1<<1)) { Led7On; } else { Led7Off; }
  if (val & (1<<2)) { Led6On; } else { Led6Off; }
  if (val & (1<<3)) { Led5On; } else { Led5Off; }
  if (val & (1<<4)) { Led4On; } else { Led4Off; }
}

#define CPU_F ((double)8000000)
#define Delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

void delay_ms(unsigned int ms)
{
    while(ms)
    {
        Delay_ms(1);
        ms--;
    }
}
