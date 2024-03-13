/*
 * DO NOT CHANGE THIS FILE!
 */

#include "searescue.h"
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void init_screen()
{
  initscr();

  start_color();
  init_pair(WATER, COLOR_BLUE, COLOR_WHITE);
  init_pair(PLAYER_COLOR, COLOR_WHITE, COLOR_BLACK);
  init_pair(SHARK_COLOR, COLOR_BLACK, COLOR_RED);
  init_pair(DIVER_COLOR, COLOR_BLACK, COLOR_MAGENTA);
  init_pair(DEBRI_COLOR, COLOR_WHITE, COLOR_BLACK);

  if (has_colors() != TRUE)
  {
    printf("ERROR: Your Screen does not support colors\n");
  }
  game_window = newwin(MAP_LENGTH, MAP_WIDTH, 0, 0);
}

void init_map()
{
  for (int y = 0; y < MAP_LENGTH; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      game_map[y][x] = (char)EMPTY;
    }
  }
}

char getMapValue(int x_pos, int y_pos)
{
  return game_map[y_pos][x_pos];
}

int isNearSurface(position pos)
{
  return pos.y_ < SURFACE_Y_LEVEL;
}

int diversCollected()
{
  return !divers_collected_flag && remaining_divers == 0;
}

void spawnEnemy(char type, int x_pos, int y_pos)
{
  if (type == SHARK)
  {
    game_map[y_pos][x_pos] = (char)SHARK;
  }
  else if (type == DEBRI)
  {
    game_map[y_pos][x_pos] = (char)DEBRI;
  }
}

void spawnDiver(int x_pos, int y_pos)
{
  game_map[y_pos][x_pos] = (char)DIVER;
}

void clearPosition(int x_pos, int y_pos) 
{
  game_map[y_pos][x_pos] = (char)EMPTY;
}

void refreshMap()
{
  werase(game_window);

  mvwprintw(game_window, 1, 1, "Searescue");
  mvwprintw(game_window, 1, 43, "v.0.1.0");
  mvwprintw(game_window, 2, 1, "POINTS: %07d\n", points);
  mvwprintw(game_window, 2, 20, "Oxygen: %04d\n", oxygen);

  for (int y = 3; y < MAP_LENGTH; ++y)
  {
    for (int x = 0; x < MAP_WIDTH; ++x)
    {
      if (game_map[y][x] != EMPTY)
      {
        if (game_map[y][x] == PLAYER)
        {
          wattron(game_window, COLOR_PAIR(PLAYER_COLOR));
          mvwaddch(game_window, y, x, 'P');
          wattroff(game_window, COLOR_PAIR(PLAYER_COLOR));
        }
        else if (game_map[y][x] == SHARK)
        {
          wattron(game_window, COLOR_PAIR(SHARK_COLOR));
          mvwaddch(game_window, y, x, 'S');
          wattroff(game_window, COLOR_PAIR(SHARK_COLOR));
        }
        else if (game_map[y][x] == DIVER)
        {
          wattron(game_window, COLOR_PAIR(DIVER_COLOR));
          mvwaddch(game_window, y, x, 'D');
          wattroff(game_window, COLOR_PAIR(DIVER_COLOR));
        }
        else if (game_map[y][x] == DEBRI)
        {
          wattron(game_window, COLOR_PAIR(DEBRI_COLOR));
          mvwaddch(game_window, y, x, 'M');
          wattroff(game_window, COLOR_PAIR(DEBRI_COLOR));
        }
      }
      else
      {
        wattron(game_window, COLOR_PAIR(WATER));
        mvwaddch(game_window, y, x, '\0');
        wattroff(game_window, COLOR_PAIR(WATER));
      }
    }
  }
  wrefresh(game_window);
}

char getRandPosX()
{
  return rand() % 30 + 5;
}

char getRandPosY()
{
  return rand() % 30 + 5;
}

