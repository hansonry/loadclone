/*
 *  Copyright (C) 2015 Ryan Hanson <hansonry@gmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 */
#ifndef __GLOBALDATA_H__
#define __GLOBALDATA_H__



// Width and height of each tile
#define TILE_WIDTH   32
#define TILE_HEIGHT  32


// Map tile image locations
// y & x
#define IMGID_BLOCK          0x0000
#define IMGID_BROKENBLOCK_0  0x0300
#define IMGID_BROKENBLOCK_1  0x0301
#define IMGID_BROKENBLOCK_2  0x0302
#define IMGID_LADDER         0x0102
#define IMGID_GOLD           0x0502
#define IMGID_GUY            0x0000
#define IMGID_BAR            0x0002
#define IMGID_DOORCLOSE      0x0003
#define IMGID_DOOROPEN       0x0100



// Movement Timeing
#define MOVE_TIMEOUT             0.3f
#define FALL_TIMEOUT             0.1f
#define DIG_TIMEOUT              0.5f
#define HOLE_TIMEOUT             5.0f
#define DIG_SPOT_DELATA_FRAME_TIMEOUT 0.1f
#define DIG_SPOT_FRAME_COUNT     3

#endif // __GLOBALDATA_H__

