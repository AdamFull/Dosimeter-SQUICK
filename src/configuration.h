#pragma once
#include <avr/pgmspace.h>

#define en 0
#define ru 1

//Выбор языка, en - английский, ru -  русский
//Русский язык занимает RAM: 92 байт Flash: 118 байт
#define LANGUAGE ru
#ifndef LANGUAGE
#define LANGUAGE en
#endif

#if LANGUAGE == ru
#define RUS
#else
#define EN
#endif

//Вывод логотипа при включении
//Занимает RAM: 0 байт Flash: 386 байт
//Раскоментировать чтобы выводить логотип
#define SHOW_LOGO

//Время показа логотипа в мс
#if defined(SHOW_LOGO)
#define LOGO_TIME 1000
#endif

//Продвинутый алгоритм расчёта погрешности
//Занимает RAM: 12 байт Flash: 1390 байт
//Раскомментировать, чтобы включить улучшеный алгоритм расчёта погрешности
#define ADVANCED_ERROR          

//Режим сна. Заснуть можно в меню. Выход из сна долгое удержание кнопки set
//Занимает RAM: 42 байт Flash: 456 байт
//Раскоментировать чтобы включить
#define CAN_SLEEP

//Отрисовка графика, раскоментировать чтобы включить
//Занимает RAM: 85 байт Flash: 146 байт
#define DRAW_GRAPH

#if defined(DRAW_GRAPH)
//Константа для графика
#define RAD_MIN 0
#endif

//Универсальная плата. Позволяет выбрать счётчик гейгера из пресетов, или настроить свой
//Имеет некоторые ограничения, т.к. занимает много памяти и может вызвать ошибки при
//выставлении большого значения времени счёта
//Занимает RAM: 49 байт Flash: 1988 байт
//Раскомментировать чтобы включить
#define UNIVERSAL_COUNTER

//Если прибор не универсален, то устанавливаем время счёта вручную
#if !defined(UNIVERSAL_COUNTER)
#define GEIGER_TIME 37
#endif

#if defined(UNIVERSAL_COUNTER) && defined(CAN_SLEEP)
#define PAGES 8
#elif defined(UNIVERSAL_COUNTER) && !defined(CAN_SLEEP)
#define PAGES 7
#elif !defined(UNIVERSAL_COUNTER) && defined(CAN_SLEEP)
#define PAGES 6
#else
#define PAGES 5
#endif


#define TIMER1_PRELOAD 64910 //65535-64910=625, 15625/625=25Гц
#define HVGEN_FACT 5 // 25/5=5Гц частота подкачки преобразователя
#define TIME_FACT 25 // 25Гц/25=1Гц секундные интервалы

#if defined(SHOW_LOGO)
static const unsigned char PROGMEM logo_Bitmap[] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x80, 0x88, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x80, 0x1e, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xc0, 0x78, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x0c, 0x7f, 
	0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x0c, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0xf0, 0x88, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0xf8, 
	0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xfc, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 
	0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x07, 0xff, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x03, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 
	0xf8, 0xf1, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x20, 0xff, 0xfc, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x20, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
	0xff, 0xf0, 0x20, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3c, 0x00, 0x20, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x00, 0x20, 0x09, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0xf0, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xc1, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0x01, 0xfc, 
	0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0x03, 0xfc, 0x0a, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x43, 0x07, 0xfe, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x07, 
	0x07, 0x09, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0e, 0x67, 0x08, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0c, 0x1c, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
	0x1c, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xff, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7c, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x67, 0xf0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

static const unsigned char PROGMEM battery_Bitmap[] = {
	0x7f, 0xfe, 0xc0, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0xc0, 0x02, 0x7f, 0xfe
};

#if defined(ADVANCED_ERROR)
// делить на 100
const uint16_t t_quantile[52][6] PROGMEM= {{196, 307, 631, 127, 3182, 6365}, //1
                                          {138, 188, 292, 430, 696, 992}, //2
                                          {124, 163, 235, 318, 454, 584}, //3
                                          {118, 153, 213, 277, 374, 460}, //4
                                          {115, 147, 201, 257, 336, 403}, //5
                                          {113, 143, 194, 244, 314, 370}, //6
                                          {111, 141, 189, 236, 299, 349}, //7
                                          {110, 139, 185, 230, 289, 335}, //8
                                          {109, 138, 183, 226, 282, 324}, //9
                                          {109, 137, 181, 222, 276, 316}, //10
                                          {108, 136, 179, 220 ,271, 310},//11
                                          {108, 135, 178, 217, 268, 305},//12
                                          {107, 135, 177, 216, 265, 301},//13
                                          {107, 134, 176, 214, 262, 297},//14
                                          {107, 134, 175, 213, 260, 294},//15
                                          {107, 133, 174, 211, 258, 292},//16
                                          {106, 133, 173, 210, 256, 289},//17
                                          {106, 133, 173, 210, 255, 287},//18
                                          {106, 132, 172, 209, 253, 286},//19
                                          {106, 132, 172, 208, 252, 284},//20
                                          {106, 132, 172, 207, 251, 283},//21
                                          {106, 132, 171, 207, 250, 281},//22
                                          {106, 131, 171, 206, 249, 280},//23
                                          {105, 131, 171, 206, 249, 279},//24
                                          {105, 131, 170, 205, 248, 278},//25
                                          {105, 131, 170, 205, 247, 277},//26
                                          {105, 131, 170, 205, 247, 277},//27
                                          {105, 131, 170, 204, 246, 276},//28
                                          {105, 131, 169, 204, 246, 275},//29
                                          {105, 131, 169, 204, 245, 275},//30
                                          {105, 130, 169, 203, 245, 274},//31
                                          {105, 130, 169, 203, 244, 273},//32
                                          {105, 130, 169, 203, 244, 273},//33
                                          {105, 130, 169, 203, 244, 272},//34
                                          {105, 130, 168, 203, 243, 272},//35
                                          {105, 130, 168, 202, 243, 271},//36
                                          {105, 130, 168, 202, 243, 271},//37
                                          {105, 130, 168, 202, 242, 271},//38
                                          {105, 130, 168, 202, 242, 270},//39
                                          {105, 130, 168, 202, 242, 270},//40
                                          {104, 130, 168, 201, 242, 270},//41
                                          {104, 130, 168, 201, 241, 269},//42
                                          {104, 130, 168, 201, 241, 269},//43
                                          {104, 130, 168, 201, 241, 269},//44
                                          {104, 130, 167, 201, 241, 268},//45
                                          {104, 130, 167, 201, 241, 268},//46
                                          {104, 129, 167, 201, 240, 268},//47
                                          {104, 129, 167, 201, 240, 268},//48
                                          {104, 129, 167, 200, 240, 268},//49
                                          {104, 129, 167, 200, 240, 267},//50
                                          {104, 129, 166, 198, 236, 262},//100
                                          {103, 128, 164, 196, 233, 258}};//1000
#endif