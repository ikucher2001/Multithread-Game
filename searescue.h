/*
 * DO NOT CHANGE THIS FILE!
 */

#include <ncurses.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "string.h"
#include <time.h>

// Map geometry
#define MAP_WIDTH 50
#define MAP_LENGTH 50

// Field Types
#define EMPTY 1
#define PLAYER 2
#define DIVER 3
#define OXYGEN 4
#define SHARK 6
#define DEBRI 7

// ncurses constants
#define WATER 51
#define PLAYER_COLOR 52
#define SHARK_COLOR 53
#define DIVER_COLOR 54
#define DEBRI_COLOR 55

// Game settings
#define POINTS_DIVER 100
#define SURFACE_Y_LEVEL 8

extern int OXYGEN_DEPLETION_RATE;
extern int OXYGEN_REPLENISHMENT_RATE;
extern int OXYGEN_MAX_CAPACITY;

#define INVALID_TID ((pthread_t)(-1))

#ifndef SNP_TUTOR_TEST_1_H
#define SNP_TUTOR_TEST_1_H

typedef struct
{
  unsigned char x_;
  unsigned char y_;
} position;

typedef struct
{
  unsigned char pos_x_;
  unsigned char pos_y_;
  unsigned char type_;
} parameters;


extern WINDOW *game_window;
extern char game_map[MAP_LENGTH][MAP_WIDTH];

extern int lifepoints;
extern int points;
extern int oxygen;
extern int number_sharks;
extern int number_debris;
extern int number_divers;
extern pthread_t* shark_tids;
extern pthread_t* debri_tids;
extern position player_position;
extern pthread_t player_tid;
extern pthread_t oxygen_tid;
extern pthread_t* diver_tids;
extern time_t time1;
extern int divers_collected_flag;
extern int remaining_divers;

extern void *rvalue_player;
extern void **rvalue_enemies;
extern void **rvalue_divers;
extern void *rvalue_oxygen;

void init_map();
void init_screen();

void spawnDiver(int x_pos, int y_pos);
void spawnEnemy(char type, int x_pos, int y_pos);
void clearPosition(int x_pos, int y_pos);
char getMapValue(int x_pos, int y_pos);
int isNearSurface(position position);
int diversCollected();

void refreshMap();

int start_game(int argc, char* argv[]);
int end_game(void *rvalue, void **rvalues_enemies, void **rvalues_diver, void *rvalue_oxygen);

void *playerLogic();
void* oxygenLogic();

void *enemyShark(parameters *params);
void *enemyDebri(parameters *params);

void *diver(parameters *params);

void init_enemies(unsigned char type, int number_of_enemy_type);

void movePlayer(char *direction, char *prev_direction, position *pos, char type);

char getRandPosX();

char getRandPosY();

#endif // SNP_TUTOR_TEST_1_H
