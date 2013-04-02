/* ============================================================================
 *  Registers.md: VI registers.
 *
 *  VIDEOSIM: VIDEO Interface SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef VI_REGISTER_LIST
#define VI_REGISTER_LIST \
  X(VI_STATUS_REG) \
  X(VI_ORIGIN_REG) \
  X(VI_WIDTH_REG) \
  X(VI_INTR_REG) \
  X(VI_CURRENT_REG) \
  X(VI_BURST_REG) \
  X(VI_V_SYNC_REG) \
  X(VI_H_SYNC_REG) \
  X(VI_LEAP_REG) \
  X(VI_H_START_REG) \
  X(VI_V_START_REG) \
  X(VI_V_BURST_REG) \
  X(VI_X_SCALE_REG) \
  X(VI_Y_SCALE_REG)
#endif

VI_REGISTER_LIST

