/**
 * @file host.c
 * 
 * @brief Implementation of host_t.
 * 
 */

/*
 * Copyright (C) 2005 Jan Hutter, Martin Willi
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
 */

#include "host.h"

#include <utils/allocator.h>


typedef struct private_host_t private_host_t;

/**
 * @brief Private Data of a host object.
 */
struct private_host_t { 	
	/**
	 * Public data
	 */
	host_t public;
	
	/**
	 * Address family to use, such as AF_INET or AF_INET6
	 */
	int family;
	
	/**
	 * low-lewel structure, wich stores the address
	 */
	sockaddr_t address;
	
	/**
	 * length of address structure
	 */
	socklen_t socklen;
};


/**
 * implements host_t.get_sockaddr
 */
static sockaddr_t  *get_sockaddr(private_host_t *this)
{
	return &(this->address);
}

/**
 * implements host_t.get_sockaddr_len
 */
static socklen_t *get_sockaddr_len(private_host_t *this)
{
	return &(this->socklen);
}

/**
 * implements host_t.get_address
 */
static char *get_address(private_host_t *this)
{
	switch (this->family) 
	{
		case AF_INET: 
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)&(this->address);
			return inet_ntoa(sin->sin_addr);
		}
		default:
		{
			return "(family	not supported)";
		}
	}
}

/**
 * Implementation of host_t.get_address_as_chunk.
 */
static chunk_t get_address_as_chunk(private_host_t *this)
{
	chunk_t address = CHUNK_INITIALIZER;
	
	switch (this->family) 
	{
		case AF_INET: 
		{
			/* allocate 4 bytes for IPV4 address*/
			address.ptr = allocator_alloc(4);
			address.len = 4;
			struct sockaddr_in *sin = (struct sockaddr_in*)&(this->address);
			memcpy(address.ptr,&(sin->sin_addr.s_addr),4);
		}
		default:
		{
			/* empty chunk is returned */
			return address;
		}
	}
	
}

/**
 * implements host_t.get_port
 */
static u_int16_t get_port(private_host_t *this)
{
	switch (this->family) 
	{
		case AF_INET: 
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)&(this->address);
			return ntohs(sin->sin_port);
		}
		default:
		{
			return 0;
		}
	}
}

/**
 * Implements host_t.destroy
 */
static void destroy(private_host_t *this)
{
	allocator_free(this);
}

/**
 * Implements host_t.clone.
 */
static private_host_t *clone(private_host_t *this)
{
	private_host_t *new = allocator_alloc_thing(private_host_t);
		
	memcpy(new, this, sizeof(private_host_t));
	return new;
}


/**
 * Impelements host_t.equals
 */
static bool equals(private_host_t *this, private_host_t *other)
{
	switch (this->family)
	{
		/* IPv4 */
		case AF_INET:
		{
			struct sockaddr_in *sin1 = (struct sockaddr_in*)&(this->address);
			struct sockaddr_in *sin2 = (struct sockaddr_in*)&(other->address);
			if ((sin1->sin_family == sin2->sin_family) &&
				(sin1->sin_port == sin2->sin_port) &&
				(sin1->sin_addr.s_addr == sin2->sin_addr.s_addr))
			{
				return TRUE;	
			}
		}
	}
	return FALSE;
}


/*
 * Described in header.
 */
host_t *host_create(int family, char *address, u_int16_t port)
{
	private_host_t *this = allocator_alloc_thing(private_host_t);
	
	this->public.get_sockaddr = (sockaddr_t* (*) (host_t*))get_sockaddr;
	this->public.get_sockaddr_len = (socklen_t*(*) (host_t*))get_sockaddr_len;
	this->public.clone = (host_t* (*) (host_t*))clone;
	this->public.get_address = (char* (*) (host_t *))get_address;
	this->public.get_address_as_chunk = (chunk_t (*) (host_t *)) get_address_as_chunk;
	this->public.get_port = (u_int16_t (*) (host_t *))get_port;
	this->public.destroy = (void (*) (host_t*))destroy;
	
	this->family = family;

	switch (family)
	{
		/* IPv4 */
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)&(this->address);
			sin->sin_family = AF_INET;
			sin->sin_addr.s_addr = inet_addr(address);
			sin->sin_port = htons(port);
			this->socklen = sizeof(struct sockaddr_in);
			return (host_t*)this;
		}
	}
	allocator_free(this);
	return NULL;
}

/*
 * Described in header.
 */
host_t *host_create_from_chunk(int family, chunk_t address, u_int16_t port)
{
	private_host_t *this = allocator_alloc_thing(private_host_t);
	
	this->public.get_sockaddr = (sockaddr_t* (*) (host_t*))get_sockaddr;
	this->public.get_sockaddr_len = (socklen_t*(*) (host_t*))get_sockaddr_len;
	this->public.clone = (host_t* (*) (host_t*))clone;
	this->public.get_address = (char* (*) (host_t *))get_address;
	this->public.get_address_as_chunk = (chunk_t (*) (host_t *)) get_address_as_chunk;
	this->public.get_port = (u_int16_t (*) (host_t *))get_port;
	this->public.equals = (bool (*) (host_t *,host_t *))equals;
	this->public.destroy = (void (*) (host_t*))destroy;
	
	this->family = family;

	if (address.len == 4)
	{
		switch (family)
		{
			/* IPv4 */
			case AF_INET:
			{
				struct sockaddr_in *sin = (struct sockaddr_in*)&(this->address);
				sin->sin_family = AF_INET;
				memcpy(&(sin->sin_addr.s_addr),address.ptr,4);
				sin->sin_port = htons(port);
				this->socklen = sizeof(struct sockaddr_in);
				return (host_t*)this;
			}
		}
	}
	allocator_free(this);
	return NULL;
}
