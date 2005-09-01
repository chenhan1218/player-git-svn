/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) <insert dates here>
 *     <insert author's name(s) here>
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

#include <errno.h>
#include <unistd.h>

#include "remote_driver.h"

TCPRemoteDriver::TCPRemoteDriver(player_devaddr_t addr, void* arg) 
        : Driver(NULL, 0, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  if(this->ptcp)
    this->ptcp = (PlayerTCP*)arg;
  else
    this->ptcp = NULL;

  this->sock = -1;
}

TCPRemoteDriver::~TCPRemoteDriver()
{
}

int 
TCPRemoteDriver::Setup() 
{
  struct sockaddr_in server;
  char banner[PLAYER_IDENT_STRLEN];

  printf("trying to Setup %d:%d:%d:%d\n",
         this->device_addr.host,
         this->device_addr.robot,
         this->device_addr.interf,
         this->device_addr.index);

  // Construct socket 
  this->sock = socket(PF_INET, SOCK_STREAM, 0);
  if(this->sock < 0)
  {
    PLAYER_ERROR1("socket call failed with error [%s]", strerror(errno));
    return(-1);
  }

  server.sin_family = PF_INET;
  server.sin_addr.s_addr = this->device_addr.host;
  server.sin_port = htons(this->device_addr.robot);

  // Connect the socket 
  if(connect(this->sock, (struct sockaddr*)&server, sizeof(server)) < 0)
  {
    PLAYER_ERROR3("connect call on [%u:%u] failed with error [%s]",
                this->device_addr.host, 
                this->device_addr.robot, 
                strerror(errno));
    return(-1);
  }

  printf("connected to: %u:%u\n",
         this->device_addr.host, this->device_addr.robot);

  // Get the banner 
  if(recv(this->sock, banner, sizeof(banner), 0) < (int)sizeof(banner))
  {
    PLAYER_ERROR("incomplete initialization string");
    return(-1);
  }
  printf("got banner: %s\n", banner);

  /*
  // subscribe to the remote device
  unsigned char buf[512];
  player_msghdr_t hdr;
  player_device_req_t req;

  hdr.addr = this->device_addr;
  hdr.type = PLAYER_MSGTYPE_REQ;
  hdr.subtype = PLAYER_DRI

  req.addr = this->device_addr;
  req.access = PLAYER_OPEN_MODE;
  req.driver_name_count = 0;
  if(
  //this->ptcp->AddClient(NULL, 0, this->sock, this);
  */

  return(0);
}

int 
TCPRemoteDriver::Shutdown() 
{ 
  printf("trying to Shutdown %d:%d:%d:%d\n",
         this->device_addr.host,
         this->device_addr.robot,
         this->device_addr.interf,
         this->device_addr.index);
  
  if(this->sock >= 0)
    close(this->sock);
  this->sock = -1;
  return(0); 
}

void 
TCPRemoteDriver::Update() 
{
}

Driver* 
TCPRemoteDriver::TCPRemoteDriver_Init(player_devaddr_t addr, void* arg)
{
  printf("TCPRemoteDriver_Init(%d:%d:%d:%d)\n",
         addr.host, addr.robot, addr.interf, addr.index);
  return((Driver*)(new TCPRemoteDriver(addr, arg)));
}
