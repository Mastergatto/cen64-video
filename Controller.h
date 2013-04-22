/* ============================================================================
 *  Controller.h: Video controller.
 *
 *  VIDEOSIM: VIDEO Interface SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VIF__CONTROLLER_H__
#define __VIF__CONTROLLER_H__
#include "Address.h"
#include "Common.h"
#include "Render.h"
#include <GL/glfw.h>

enum VIRegister {
#define X(reg) reg,
#include "Registers.md"
#undef X
  NUM_VI_REGISTERS
};

#ifndef NDEBUG
extern const char *VIRegisterMnemonics[NUM_VI_REGISTERS];
#endif

struct BusController;

struct VIFController {
  struct BusController *bus;

  unsigned long long cyclesUntilIntr;
  double startTime;
  unsigned frameCount;

  uint32_t regs[NUM_VI_REGISTERS];
  struct RenderArea renderArea;
  GLuint frameTexture;
};

struct VIFController *CreateVIF(void);
void DestroyVIF(struct VIFController *);

#endif

