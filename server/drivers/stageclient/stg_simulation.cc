/*
 *  Stage-1.4 driver for Player
 *  Copyright (C) 2003  Richard Vaughan (rtv) vaughan@hrl.com 
 *
 * This program is free software; you can redistribute it and/or modify
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
 * $Id$
 */
#include "playercommon.h"
#include "drivertable.h"
#include "player.h"
#include "stageclient.h"

#include "stg_time.h"
extern PlayerTime* GlobalTime;

extern int global_argc;
extern char** global_argv;


// DRIVER FOR SIMULATION INTERFACE ////////////////////////////////////////


// CLASS -------------------------------------------------------------

// This driver talks to the Stage simulator over a socket. It creates
// a Stage client, then reads a Stage worldfile and uploads and
// configures models. It also receives model properties that are
// generated by subscriptions and caches them locally for other
// devices to retrieve.

class StgSimulation:public Stage1p4
{
public:
  StgSimulation(char* interface, ConfigFile* cf, int section);
  ~StgSimulation();
  
  // TODO - implement some simulator interface things - pause, resume,
  // save, load, etc.

  virtual void Update();
};


StgSimulation::StgSimulation(char* interface, ConfigFile* cf, int section ) 
  : Stage1p4( interface, cf, section, 
	      sizeof(player_simulation_data_t), 
	      sizeof(player_simulation_cmd_t), 1, 1 )
{
  PLAYER_MSG1( "constructing StgSimulation with interface %s", interface );
  
  if( Stage1p4::stage_client != NULL )
    {
      PRINT_ERR( "Stage client already exists."
		 " Player only supports one Stage connection" );
      exit(-1);
    }
  
  const int stage_port = 
    config->ReadInt(section, "port", STG_DEFAULT_SERVER_PORT);
  
  const char* stage_host = 
    config->ReadString(section, "host", STG_DEFAULT_SERVER_HOST);
  
  // load a worldfile
  char worldfile_name[MAXPATHLEN];
  const char* wfn = 
    config->ReadString(section, "worldfile", STG_DEFAULT_WORLDFILE);
  strncpy( worldfile_name, wfn, MAXPATHLEN );
  
  // Find the worldfile.  If the filename begins with a '/', it's
  // an absolute path and we leave it alone. Otherwise it's a
  // relative path so we need to append the config file's path to
  // the front.
  
  // copy the config filename
  char tmp[MAXPATHLEN];
  strncpy( tmp, cf->filename, MAXPATHLEN );
  
  // cut off the filename, leaving just the path prefix
  char* last_slash = strrchr( tmp, '/' );
  if( last_slash )
    {
      *last_slash = 0; // turn the last slash into a terminator 
      assert( chdir( tmp ) == 0 );
    }
  
  //printf( "cfg.filename \"%s\" (path: \"%s\")"
  //"\nwfn \"%s\"\nWorldfile_name \"%s\"\n", 
  //cf->filename, tmp, wfn, worldfile_name );
  
  // a little sanity testing
  if( !g_file_test( worldfile_name, G_FILE_TEST_EXISTS ) )
    {
      PLAYER_ERROR1( "worldfile \"%s\" does not exist", worldfile_name );
      return;
    }
  
  printf( "    Creating Stage client..." ); fflush(stdout);
  Stage1p4::stage_client = stg_client_create();
  assert(Stage1p4::stage_client);
  puts( " done" );
  
  // steal the global clock - a bit aggressive, but a simple approach
  if( GlobalTime ) delete GlobalTime;
  assert( (GlobalTime = new StgTime( Stage1p4::stage_client ) ));
  
  // create a passel of Stage models in the local cache based on the
  // worldfile
  printf( "    Stage driver loading worldfile \"%s\" ", worldfile_name );      
  fflush(stdout);
  Stage1p4::world = 
    stg_client_worldfile_load( Stage1p4::stage_client, worldfile_name );
  assert(world);
  printf( " done.\n" );
   
  // this kicks off the Stage client thread
  printf( "    Connecting to Stage at %s:%d... ", 
	  stage_host, stage_port ); 
  fflush(stdout); 
  stg_client_connect( Stage1p4::stage_client, (char*)stage_host, stage_port );
  puts( " done." );
  
  //now we push the cache up into the server
  printf( "    Uploading Stage world to server..." ); fflush(stdout);
  stg_client_push( stage_client );
  puts( " done." );
  
  // subscribe to something so we get the clock updates 
  // (there's always a model called "root" )
  stg_model_t* root = 
    stg_world_model_name_lookup( Stage1p4::world, "root" );
  assert(root);
  
  stg_model_subscribe( root, STG_PROP_TIME, 100 ); //100ms
  
  // start the simulation
  printf( "    Stage driver starting world clock... " ); fflush(stdout);
  stg_world_resume( world );
  puts( "done." );

  this->subscribe_prop = STG_PROP_TIME;
  
  // make Player call Update() on this device even when noone is subscribed
  this->alwayson = TRUE;
}

StgSimulation::~StgSimulation()
{
  if( stage_client )
    {
      PLAYER_MSG0( "Shutting down Stage client" );
      
      // Stage will automatically trash the world we created when it
      // gets a SIGHUP from our connection. We can just fold up the
      // client this end.
      stg_client_destroy( stage_client );
      stage_client = NULL;
    }
}

// Player registration ----------------------------------------------------

CDevice* StgSimulation_Init(char* interface, ConfigFile* cf, int section)
{
  if(strcmp(interface, PLAYER_SIMULATION_STRING))
    {
      PLAYER_ERROR1("driver \"stg_simulation\" does not support interface \"%s\"\n",
		    interface);
      return(NULL);
    }
  else 
    return((CDevice*)(new StgSimulation(interface, cf, section)));
}


void StgSimulation_Register(DriverTable* table)
{
  table->AddDriver("stg_simulation", PLAYER_ALL_MODE, StgSimulation_Init);
}


void StgSimulation::Update( void )
{
  // poll the client for new data, sleeping not at all
  stg_client_read( stage_client, 0 );
}