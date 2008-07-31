/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * $Id$
 */

#include "nm_plugin.h"
#include "nm_service.h"

#include <daemon.h>
#include <processing/jobs/callback_job.h>

typedef struct private_nm_plugin_t private_nm_plugin_t;

/**
 * private data of nm plugin
 */
struct private_nm_plugin_t {

	/**
	 * implements plugin interface
	 */
	nm_plugin_t public;
	
	GMainLoop *loop;
};

/**
 * NM plugin processing routine, creates and handles NMVPNPlugin
 */
static job_requeue_t run(private_nm_plugin_t *this)
{
	NMStrongswanPlugin *plugin;
	GMainLoop *loop;

	plugin = nm_strongswan_plugin_new();
	
	this->loop = loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
	
	g_main_loop_unref(loop);
	g_object_unref(plugin);
	
	return JOB_REQUEUE_NONE;
}

/**
 * Implementation of plugin_t.destroy
 */
static void destroy(private_nm_plugin_t *this)
{
	if (this->loop)
	{
		g_main_loop_quit(this->loop);
	}
	free(this);
}

/*
 * see header file
 */
plugin_t *plugin_create()
{
	private_nm_plugin_t *this = malloc_thing(private_nm_plugin_t);
	
	this->public.plugin.destroy = (void(*)(plugin_t*))destroy;
	
	this->loop = NULL;
	g_type_init ();
	if (!g_thread_supported())
	{
		g_thread_init(NULL);
	}
	
	charon->processor->queue_job(charon->processor, 
		 (job_t*)callback_job_create((callback_job_cb_t)run, this, NULL, NULL));
	
	return &this->public.plugin;
}

