/* shared_symbols.h
 * Consumed by both the game and the tools.
 */

#ifndef SHARED_SYMBOLS_H
#define SHARED_SYMBOLS_H

#define NS_sys_tilesize 32
#define NS_sys_mapw 20
#define NS_sys_maph 11

#define CMD_map_image     0x20 /* u16:imageid */
#define CMD_map_song      0x21 /* u16:songid */
#define CMD_map_sprite    0x60 /* u16:pos u16:spriteid u32:reserved */

#define NS_tilesheet_physics     1
#define NS_tilesheet_neighbors   0
#define NS_tilesheet_family      0
#define NS_tilesheet_weight      0

#define NS_spritetype_hero 1
#define NS_spritetype_pumpkin 2
#define NS_FOR_EACH_spritetype \
  _(hero) \
  _(pumpkin)

#endif
