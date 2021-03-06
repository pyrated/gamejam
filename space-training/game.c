#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <AL/al.h>

#include "bootstrap.h"
#include "tile.h"

static inline void render(GameState* gs);
int main(int argc, char** argv) {

  int i, j, k;

  boot_init(&argc, argv);

  GameState gs;

  // Load everything importante~ into the gamestate
  game_init(&gs);

  tileset_load(&gs, "level/1.bmp");

  glfwSetWindowTitle("Space Training");
  glClearColor(0.13725490196f, 0.25882352941f, 0.2f, 1.0f); // dark background

  float t0, t1, dt;
  t0 = glfwGetTime();
  // Game loop
  while (glfwGetWindowParam(GLFW_OPENED) == true) {
    t1 = glfwGetTime();
    dt = t1 - t0;
    t0 = t1;

    if (gs.frames > 60) {
      gs.fps = gs.frames / (t1 - gs.dt);
      printf("%.1lf fps\n", gs.fps);
      gs.dt = glfwGetTime();
      gs.frames = 0;
    }
    gs.frames++;
    
    if (glfwGetKey(GLFW_KEY_UP))
      gs.player_vy -= 200 * dt;
    if (glfwGetKey(GLFW_KEY_DOWN))
      gs.player_vy += 200 * dt;
    if (glfwGetKey(GLFW_KEY_LEFT))
      gs.player_vx -= 200 * dt;
    if (glfwGetKey(GLFW_KEY_RIGHT))
      gs.player_vx += 200 * dt;

    gs.player_vx = clampf(gs.player_vx, -400, 400);
    gs.player_vy = clampf(gs.player_vy, -400, 400);

    alSourcef(gs.player_source, AL_PITCH, (rand()%30)/10.0); 

    // Bouncy side walls
    if (gs.player_x < 0) {
      gs.player_x = 0;
      gs.player_vx = -gs.player_vx/2;
      alSourcei(gs.player_source, AL_BUFFER, gs.bounce_sound->buffer);
      alSourcePlay(gs.player_source);
    } else if (gs.player_x > 256*8-16) {
      gs.player_x = 256*8-16;
      gs.player_vx = -gs.player_vx/2;
      alSourcei(gs.player_source, AL_BUFFER, gs.bounce_sound->buffer);
      alSourcePlay(gs.player_source);
    }
    if (gs.player_y < 0) {
      gs.player_y = 0;
      gs.player_vy = -gs.player_vy/2;
      alSourcei(gs.player_source, AL_BUFFER, gs.bounce_sound->buffer);
      alSourcePlay(gs.player_source);
    } else if (gs.player_y > 256*8-16) {
      gs.player_y = 256*8-16;
      gs.player_vy = -gs.player_vy/2;
      alSourcei(gs.player_source, AL_BUFFER, gs.bounce_sound->buffer);
      alSourcePlay(gs.player_source);
    }

    float old_x = gs.player_x;
    float old_y = gs.player_y;

    gs.player_x += gs.player_vx * dt;

    char tile = TILE_NONE;

    for (j = 0; j < 3; j++) {
      for (i = 0; i < 3; i++) {
        tile = gs.tilemap[(int)((gs.player_x + (i * 8)) / 8)][(int)(gs.player_y / 8) + j];

        switch (tile) {
          case TILE_CAUTION:
            gs.player_x = old_x;
            alSourcei(gs.player_source, AL_BUFFER, gs.bounce_sound->buffer);
            alSourcePlay(gs.player_source);
            if (gs.player_vx < 0)
              gs.player_x = (int)(gs.player_x/8) * 8;
            else if (gs.player_vx > 0)
              gs.player_x = (int)(((gs.player_x + 16) / 8) * 8) - 16;

            gs.player_vx = -gs.player_vx/2;
            break;

          case TILE_SPIKE_UP ... TILE_SPIKE_RIGHT:
            alSourcei(gs.player_source, AL_BUFFER, gs.ouch_sound->buffer);
            alSourcePlay(gs.player_source);
            gs.player_x = old_x;
            gs.player_vx = -gs.player_vx;
            break;
        }
      }
    }

    gs.player_y += gs.player_vy * dt;

    for (j = 0; j < 3; j++) {
      for (i = 0; i < 3; i++) {
        tile = gs.tilemap[(int)(gs.player_x / 8) + j][(int)((gs.player_y + (i * 8)) / 8)];

        switch(tile) {
          case TILE_CAUTION:
            gs.player_y = old_y;
            alSourcei(gs.player_source, AL_BUFFER, gs.bounce_sound->buffer);
            alSourcePlay(gs.player_source);
            if (gs.player_vy < 0)
              gs.player_y = (int)(gs.player_y/8) * 8;
            else if (gs.player_vy > 0)
              gs.player_y = (int)(((gs.player_y + 16) / 8) * 8) - 16;

            gs.player_vy = -gs.player_vy/2;
            break;

          case TILE_SPIKE_UP ... TILE_SPIKE_RIGHT:
            alSourcei(gs.player_source, AL_BUFFER, gs.ouch_sound->buffer);
            alSourcePlay(gs.player_source);
            gs.player_y = old_y;
            gs.player_vy = -gs.player_vy;
            break;
        }
      }
    }

    // camera follow
    gs.cam_x = gs.player_x - 72;
    if (gs.cam_x < 0) gs.cam_x = 0;
    else if (gs.cam_x > 256*8-160) gs.cam_x = 256*8-160;

    gs.cam_y = gs.player_y - 64;
    if (gs.cam_y < 0) gs.cam_y = 0;
    else if (gs.cam_y > 256*8-144) gs.cam_y = 256*8-144;

    gs.player_frame += dt;

    for (k = 0; k < gs.num_entities; k++) {

      if (gs.ent[k].y < gs.player_y)
        gs.ent[k].vy += 200 * dt;
      else if (gs.ent[k].y > gs.player_y)
        gs.ent[k].vy -= 200 * dt;
      if (gs.ent[k].x < gs.player_x)
        gs.ent[k].vx += 200 * dt;
      else if (gs.ent[k].x > gs.player_x)
        gs.ent[k].vx -= 200 * dt;

      gs.ent[k].vx = clampf(gs.ent[k].vx, -400, 400);
      gs.ent[k].vy = clampf(gs.ent[k].vy, -400, 400);

      // Bouncy side walls
      if (gs.ent[k].x < 0) {
        gs.ent[k].x = 0;
        gs.ent[k].vx = -gs.ent[k].vx/2;
      } else if (gs.ent[k].x > 256*8-16) {
        gs.ent[k].x = 256*8-16;
        gs.ent[k].vx = -gs.ent[k].vx/2;
      }
      if (gs.ent[k].y < 0) {
        gs.ent[k].y = 0;
        gs.ent[k].vy = -gs.ent[k].vy/2;
      } else if (gs.ent[k].y > 256*8-16) {
        gs.ent[k].y = 256*8-16;
        gs.ent[k].vy = -gs.ent[k].vy/2;
      }

      old_x = gs.ent[k].x;
      old_y = gs.ent[k].y;

      gs.ent[k].x += gs.ent[k].vx * dt;

      for (j = 0; j < 3; j++) {
        for (i = 0; i < 3; i++) {
          tile = gs.tilemap[(int)((gs.ent[k].x + (i * 8)) / 8)][(int)(gs.ent[k].y / 8) + j];

          switch (tile) {
            case TILE_CAUTION:
              gs.ent[k].x = old_x;
              if (gs.ent[k].vx < 0)
                gs.ent[k].x = (int)(gs.ent[k].x/8) * 8;
              else if (gs.ent[k].vx > 0)
                gs.ent[k].x = (int)(((gs.ent[k].x + 16) / 8) * 8) - 16;

              gs.ent[k].vx = -gs.ent[k].vx/2;
              break;
          }
        }
      }

      gs.ent[k].y += gs.ent[k].vy * dt;

      for (j = 0; j < 3; j++) {
        for (i = 0; i < 3; i++) {
          tile = gs.tilemap[(int)(gs.ent[k].x / 8) + j][(int)((gs.ent[k].y + (i * 8)) / 8)];

          switch(tile) {
            case TILE_CAUTION:
              gs.ent[k].y = old_y;
              if (gs.ent[k].vy < 0)
                gs.ent[k].y = (int)(gs.ent[k].y/8) * 8;
              else if (gs.ent[k].vy > 0)
                gs.ent[k].y = (int)(((gs.ent[k].y + 16) / 8) * 8) - 16;

              gs.ent[k].vy = -gs.ent[k].vy/2;
              break;
          }
        }
      }

      gs.ent[k].frame += dt;
    }

    render(&gs);
  }
  
  boot_shutdown();

  return 0;
}

