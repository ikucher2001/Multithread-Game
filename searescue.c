/*
 * Implement the missing functionality in this file. DO NOT CHANGE ANY OTHER FILE!
 * Add your code only inside the given TODO borders
 */

#include "searescue.h"
#include <stdlib.h>

WINDOW *game_window;
char game_map[MAP_LENGTH][MAP_WIDTH];

// поток для игрока. игрок 1
pthread_t player_tid;
// масси потоков для акул. придется работать с памятью
pthread_t *shark_tids;
// массив для мусора
pthread_t *debri_tids;
// массив для водолазов. понять бы еще, что они делают
pthread_t *diver_tids;
// массив для кислорода?
pthread_t oxygen_tid;

void *rvalue_player = 0;
void *rvalue_oxygen = 0;
void **rvalue_enemies;
void **rvalue_divers;

time_t time1;

position player_position = {MAP_WIDTH / 2, MAP_LENGTH / 2};

int lifepoints = 100;
int points = 0;
int oxygen = 100;
int divers_collected = 0;
int remaining_divers = 0;
int divers_collected_flag = TRUE;

int number_sharks = 7;
int number_divers = 3;
int number_debris = 5;

int OXYGEN_DEPLETION_RATE = 1;
int OXYGEN_REPLENISHMENT_RATE = 5;
int OXYGEN_MAX_CAPACITY = 100;

void *playerLogic()
{
  game_map[player_position.y_][player_position.x_] = (char)PLAYER;

  char direction = 'l';
  char prev_direction = 'l';

  // player controls
  int c;
  keypad(stdscr, TRUE);
  noecho();
  timeout(1);
  while (true)
  {
    c = getch();
    switch (c)
    {
    case 's':
      direction = 'd';
      break;
    case 'w':
      direction = 'u';
      break;
    case 'a':
      direction = 'l';
      break;
    case 'd':
      direction = 'r';
      break;
    default:
      break;
    }
    if (c == 'q' || oxygen <= 0)
    {
      lifepoints = 0;
      continue;
    }

    usleep(100000);

    // TODO (8):
    // - If the player has collected ALL divers and is near the surface,
    // - the divers are picked up by a rescue boat. (Hint: isNearSurface(), player_position, diversCollected())
    // - Once the divers are rescued by the boat the score should increase for every diver and
    // - their corresponding threads should somehow terminate
    // - also set divers_collected_flag to 1

    if (isNearSurface(player_position) && diversCollected())
    {

      for (int i = 0; i < number_divers; i++)
      {
        pthread_cancel(diver_tids[i]);

        if (divers_collected >= number_divers)
        {
          //sizeof(void *) * (number_divers)
          rvalue_divers = realloc(rvalue_divers, sizeof(void *) * (divers_collected + 1));
          if (rvalue_divers == NULL)
          {
            free(rvalue_divers);
          }
        }

        void *rval;

        pthread_join(diver_tids[i], &rval);
        rvalue_divers[divers_collected] = rval;

        divers_collected++;
        points = points + POINTS_DIVER;
      }
      divers_collected_flag = TRUE;
    }

    // TODO END

    movePlayer(&direction, &prev_direction, &player_position, PLAYER);
  }
}

void *oxygenLogic()
{
  while (lifepoints > 0)
  {
    usleep(100000);

    // TODO (2):
    // Implement the oxygen replenishment logic
    // If the player is near the surface the current amount of oxygen should increase by the specified replenishment rate.
    // else the current oxygen level should decrease by the depletion rate.  (Hint: isNearSurface)
    // Make sure that the oxygen level doesnt exceed the maximum capacity or decreases below zero.

    if (!isNearSurface(player_position))
    {
      int oxygen_to_be = oxygen - OXYGEN_DEPLETION_RATE;
      if (oxygen_to_be <= 0)
      {
        oxygen = 0;
      }
      else
      {
        oxygen = oxygen_to_be;
      }
    }
    else
    {
      int oxygen_to_be = oxygen + OXYGEN_REPLENISHMENT_RATE;
      if (oxygen_to_be > OXYGEN_MAX_CAPACITY)
      {
        oxygen = OXYGEN_MAX_CAPACITY;
      }
      else
      {
        oxygen = oxygen_to_be;
      }
    }
    // TODO END
  }

  return (void *)777;
}

