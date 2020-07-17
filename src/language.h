#pragma once
#include <configuration.h>
#include <avr/pgmspace.h>

#if defined(RUS)
static PGM_P const MENU PROGMEM = "Meню";
static PGM_P const MODE PROGMEM = "Peжим";
static PGM_P const SETTINGS PROGMEM = "Hacтpoйkи";
static PGM_P const RESET PROGMEM = "Cбpoc";
static PGM_P const BACKGROUND PROGMEM = "Поиск";
static PGM_P const SAMPLE PROGMEM = "Oбpaзeц";
static PGM_P const PRESSSET PROGMEM = "Haжми ceт";
static PGM_P const SUCCESS PROGMEM = "Гoтoвo";
static PGM_P const ACTIVITY PROGMEM = "Akтивнocть";
static PGM_P const TONE PROGMEM = "Toнaлн:";
static PGM_P const BACKLIGHT PROGMEM = "Пoдcвeтka:";
static PGM_P const CONTRAST PROGMEM = "Koнтpacт:";
static PGM_P const DOSE_SAVE PROGMEM = "Coxp. дoз:";
static PGM_P const YES PROGMEM = "Дa";
static PGM_P const NO PROGMEM = "Heт";
static PGM_P const DOSE PROGMEM = "Дoзa";
static PGM_P const ALL PROGMEM = "Bce";
static PGM_P const TIME PROGMEM = "Bpeмя:";
static PGM_P const MEANS PROGMEM = "Измepeний:";
static PGM_P const BEGIN PROGMEM = "Haчaть";
static PGM_P const MODE_SEC PROGMEM = "Секунда";
static PGM_P const ALARM PROGMEM = "Tpeвoгa:";
#if defined(UNIVERSAL_COUNTER)
static PGM_P const GCOUNTER PROGMEM = "Cчeтчиk";
static PGM_P const CUSTOM PROGMEM = "Cвoй";
static PGM_P const VOLTAGE PROGMEM = "Haпpяжeн:";
static PGM_P const GTIME PROGMEM = "Bpемя изм:";
static PGM_P const ERROR PROGMEM = "Oшибka:";
#endif
#if defined(CAN_SLEEP)
static PGM_P const POFF PROGMEM = "Coн";
static PGM_P const SURE PROGMEM = "Увepeны?";
#endif

#else
static PGM_P const MENU PROGMEM = "Menu";
static PGM_P const MODE PROGMEM = "Mode";
static PGM_P const SETTINGS PROGMEM = "Settings";
static PGM_P const RESET PROGMEM = "Reset";
static PGM_P const BACKGROUND PROGMEM = "Background";
static PGM_P const SAMPLE PROGMEM = "sample";
static PGM_P const PRESSSET PROGMEM = "Press set";
static PGM_P const SUCCESS PROGMEM = "Ready";
static PGM_P const ACTIVITY PROGMEM = "Activity";
static PGM_P const TONE PROGMEM = "Tone:";
static PGM_P const BACKLIGHT PROGMEM = "Backlight:";
static PGM_P const CONTRAST PROGMEM = "Contrast:";
static PGM_P const DOSE_SAVE PROGMEM = "Sav. dose:";
static PGM_P const YES PROGMEM = "Yes";
static PGM_P const NO PROGMEM = "No";
static PGM_P const DOSE PROGMEM = "Dose";
static PGM_P const ALL PROGMEM = "All";
static PGM_P const TIME PROGMEM = "Time:";
static PGM_P const MEANS PROGMEM = "Meansure:";
static PGM_P const BEGIN PROGMEM = "Begin";
static PGM_P const MODE_SEC PROGMEM = "One sec";
#if defined(UNIVERSAL_COUNTER)
static PGM_P const GCOUNTER PROGMEM = "Counter";
static PGM_P const CUSTOM PROGMEM = "Custom";
static PGM_P const VOLTAGE PROGMEM = "Voltage:";
static PGM_P const GTIME PROGMEM = "Means time:";
static PGM_P const ERROR PROGMEM = "Error:";
#endif
#if defined(CAN_SLEEP)
static PGM_P const POFF PROGMEM = "Sleep";
static PGM_P const SURE PROGMEM = "Sure?";
#endif
#endif

static PGM_P const T_URH PROGMEM = "uR/h";
static PGM_P const T_MRH PROGMEM = "mR/h";
static PGM_P const T_RH PROGMEM = "R/h";
static PGM_P const T_UR PROGMEM = "uR";
static PGM_P const T_MR PROGMEM = "mR";
static PGM_P const T_R PROGMEM = "R";
static PGM_P const T_CPS PROGMEM = "cps";
static PGM_P const T_MAX PROGMEM = "max";
static PGM_P const T_OLD PROGMEM = "old";

static PGM_P const T_CURSOR PROGMEM = ">";

#if defined(UNIVERSAL_COUNTER)
static PGM_P const G_SBM20 PROGMEM = "SBM-20/STS-5";
static PGM_P const G_SBM19 PROGMEM = "SBM-19/STS-6";
static PGM_P const G_BETA PROGMEM = "BETA-1/BETA-2";
#endif

static PGM_P const current_page_name[PAGES] = {MENU, MODE, SETTINGS, RESET, ACTIVITY
#if defined(CAN_SLEEP)
, SURE
#endif
#if defined(UNIVERSAL_COUNTER)
, GCOUNTER, CUSTOM
#endif
};