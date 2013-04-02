/* ============================================================================
 *  Externs.h: External definitions for the VIF plugin.
 *
 *  VIDEOSIM: VIDEO Interface SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VIF__EXTERNS_H__
#define __VIF__EXTERNS_H__

struct BusController;

void BusClearRCPInterrupt(struct BusController *, unsigned);
void BusRaiseRCPInterrupt(struct BusController *, unsigned);
const uint8_t *BusGetRDRAMPointer(struct BusController *);

#endif

