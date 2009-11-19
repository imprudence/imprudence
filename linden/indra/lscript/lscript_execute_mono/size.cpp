/** 
 * NOTE: none of this actually applies as this file was taken from mono/samples
 * @file size.c
 * @brief file from mono samples 
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include <glib.h>
#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/profiler.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/llassembly.h>
#include <string.h>

#include "stdtypes.h"
#include "linden_common.h"

#define FIELD_ATTRIBUTE_STATIC 0x10
#define FIELD_ATTRIBUTE_HAS_FIELD_RVA 0x100

static int memory_usage (MonoObject *obj, GHashTable *visited, 
			 int (*functor)(MonoObject*, MonoType*, int));

static int
memory_usage_array (MonoArray *array, GHashTable *visited,
		    int (*functor)(MonoObject*, MonoType*, int))
{
  int total = 0;
  MonoClass *array_class = mono_object_get_class ((MonoObject *) array);
  MonoClass *element_class = mono_class_get_element_class (array_class);
  MonoType *element_type = mono_class_get_type (element_class);

  if (MONO_TYPE_IS_REFERENCE (element_type)) {
    int i;

    for (i = 0; i < mono_array_length (array); i++) {
      MonoObject *element = (MonoObject*)mono_array_get (array, gpointer, i);

      if (element != NULL)
	total += memory_usage (element, visited, functor);
    }
  }

  return total;
}

static int
memory_usage (MonoObject *obj, GHashTable *visited, 
	      int (*functor)(MonoObject*, MonoType*, int))
{
  int total = 0;
  MonoClass *klass;
  MonoType *type;
  gpointer iter = NULL;
  MonoClassField *field;

  if (g_hash_table_lookup (visited, obj))
    return 0;

  g_hash_table_insert (visited, obj, obj);

  klass = mono_object_get_class (obj);
  type = mono_class_get_type (klass);

  /* This is an array, so drill down into it */
  if (type->type == MONO_TYPE_SZARRAY)
    total += memory_usage_array ((MonoArray *) obj, visited, functor);

  while ((field = mono_class_get_fields (klass, &iter)) != NULL) {
    MonoType *ftype = mono_field_get_type (field);
    gpointer value;

    if ((ftype->attrs & (FIELD_ATTRIBUTE_STATIC | FIELD_ATTRIBUTE_HAS_FIELD_RVA)) != 0)
      continue;

    /* FIXME: There are probably other types we need to drill down into */
    switch (ftype->type) {

    case MONO_TYPE_CLASS:
    case MONO_TYPE_OBJECT:
      mono_field_get_value (obj, field, &value);

      if (value != NULL)
	total += memory_usage ((MonoObject *) value, visited, functor);

      break;

    case MONO_TYPE_STRING:
      mono_field_get_value (obj, field, &value);
      if (value != NULL) 
	total += mono_object_get_size ((MonoObject *) value);
      break;

    case MONO_TYPE_SZARRAY:
      mono_field_get_value (obj, field, &value);

      if (value != NULL) {
	total += memory_usage_array ((MonoArray *) value, visited, functor);
	total += mono_object_get_size ((MonoObject *) value);
      }

      break;

    default:
      /* printf ("Got type 0x%x\n", ftype->type); */
      /* ignore, this will be included in mono_object_get_size () */
      break;
    }
  }

  total = functor(obj, type, total);

  return total;
}


int addObjectSize(MonoObject* obj, MonoType* type, int total) 
{
  return total + mono_object_get_size (obj);
}

/*
 * Only returns data for instances, not for static fields, those might
 * be larger, or hold larger structures
 */
int
GetMemoryUsage (MonoObject *obj)
{
  GHashTable *visited = g_hash_table_new (NULL, NULL);
  int n;
  
  n = memory_usage (obj, visited, addObjectSize);

  g_hash_table_destroy (visited);
  
  return n;
}



int printObjectSize(MonoObject* obj, MonoType* type, int total) 
{
  total += mono_object_get_size (obj);
  llinfos << "Object type: " << mono_type_full_name(type) << " size: "
	  << total << llendl;
    
  return total;
}

/*
 * Only returns data for instances, not for static fields, those might
 * be larger, or hold larger structures
 */
int
PrintMemoryUsage (MonoObject *obj)
{
  GHashTable *visited = g_hash_table_new (NULL, NULL);
  int n;
  
  n = memory_usage (obj, visited, printObjectSize);

  g_hash_table_destroy (visited);
  
  return n;
}
