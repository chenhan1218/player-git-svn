/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *     Nik Melchior
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <playerclient.h>
#include <string.h>


void
EnergyProxy::FillData(player_msghdr_t hdr, const char *buffer)
{
  if(hdr.size != sizeof(player_energy_data_t)) {
    if(player_debug_level(-1) >= 1)
      fprintf(stderr,"WARNING: energy proxy expected %d bytes of"
              " data, but received %d. Unexpected results may"
              " ensue.\n",
              sizeof(player_energy_data_t), hdr.size);
  }
  
  this->joules = (double)((player_energy_data_t *)buffer)->mjoules / 1000.0;
  this->watts = (double)((player_energy_data_t *)buffer)->mwatts / 1000.0;
  this->charging = (bool)((player_energy_data_t *)buffer)->charging;
}

// interface that all proxies SHOULD provide
void 
EnergyProxy::Print()
{
  printf("#Energy(%d:%d) - %c\n", m_device_id.code, 
         m_device_id.index, access);
  printf( "#Joules / Watts / Charging\n%f\t%f\t%s\n",
	  this->joules, this->watts, this->charging ? "TRUE" : "FALSE" );
  puts(" ");
}

