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
#ifndef __VIF__RENDER_H__
#define __VIF__RENDER_H__

struct RenderArea {
  struct {
    unsigned start;
    unsigned end;
  } x;

  struct {
    unsigned start;
    unsigned end;
  } y;

  unsigned height;
  unsigned width;
  int hskip;
};

struct VIFController;

void RenderFrame(struct VIFController *);
void ResetPerspective(struct VIFController *);

#endif

