#ifndef GUARD_FIELDMAP2_H
#define GUARD_FIELDMAP2_H
#include "field_map_obj.h"

struct MapHeader *mapconnection_get_mapheader(struct MapConnection *connection);
int GetMapBorderIdAt(int x, int y);
extern bool8 IsMetatileDirectionallyImpassable(struct MapObject *mapObject, s16 x, s16 y, u8 direction);
int CanCameraMoveInDirection(int direction);

#endif
