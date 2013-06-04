#ifndef __MAIN_H__
#define __MAIN_H__

#define C    261
#define Cis  277
#define D    293
#define Dis  311
#define E    329
#define F    349
#define Fis  369
#define G    391
#define Gis  415
#define A    440
#define Ais  466
#define H    493
#define B    350
#define Takt 1700
#define TTK 80


const uint16_t snd_tetris[][2]={
{E * 2, Takt / 4},
{H * 1, Takt / 8},
{C * 2, Takt / 8},
{D * 2, Takt / 4},
{C * 2, Takt / 8},
{H * 1, Takt / 8},
{A * 1, Takt / 4},
{A * 1, Takt / 8},
{C * 2, Takt / 8},
{E * 2, Takt / 8},
{E * 2, Takt / 8},
{D * 2, Takt / 8},
{C * 2, Takt / 8},
{H * 1, Takt / 2.5},
{C * 2, Takt / 8},
{D * 2, Takt / 4},
{E * 2, Takt / 4},
{C * 2, Takt / 4},
{A * 1, Takt / 4},
{0, Takt / (8 / 3)},
{D * 2, Takt / 3.25},
{F * 2, Takt / 8},
{Ais * 2, Takt / 4},
{G * 2, Takt / 8},
{F * 2, Takt / 8},
{E * 2, Takt / 3},
{C * 2, Takt / 8},
{E * 2, Takt / 8},
{E * 2, Takt / 8},
{D * 2, Takt / 8},
{C * 2, Takt / 8},
{H * 1, Takt / 4},
{H * 1, Takt / 8},
{C * 2, Takt / 8},
{D * 2, Takt / 4},
{E * 2, Takt / 4},
{C * 2, Takt / 4},
{A * 1, Takt / 4},
{A * 1, Takt / 4},
{2000,0}
};


void dus(unsigned int);
void beep(uint16_t, unsigned int);
void piep(const uint16_t [][2]);
void init_motor(void);
void deinit_motor(void);
void set_motor_speed(uint8_t);
int main(void);


#endif
