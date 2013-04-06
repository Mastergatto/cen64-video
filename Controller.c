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

  if (glfwInit() != GL_TRUE) {
    debug("Failed to initialize GLFW.");
    return NULL;
  }

  glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
  if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 8, 0, GLFW_WINDOW) != GL_TRUE) {
    debug("Failed to open a GLFW window.");

    glfwTerminate();
    return NULL;
  }

  glfwSetWindowTitle("CEN64");
  glfwPollEvents();

  /* Load the viewport. */
  glViewport(0, 0, 640, 480);

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
  glEnable(GL_TEXTURE_RECTANGLE);
  glGenTextures(1, &controller->frameTexture);
  glBindTexture(GL_TEXTURE_RECTANGLE, controller->frameTexture);

  /* Tell OpenGL that the byte order is swapped. */
#ifdef LITTLE_ENDIAN
  glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
#endif

  InitVIF(controller);
  return controller;
}

/* ============================================================================
 *  DestroyVIF: Releases any resources allocated for an VIF instance.
 * ========================================================================= */
void
DestroyVIF(struct VIFController *controller) {
  glDeleteTextures(1, &controller->frameTexture);

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
}

/* ============================================================================
 *  CycleVIF: Lets the VI know we are cycling the machine.
 * ========================================================================= */
void
CycleVIF(struct VIFController *controller) {
  if (unlikely(controller->cyclesUntilIntr == 0)) {
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
  case VI_H_START_REG:
  case VI_V_START_REG:
  case VI_X_SCALE_REG:
  case VI_Y_SCALE_REG:
  case VI_WIDTH_REG:
    controller->regs[reg] = *data;
    ResetPerspective(controller);
    break;

  case VI_CURRENT_REG:
    BusClearRCPInterrupt(controller->bus, MI_INTR_VI);
    break;

  default:
    controller->regs[reg] = *data;
    break;
  }

  return 0;
}