static inline void render(GameState* gs) {

  int i, j;

  // Clear screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(gs->tiles_program->id);

  // Bind the tiles to texture 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gs->tiles_sheet);
  glUniform1i(gs->tiles_program->uniform[1], 0);

  // Bind tiles vbo
  glBindBuffer(GL_ARRAY_BUFFER, gs->tiles_vbo);
  glEnableVertexAttribArray(gs->tiles_program->attribute[0]);
  glVertexAttribPointer(gs->tiles_program->attribute[0], 2, GL_FLOAT, false, 4*sizeof(float), (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(gs->tiles_program->attribute[1]);
  glVertexAttribPointer(gs->tiles_program->attribute[1], 2, GL_FLOAT, false, 4*sizeof(float), (void*)(2 * sizeof(float)));

  int t_x = gs->cam_x/8;
  int t_y = gs->cam_y/8;
  for (j = t_y; j < 19+t_y; j++) {
    for (i = t_x; i < 21+t_x; i++) {
      if (gs->tilemap[i][j] == TILE_NONE) continue;
      char tile = gs->tilemap[i][j];
      glUniform2f(gs->tiles_program->uniform[2], tile%16, tile/16);
      glUniform2f(gs->tiles_program->uniform[0], roundf(8*i-gs->cam_x), roundf(8*j-gs->cam_y));
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
  }

  glDisableVertexAttribArray(gs->tiles_program->attribute[0]);
  glDisableVertexAttribArray(gs->tiles_program->attribute[1]);

  glUseProgram(gs->player_program->id);

  glUniform2f(gs->player_program->uniform[0], roundf(gs->player_x-gs->cam_x), roundf(gs->player_y-gs->cam_y));

  // Bind the player to texture 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gs->player_spritesheet);
  glUniform1i(gs->player_program->uniform[1], 0);

  // Bind player vbo
  glBindBuffer(GL_ARRAY_BUFFER, gs->player_vbo);
  glEnableVertexAttribArray(gs->player_program->attribute[0]);
  glVertexAttribPointer(gs->player_program->attribute[0], 2, GL_FLOAT, false, 4*sizeof(float), (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(gs->player_program->attribute[1]);
  glVertexAttribPointer(gs->player_program->attribute[1], 2, GL_FLOAT, false, 4*sizeof(float), (void*)(2 * sizeof(float)));

  // set player direction
  glUniform1i(gs->player_program->uniform[2], (gs->player_vx > 0));

  glDrawArrays(GL_TRIANGLE_FAN, 4*((int)gs->player_frame % 2), 4);

  glDisableVertexAttribArray(gs->player_program->attribute[0]);
  glDisableVertexAttribArray(gs->player_program->attribute[1]);

  // Entities
  for (i = 0; i < gs->num_entities; i++) {

    glUseProgram(gs->ent[i].program->id);
    glUniform2f(gs->ent[i].program->uniform[0], roundf(gs->ent[i].x-gs->cam_x), roundf(gs->ent[i].y-gs->cam_y));

    // Bind the player to texture 0
    glBindTexture(GL_TEXTURE_2D, gs->ent[i].spritesheet);
    glUniform1i(gs->ent[i].program->uniform[1], 0);
  
    // Bind player vbo
    glBindBuffer(GL_ARRAY_BUFFER, gs->ent[i].vbo);
    glEnableVertexAttribArray(gs->player_program->attribute[0]);
    glVertexAttribPointer(gs->ent[i].program->attribute[0], 2, GL_FLOAT, false, 4*sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(gs->ent[i].program->attribute[1]);
    glVertexAttribPointer(gs->ent[i].program->attribute[1], 2, GL_FLOAT, false, 4*sizeof(float), (void*)(2 * sizeof(float)));

    glUniform1i(gs->player_program->uniform[2], (gs->ent[i].vx > 0));

    glDrawArrays(GL_TRIANGLE_FAN, 4*((int)gs->ent[i].frame % 2), 4);

    glDisableVertexAttribArray(gs->ent[i].program->attribute[0]);
    glDisableVertexAttribArray(gs->player_program->attribute[1]);
  }

  glfwSwapBuffers();
}