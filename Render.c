/* ============================================================================
 *  Render.c: Video rendering.
 *
 *  VIDEOSIM: VIDEO Interface SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Controller.h"
#include "Externs.h"
#include "Render.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

#ifdef GLFW3
#include <GLFW/glfw3.h>
#else
#include <GL/glfw.h>
#endif

/* Hack to get access to this */
#ifdef _WIN32 /* #ifdef OPENGL_WUT */
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#endif

/* ============================================================================
 *  RenderFrame: Draws a frame to the display.
 * ========================================================================= */
void
RenderFrame(struct VIFController *controller) {
  debug("== Rastering Frame ==");
  debugarg("Height:  [%u].", controller->renderArea.height);
  debugarg("Width:   [%u].", controller->renderArea.width);
  debugarg("X Start: [%u].", controller->renderArea.x.start);
  debugarg("X End:   [%u].", controller->renderArea.x.end);
  debugarg("Y Start: [%u].", controller->renderArea.y.start);
  debugarg("Y End:   [%u].", controller->renderArea.y.end);

  uint32_t offset = controller->regs[VI_ORIGIN_REG] & 0xFFFFFF;
  const uint8_t *buffer = BusGetRDRAMPointer(controller->bus) + offset;

  int hskip = controller->renderArea.hskip;
  int vres = controller->renderArea.height;
  int hres = controller->renderArea.width;

  debugarg("H Res:   [%u].", hres);
  debugarg("V Res:   [%u].", vres);
  debugarg("H Skip:  [%d].", hskip);

  if (hres <= 0 || vres <= 0)
    return;

  /* Hacky? */
  if (hres > 640) {
    hskip += (hres - 640);
    hres = 640;
  }

  if (vres > 480) {
    vres = 480;
  }

  switch(controller->regs[VI_STATUS_REG] & 0x3) {
  case 0:
    break;

  case 1:
    assert(0 && "Attempted to use reserved frame type.");
    break;

  case 2: /* Renders a 16-bit frame. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hres + hskip, vres,
      0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, buffer);
    break;

  case 3: /* Renders a 32-bit frame. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hres + hskip, vres,
      0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    break;
  }

  controller->viuv[2] = controller->viuv[4] =
    (float) hres / (hres + hskip);

  glDrawArrays(GL_QUADS, 0, 4);

#ifdef GLFW3
  glfwPollEvents();
  glfwSwapBuffers(controller->window);
#else
  glfwSwapBuffers();
#endif
}

/* ============================================================================
 *  ResetPerspective: Calculates the coordinates of the visible area.
 * ========================================================================= */
void
ResetPerspective(struct VIFController *controller) {
  struct RenderArea *renderArea = &controller->renderArea;
  float hcoeff, vcoeff;

  /* Calculate the bounding positions. */
  renderArea->x.start = controller->regs[VI_H_START_REG] >> 16 & 0x3FF;
  renderArea->x.end = controller->regs[VI_H_START_REG] & 0x3FF;
  renderArea->y.start = controller->regs[VI_V_START_REG] >> 16 & 0x3FF;
  renderArea->y.end = controller->regs[VI_V_START_REG] & 0x3FF;

  hcoeff = (float)(controller->regs[VI_X_SCALE_REG] & 0xFFF) / (1 << 10);
  vcoeff = (float)(controller->regs[VI_Y_SCALE_REG] & 0xFFF) / (1 << 10);

  /* Calculate the height and width. */
  renderArea->height =((renderArea->y.end - renderArea->y.start) >> 1) * vcoeff;
  renderArea->width = ((renderArea->x.end - renderArea->x.start)) * hcoeff;
  renderArea->hskip = controller->regs[VI_WIDTH_REG] - renderArea->width;
}

