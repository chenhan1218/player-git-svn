/* 
 *  libplayerc : a Player client library
 *  Copyright (C) Andrew Howard 2002
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/***************************************************************************
 * Desc: Sonar device proxy
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id$
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "playerc.h"
#include "error.h"


// Local declarations
void playerc_sonar_putdata(playerc_sonar_t *device, player_msghdr_t *header,
                           player_sonar_data_t *data, size_t len);

// Create a new sonar proxy
playerc_sonar_t *playerc_sonar_create(playerc_client_t *client, int index)
{
  playerc_sonar_t *device;

  device = malloc(sizeof(playerc_sonar_t));
  memset(device, 0, sizeof(playerc_sonar_t));
  playerc_device_init(&device->info, client, PLAYER_SONAR_CODE, index,
                      (playerc_putdata_fn_t) playerc_sonar_putdata);
    
  return device;
}


// Destroy a sonar proxy
void playerc_sonar_destroy(playerc_sonar_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}


// Subscribe to the sonar device
int playerc_sonar_subscribe(playerc_sonar_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the sonar device
int playerc_sonar_unsubscribe(playerc_sonar_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_sonar_putdata(playerc_sonar_t *device, player_msghdr_t *header,
                           player_sonar_data_t *data, size_t len)
{
  int i;

  assert(sizeof(*data) <= len);

  device->scan_count = 16;
  for (i = 0; i < 16; i++)
    device->scan[i] = ntohs(data->ranges[i]) / 1000.0;
}