void *enemyDebri(parameters *params)
{
  if (params->type_ != DEBRI)
    return (void *)-1;

  // TODO (4):
  // Spawn the debri correctly in the map. The given position gives the position of the top left field.
  // Make sure to initialize the other fields correctly!
  // spawnEnemy *only* initializes *one* field on the map.

  unsigned char position_x = params->pos_x_;
  unsigned char position_y = params->pos_y_;
  free(params);

  while (lifepoints > 0)
  {
    usleep(300000);
    spawnEnemy(DEBRI, position_x, position_y);
    spawnEnemy(DEBRI, position_x + 1, position_y);
    spawnEnemy(DEBRI, position_x, position_y + 1);
    spawnEnemy(DEBRI, position_x + 1, position_y + 1);
  }
  // TODO END

  return (void *)999;
}

void *enemyShark(parameters *params)
{
  if (params->type_ != SHARK)
    return (void *)-1;

  // TODO (5):
  // -) set the shark position correctly
  // -) spawn the enemy shark correctly in the map

  position enemy_pos = {params->pos_x_, params->pos_y_};
  free(params);
  spawnEnemy(SHARK, enemy_pos.x_, enemy_pos.y_);
  // TODO END

  int nr_direction = 0;
  char prev_direction = 'l';

  while (lifepoints > 0)
  {
    usleep(300000);

    if (rand() % 5 == 0)
    {
      nr_direction = rand() % 4;
    }
    char direction;
    switch (nr_direction)
    {
    case 0:
      direction = 'l';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)SHARK);
      break;
    case 1:
      direction = 'r';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)SHARK);
      break;
    case 2:
      direction = 'u';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)SHARK);
      break;
    case 3:
      direction = 'd';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)SHARK);
      break;
    default:
      break;
    }
  }
  return (void *)999;
}

void init_enemies(unsigned char type, int number_of_enemy_type)
{
  pthread_attr_t enemy_attr;
  pthread_attr_init(&enemy_attr);

  // TODO (3): Implement the whole function:
  // - spawn all enemies of the given type (one thread per entity), make sure each one appears one it's random coordinates and has the type specified in type_
  // - use the attribute for thread creation
  // - use the shark_tids and debri_tid arrays to store the thread IDs in them
  // - all enemies have to move on their own and concurrently with the others!
  // - look at the provided data structures in the header. Use them!
  // - make sure to get a new random position for each enemy entity!
  // - Arguments needed? generate them via getRandPosX() and getRandPosY()
  // - pay attention to not call the getRandPos-functions unnecessary or more often then you need, since this will fail on the testsystem
  // - Furthermore, make sure to keep the lifetime of variables in mind and to not leak any memory!

  // It is ok for sharks and debris to overlap! It is also ok to have race conditions in your code. The testsystem will not test for this
  // and you will learn how to fix them in A2!

  pthread_attr_setdetachstate(&enemy_attr, PTHREAD_CREATE_DETACHED);

  // TODO start from here!
  pthread_attr_setdetachstate(&enemy_attr, PTHREAD_CREATE_JOINABLE);
  for (int i = 0; i < number_of_enemy_type; i++)
  {
    parameters *params = malloc(sizeof(parameters));

    params->pos_x_ = getRandPosX();
    params->pos_y_ = getRandPosY();
    params->type_ = type;

    if (type == SHARK)
    {
      pthread_t shark_t;
      pthread_create(&shark_t, &enemy_attr, (void *)&enemyShark, params);
      shark_tids[i] = shark_t;
    }
    else if (type == DEBRI)
    {
      pthread_t debry_t;
      pthread_create(&debry_t, &enemy_attr, (void *)&enemyDebri, params);
      debri_tids[i] = debry_t;
    }
  }

  // TODO END

  pthread_attr_destroy(&enemy_attr);
}

void *diver(parameters *params)
{
  if (params->type_ != DIVER)
    return (void *)-1;

  // TODO (7):
  // - In general there should be NUMBER_DIVERS divers on the map at a time
  // - place the diver on the position taken from the function parameters
  // - the diver is only drawn on the map as long as it was not collected by the
  // - player. The diver threads should terminate IMMEDIATELY once the player
  // - reaches the surface with all divers on board. Think about what mechanism/characteristic which could be used for this.

  unsigned char position_x = params->pos_x_;
  unsigned char position_y = params->pos_y_;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  free(params);

  // TODO END
  int diver_collected = FALSE;

  while (1)
  {

    if (player_position.x_ == position_x && player_position.y_ == position_y && !diver_collected)
    {
      diver_collected = true;
      remaining_divers = remaining_divers - 1;
      clearPosition(position_x, position_y);
    }

    if (!diver_collected)
    {
      spawnDiver(position_x, position_y);
    }
  }
}

