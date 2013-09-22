/* ============================================================================
 *  Controller.c: Video controller.
 *
 *  VIDEOSIM: VIDEO Interface SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Address.h"
#include "Common.h"
#include "Controller.h"
#include "Definitions.h"
#include "Externs.h"
#include "Render.h"

#ifdef __cplusplus
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <GL/glfw.h>

static void InitVIF(struct VIFController *);

/* ============================================================================
 *  Mnemonics table.
 * ========================================================================= */
#ifndef NDEBUG
const char *VIRegisterMnemonics[NUM_VI_REGISTERS] = {
#define X(reg) #reg,
#include "Registers.md"
#undef X
};
#endif

/* ============================================================================
 *  ConnectVIFToBus: Connects a VIF instance to a Bus instance.
 * ========================================================================= */
void
ConnectVIFToBus(struct VIFController *controller, struct BusController *bus) {
  controller->bus = bus;
}

/* ============================================================================
 *  CreateVIF: Creates and initializes an VIF instance.
 * ========================================================================= */
struct VIFController *
CreateVIF(void) {
  struct VIFController *controller;

  /* Disable everything. */
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);

  /* Allocate memory for controller. */
  if ((controller = (struct VIFController*) malloc(
    sizeof(struct VIFController))) == NULL) {
    return NULL;
  }

  /* Generate frame texture. */
  glEnable(GL_TEXTURE_2D);

  /* Init texture */
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  /* Init OpenGL arrays */
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, 0, controller->viuv);
  glVertexPointer(2, GL_FLOAT, 0, controller->quad);

  /* Tell OpenGL that the byte order is swapped. */
#ifdef LITTLE_ENDIAN
  glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
#endif

  InitVIF(controller);

  /* Set the initial viuw, quad coordinates. */
  controller->quad[0] = controller->quad[5] =
  controller->quad[6] = controller->quad[7] = -1;
  controller->quad[1] = controller->quad[2] =
  controller->quad[3] = controller->quad[4] = 1;
  controller->viuv[2] = controller->viuv[4] =
  controller->viuv[5] = controller->viuv[7] = 1;

  return controller;
}

/* ============================================================================
 *  DestroyVIF: Releases any resources allocated for an VIF instance.
 * ========================================================================= */
void
DestroyVIF(struct VIFController *controller) {
  glfwCloseWindow();
  glfwTerminate();
  free(controller);
}

/* ============================================================================
 *  InitVIF: Initializes the VIF controller.
 * ========================================================================= */
static void
InitVIF(struct VIFController *controller) {
  debug("Initializing VIF.");
  memset(controller, 0, sizeof(*controller));
  controller->startTime = glfwGetTime();
}

/* ============================================================================
 *  CycleVIF: Lets the VI know we are cycling the machine.
 * ========================================================================= */
void
CycleVIF(struct VIFController *controller) {
  if (unlikely(controller->cyclesUntilIntr == 0)) {
    double vis, hz;

    if (++(controller->frameCount) == 10) {
      vis = (double) 10 / (glfwGetTime() - controller->startTime);
      hz = ((double) 6250000 / 60) / (glfwGetTime() - controller->startTime);
      printf("%.2f VI/s, RCP: %.2f MHZ, VR4300: %.2f MHz\n", vis, hz / 10000,
        hz / 10000 * 1.5);

      controller->startTime = glfwGetTime();
      controller->frameCount = 0;
    }

    ResetPerspective(controller);
    RenderFrame(controller);

    /* Raise an interrupt. */
    BusRaiseRCPInterrupt(controller->bus, MI_INTR_VI);
    controller->cyclesUntilIntr = (62500000 / 60) + 1;
  }

  controller->cyclesUntilIntr--;
}

/* ============================================================================
 *  VIRegRead: Read from VI registers.
 * ========================================================================= */
int
VIRegRead(void *_controller, uint32_t address, void *_data) {
   struct VIFController *controller = (struct VIFController*) _controller;
   uint32_t *data = (uint32_t*) _data;

  address -= VI_REGS_BASE_ADDRESS;
  enum VIRegister reg = (enum VIRegister) (address / 4);

  /* TODO: This might very well be a giant hack. */
  if (controller->regs[VI_V_SYNC_REG] > 0) {
    controller->regs[VI_CURRENT_REG] =
      (((62500000 / 60) + 1) - (controller->cyclesUntilIntr)) /
      (((62500000 / 60) + 1) / controller->regs[VI_V_SYNC_REG]);

    controller->regs[VI_CURRENT_REG] &= ~0x1;
  }

  else
    controller->regs[VI_CURRENT_REG] = 0;

  debugarg("VIRegRead: Reading from register [%s].", VIRegisterMnemonics[reg]);
  *data = controller->regs[reg];

  return 0;
}

/* ============================================================================
 *  VIRegWrite: Write to VI registers.
 * ========================================================================= */
int
VIRegWrite(void *_controller, uint32_t address, void *_data) {
   struct VIFController *controller = (struct VIFController*) _controller;
   uint32_t *data = (uint32_t*) _data;

  address -= VI_REGS_BASE_ADDRESS;
  enum VIRegister reg = (enum VIRegister) (address / 4);

  debugarg("VIRegWrite: Writing to register [%s].", VIRegisterMnemonics[reg]);

  switch(reg) {
  case VI_CURRENT_REG:
    BusClearRCPInterrupt(controller->bus, MI_INTR_VI);
    break;

  default:
    controller->regs[reg] = *data;
    break;
  }

  return 0;
}