void movePlayer(char *direction, char *prev_direction, position *pos, char type)
{
  switch (*direction)
  {
  case 'l':
    if (pos->x_ < 1)
    {
      if (type == PLAYER) 
      {
        lifepoints = 0;
        break;
      }
      else
      {
        // removing person
        game_map[pos->y_][pos->x_] = (char)EMPTY;

        // revoking on the opposite site of the map
        pos->x_ = MAP_WIDTH - 1;
        game_map[pos->y_][pos->x_] = (char)type;

        break;
      }
    }
    if (type == PLAYER)
    {
      if (game_map[pos->y_][pos->x_ - 1] == (char)DEBRI ||
          game_map[pos->y_][pos->x_ - 1] == (char)SHARK)
      {
        lifepoints = 0;
        break;
      }
    }
    game_map[pos->y_][pos->x_] = (char)EMPTY;
    game_map[pos->y_][--pos->x_] = (char)type;
    break;

  case 'r':
    if (pos->x_ > MAP_WIDTH - 2)
    {
      if (type == PLAYER) 
      {
        lifepoints = 0;
        break;
      }
      else
      {
        // removing person
        game_map[pos->y_][pos->x_] = (char)EMPTY;

        // revoking on the opposite site of the map
        pos->x_ = 0;
        game_map[pos->y_][pos->x_] = (char)type;

        break;
      }
    }

    if (type == PLAYER)
    {
      if (game_map[pos->y_][pos->x_ + 1] == (char)DEBRI ||
          game_map[pos->y_][pos->x_ + 1] == (char)SHARK)
      {
        lifepoints = 0;
        break;
      }
    }
    game_map[pos->y_][pos->x_] = (char)EMPTY;
    game_map[pos->y_][++pos->x_] = (char)type;
    break;

  case 'u':
    if (pos->y_ < 4)
    {
      if (type == PLAYER) 
      {
        lifepoints = 0;
        break;
      }
      else
      {
        // removing person
        game_map[pos->y_][pos->x_] = (char)EMPTY;

        // revoking on the opposite site of the map
        pos->y_ = MAP_LENGTH - 3;
        game_map[pos->y_][pos->x_] = (char)type;

        break;
      }
    }

    if (type == PLAYER)
    {
      if (game_map[pos->y_ - 1][pos->x_] == (char)DEBRI ||
          game_map[pos->y_ - 1][pos->x_] == (char)SHARK)
      {
        lifepoints = 0;
        break;
      }
    }
    game_map[pos->y_][pos->x_] = (char)EMPTY;
    game_map[--pos->y_][pos->x_] = (char)type;
    break;

  case 'd':
    if (pos->y_ > MAP_LENGTH - 2)
    {
      if (type == PLAYER) 
      {
        lifepoints = 0;
        break;
      }
      else
      {
        // removing person
        game_map[pos->y_][pos->x_] = (char)EMPTY;

        // revoking on the opposite site of the map
        pos->y_ = 3;
        game_map[pos->y_][pos->x_] = (char)type;

        break;
      }
    }

    if (type == PLAYER)
    {
      if (game_map[pos->y_ + 1][pos->x_] == (char)DEBRI ||
          game_map[pos->y_ + 1][pos->x_] == (char)SHARK)
      {
        lifepoints = 0;
        break;
      }
    }
    game_map[pos->y_][pos->x_] = (char)EMPTY;
    game_map[++pos->y_][pos->x_] = (char)type;
    break;

  default:
    break;
  }
  *prev_direction = *direction;
}

int end_game(void *rvalue, void **rvalues_enemies, void **rvalues_divers, void *rvalue_oxygen)
{
  if (game_window)
  {
    mvwprintw(game_window, (MAP_LENGTH / 2), 16, "    GAME OVER    ");
    mvwprintw(game_window, (MAP_LENGTH / 2 + 1), 16, " %08d POINTS ", points);
    wrefresh(game_window);
    usleep(5000000);
    wborder(game_window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wclear(game_window);
    wrefresh(game_window);
    delwin(game_window);
    endwin();
  } 

  free(rvalues_enemies);
  free(rvalues_divers);

  return 0;
}