int start_game(int argc, char *argv[])
{
  init_screen();
  srand((unsigned int)time(&time1));
  diver_tids = malloc(sizeof(pthread_t) * number_divers);
  shark_tids = malloc(sizeof(pthread_t) * number_sharks);
  debri_tids = malloc(sizeof(pthread_t) * number_debris);

  rvalue_enemies = malloc(sizeof(void *) * (number_debris + number_sharks));
  rvalue_divers = malloc(sizeof(void *) * (number_divers));
  memset(rvalue_enemies, 0, sizeof(void *) * (number_debris + number_sharks));
  memset(rvalue_divers, 0, sizeof(void *) * number_divers);

  init_map();

  // TODO (1):
  // - spawn the player*, oxygen refueling* and enemies by creating separate threads for each entity
  // - Arguments needed? Attributes needed? Look at the playerLogic, oxygenLogic and the enemy functions
  // - look at the pthread_t variables from above... USE them!!
  // - Init all sharks with one call to init_enemies and all debris with another call to init_enemies
  // - Generate number_debris debris and number_sharks shark enemies
  // - Make sure to generate all sharks first, then all debris

  // spawn the player. player is one, so we dont need loop for it
  pthread_create(&player_tid, NULL, (void *)&playerLogic, NULL);
  pthread_create(&oxygen_tid, NULL, (void *)&oxygenLogic, NULL);

  init_enemies(SHARK, number_sharks);
  init_enemies(DEBRI, number_debris);
  // TODO END

  refreshMap();

  // TODO (6):
  // - spawn the initial divers by creating diver threads (for the divers position call getRandPosX() and getRandPosY())
  // - if all the divers are collected and returned to the surface (there's a flag for that),
  // - new divers shall be spawned at new random positions
  // - pay attention to not call the getRandPos-functions unnecessary or more often then you need, since this will fail on the testsystem
  // - make sure that all diver threads are terminated and store the return value in rvalue_divers - Do this check for termination in this TODO
  // - have a look at the other TODOs from above - there might be interconnects
  // - Arguments needed? generate them via getRandPosX() and getRandPosY()

  for (int i = 0; i < number_divers; i++)
  {
    pthread_t diver_t;
    parameters *params = malloc(sizeof(parameters));

    if (params == NULL)
    {
      free(params);
    }
    else
    {
      params->pos_x_ = getRandPosX();
      params->pos_y_ = getRandPosY();
      params->type_ = DIVER;
      remaining_divers++;

      pthread_create(&diver_t, NULL, (void *)&diver, params);
      diver_tids[i] = diver_t;
    }
  }

  // TODO END
  divers_collected_flag = FALSE;

  while (lifepoints > 0)
  {
    usleep(10000);

    if (divers_collected_flag)
    {
      // TODO (9): Further implementations from TODO above
      for (int i = 0; i < number_divers; i++)
      {
        pthread_t diver_t;
        parameters *params = malloc(sizeof(parameters));

        if (params == NULL)
        {
          free(params);
        }
        else
        {
          params->pos_x_ = getRandPosX();
          params->pos_y_ = getRandPosY();
          params->type_ = DIVER;
          remaining_divers++;

          pthread_create(&diver_t, NULL, (void *)&diver, params);
          diver_tids[i] = diver_t;
        }
      }

      // TODO END

      divers_collected_flag = FALSE;
    }

    refreshMap();
  }

  // TODO (10):
  // - make sure that all the running threads are terminated before returning from main and fetch their return values
  // - All threads which are able to return on their own should also do that so and therefore give back the corresponding return value
  // - Force the termination of threads which do not terminate on their own.
  // - have a closer look on the other TODOs. There might be some connections
  // - we want to make sure, that all threads are terminated for sure. But are all threads even joinable?

  int enemies_cursor = 0;
  for (int i = 0; i < number_sharks; i++)
  {
    void *rval;
    pthread_join(shark_tids[i], &rval);
    rvalue_enemies[enemies_cursor] = rval;
    enemies_cursor++;
  }
  for (int i = 0; i < number_debris; i++)
  {
    void *rval;
    pthread_join(debri_tids[i], &rval);
    rvalue_enemies[enemies_cursor] = rval;
    enemies_cursor++;
  }

  for (int i = 0; i < number_divers; i++)
  {
    pthread_cancel(diver_tids[i]);
    void *rval;
    pthread_join(diver_tids[i], &rval);
    rvalue_divers[i] = rval;
  }

  pthread_cancel(player_tid);
  void *rval_p;
  pthread_join(player_tid, &rval_p);
  rvalue_player = rval_p;

  void *rval_o;
  pthread_join(oxygen_tid, &rval_o);
  rvalue_oxygen = rval_o;
  // TODO END
  free(shark_tids);
  free(diver_tids);
  free(debri_tids);


  return end_game((void **)rvalue_player, (void *)rvalue_enemies, (void *)rvalue_divers, (void **)rvalue_oxygen);
}