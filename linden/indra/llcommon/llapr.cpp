/** 
 * @file llapr.cpp
 * @author Phoenix
 * @date 2004-11-28
 * @brief Helper functions for using the apache portable runtime library.
 *
 * Copyright (c) 2004-2007, Linden Research, Inc.
 * 
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "linden_common.h"
#include "llapr.h"

apr_pool_t *gAPRPoolp = NULL; // Global APR memory pool
apr_thread_mutex_t *gLogMutexp = NULL;


void ll_init_apr()
{
	if (!gAPRPoolp)
	{
		// Initialize APR and create the global pool
		apr_initialize();
		apr_pool_create(&gAPRPoolp, NULL);

		// Initialize the logging mutex
		apr_thread_mutex_create(&gLogMutexp, APR_THREAD_MUTEX_DEFAULT, gAPRPoolp);
	}
}


void ll_cleanup_apr()
{
	llinfos << "Cleaning up APR" << llendl;

	if (gLogMutexp)
	{
		// Clean up the logging mutex

		// All other threads NEED to be done before we clean up APR, so this is okay.
		apr_thread_mutex_destroy(gLogMutexp);
		gLogMutexp = NULL;
	}
	if (gAPRPoolp)
	{
		apr_pool_destroy(gAPRPoolp);
		gAPRPoolp = NULL;
	}
	apr_terminate();
}

//
// LLScopedLock
//
LLScopedLock::LLScopedLock(apr_thread_mutex_t* mutex) : mMutex(mutex)
{
	if(mutex)
	{
		if(ll_apr_warn_status(apr_thread_mutex_lock(mMutex)))
		{
			mLocked = false;
		}
		else
		{
			mLocked = true;
		}
	}
	else
	{
		mLocked = false;
	}
}

LLScopedLock::~LLScopedLock()
{
	unlock();
}

void LLScopedLock::unlock()
{
	if(mLocked)
	{
		if(!ll_apr_warn_status(apr_thread_mutex_unlock(mMutex)))
		{
			mLocked = false;
		}
	}
}

//
// Misc functions
//
bool ll_apr_warn_status(apr_status_t status)
{
	if(APR_SUCCESS == status) return false;
	char buf[MAX_STRING];	/* Flawfinder: ignore */
	llwarns << "APR: " << apr_strerror(status, buf, MAX_STRING) << llendl;
	return true;
}

void ll_apr_assert_status(apr_status_t status)
{
	llassert(ll_apr_warn_status(status) == false);
}

apr_file_t* ll_apr_file_open(const LLString& filename, apr_int32_t flags, S32* sizep)
{
	apr_file_t* apr_file;
	apr_status_t s;
	s = apr_file_open(&apr_file, filename.c_str(), flags, APR_OS_DEFAULT, gAPRPoolp);
	if (s != APR_SUCCESS)
	{
		if (sizep)
		{
			*sizep = 0;
		}
		return NULL;
	}

	if (sizep)
	{
		S32 file_size = 0;
		apr_off_t offset = 0;
		if (apr_file_seek(apr_file, APR_END, &offset) == APR_SUCCESS)
		{
			file_size = (S32)offset;
			offset = 0;
			apr_file_seek(apr_file, APR_SET, &offset);
		}
		*sizep = file_size;
	}

	return apr_file;
}

S32 ll_apr_file_read(apr_file_t* apr_file, void *buf, S32 nbytes)
{
	apr_size_t sz = nbytes;
	apr_status_t s = apr_file_read(apr_file, buf, &sz);
	if (s != APR_SUCCESS)
	{
		return 0;
	}
	else
	{
		return (S32)sz;
	}
}


S32 ll_apr_file_write(apr_file_t* apr_file, const void *buf, S32 nbytes)
{
	apr_size_t sz = nbytes;
	apr_status_t s = apr_file_write(apr_file, buf, &sz);
	if (s != APR_SUCCESS)
	{
		return 0;
	}
	else
	{
		return (S32)sz;
	}
}

S32 ll_apr_file_seek(apr_file_t* apr_file, apr_seek_where_t where, S32 offset)
{
	apr_off_t apr_offset = offset;
	apr_status_t s = apr_file_seek(apr_file, where, &apr_offset);
	if (s != APR_SUCCESS)
	{
		return -1;
	}
	else
	{
		return (S32)apr_offset;
	}
}

bool ll_apr_file_remove(const LLString& filename)
{
	apr_status_t s;
	s = apr_file_remove(filename.c_str(), gAPRPoolp);
	if (s != APR_SUCCESS)
	{
		llwarns << "ll_apr_file_remove failed on file: " << filename << llendl;
		return false;
	}
	return true;
}

bool ll_apr_file_rename(const LLString& filename, const LLString& newname)
{
	apr_status_t s;
	s = apr_file_rename(filename.c_str(), newname.c_str(), gAPRPoolp);
	if (s != APR_SUCCESS)
	{
		llwarns << "ll_apr_file_rename failed on file: " << filename << llendl;
		return false;
	}
	return true;
}
