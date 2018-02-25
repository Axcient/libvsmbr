/*
 * The partition functions
 *
 * Copyright (C) 2010-2018, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "libvsmbr_definitions.h"
#include "libvsmbr_io_handle.h"
#include "libvsmbr_libbfio.h"
#include "libvsmbr_libcerror.h"
#include "libvsmbr_libcthreads.h"
#include "libvsmbr_partition.h"
#include "libvsmbr_sector_data.h"
#include "libvsmbr_types.h"

/* Creates a partition
 * Make sure the value partition is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libvsmbr_partition_initialize(
     libvsmbr_partition_t **partition,
     libbfio_handle_t *file_io_handle,
     libvsmbr_partition_values_t *partition_values,
     libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                             = "libvsmbr_partition_initialize";

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	if( *partition != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid partition value already set.",
		 function );

		return( -1 );
	}
	if( partition_values == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition values.",
		 function );

		return( -1 );
	}
	internal_partition = memory_allocate_structure(
	                      libvsmbr_internal_partition_t );

	if( internal_partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create partition.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_partition,
	     0,
	     sizeof( libvsmbr_internal_partition_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear partition.",
		 function );

		goto on_error;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_initialize(
	     &( internal_partition->read_write_lock ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to intialize read/write lock.",
		 function );

		goto on_error;
	}
#endif
	internal_partition->file_io_handle   = file_io_handle;
	internal_partition->partition_values = partition_values;

	*partition = (libvsmbr_partition_t *) internal_partition;

	return( 1 );

on_error:
	if( internal_partition != NULL )
	{
		memory_free(
		 internal_partition );
	}
	return( -1 );
}

/* Frees a partition
 * Returns 1 if successful or -1 on error
 */
int libvsmbr_partition_free(
     libvsmbr_partition_t **partition,
     libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                             = "libvsmbr_partition_free";
	int result                                        = 1;

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	if( *partition != NULL )
	{
		internal_partition = (libvsmbr_internal_partition_t *) *partition;
		*partition         = NULL;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
		if( libcthreads_read_write_lock_free(
		     &( internal_partition->read_write_lock ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free read/write lock.",
			 function );

			result = -1;
		}
#endif
		memory_free(
		 internal_partition );
	}
	return( result );
}

/* Retrieves the partition type
 * Returns 1 if successful or -1 on error
 */
int libvsmbr_partition_get_type(
     libvsmbr_partition_t *partition,
     uint8_t *type,
     libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                             = "libvsmbr_partition_get_type";
	int result                                        = 1;

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	internal_partition = (libvsmbr_internal_partition_t *) partition;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libvsmbr_partition_values_get_type(
	     internal_partition->partition_values,
	     type,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve type.",
		 function );

		result = -1;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Reads (partition) data at the current offset into a buffer using a Basic File IO (bfio) handle
 * This function is not multi-thread safe acquire write lock before call
 * Returns the number of bytes read or -1 on error
 */
ssize_t libvsmbr_internal_partition_read_buffer_from_file_io_handle(
         libvsmbr_internal_partition_t *internal_partition,
         libbfio_handle_t *file_io_handle,
         void *buffer,
         size_t buffer_size,
         libcerror_error_t **error )
{
	libvsmbr_sector_data_t *sector_data = NULL;
	static char *function               = "libvsmbr_internal_partition_read_buffer_from_file_io_handle";
	off64_t element_data_offset         = 0;
	size_t buffer_offset                = 0;
	size_t read_size                    = 0;

	if( internal_partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	if( internal_partition->current_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid partition - current offset value out of bounds.",
		 function );

		return( -1 );
	}
	if( buffer_size == 0 )
	{
		return( 0 );
	}
	if( (size64_t) internal_partition->current_offset >= internal_partition->size )
	{
		return( 0 );
	}
	if( (size64_t) ( internal_partition->current_offset + buffer_size ) > internal_partition->size )
	{
		buffer_size = (size_t) ( internal_partition->size - internal_partition->current_offset );
	}
	while( buffer_size > 0 )
	{
		if( libfdata_vector_get_element_value_at_offset(
		     internal_partition->sectors_vector,
		     (intptr_t *) file_io_handle,
		     internal_partition->sectors_cache,
		     internal_partition->current_offset,
		     &element_data_offset,
		     (intptr_t **) &sector_data,
		     0,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve sector data at offset: 0x%08" PRIx64 ".",
			 function,
			 internal_partition->current_offset );

			return( -1 );
		}
		if( sector_data == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: missing sector data.",
			 function );

			return( -1 );
		}
		read_size = sector_data->data_size - (size_t) element_data_offset;

		if( buffer_size < read_size )
		{
			read_size = buffer_size;
		}
		if( memory_copy(
		     &( ( (uint8_t *) buffer )[ buffer_offset ] ),
		     &( sector_data->data[ element_data_offset ] ),
		     read_size ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy sector data.",
			 function );

			return( -1 );
		}
		internal_partition->current_offset += read_size;
		buffer_offset                           += read_size;
		buffer_size                             -= read_size;
	}
	return( (ssize_t) buffer_offset );
}

/* Reads (partition) data at the current offset into a buffer
 * Returns the number of bytes read or -1 on error
 */
ssize_t libvsmbr_partition_read_buffer(
         libvsmbr_partition_t *partition,
         void *buffer,
         size_t buffer_size,
         libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                                       = "libvsmbr_partition_read_buffer";
	ssize_t read_count                                          = 0;

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	internal_partition = (libvsmbr_internal_partition_t *) partition;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	read_count = libvsmbr_internal_partition_read_buffer_from_file_io_handle(
		      internal_partition,
		      internal_partition->file_io_handle,
		      buffer,
		      buffer_size,
		      error );

	if( read_count == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read buffer from partition.",
		 function );

		read_count = -1;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( read_count );
}

/* Reads (partition) data at a specific offset
 * Returns the number of bytes read or -1 on error
 */
ssize_t libvsmbr_partition_read_buffer_at_offset(
         libvsmbr_partition_t *partition,
         void *buffer,
         size_t buffer_size,
         off64_t offset,
         libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                                       = "libvsmbr_partition_read_buffer_at_offset";
	ssize_t read_count                                          = 0;

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	internal_partition = (libvsmbr_internal_partition_t *) partition;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libvsmbr_internal_partition_seek_offset(
	     internal_partition,
	     offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset.",
		 function );

		goto on_error;
	}
	read_count = libvsmbr_internal_partition_read_buffer_from_file_io_handle(
		      internal_partition,
		      internal_partition->file_io_handle,
		      buffer,
		      buffer_size,
		      error );

	if( read_count == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read buffer.",
		 function );

		goto on_error;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( read_count );

on_error:
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 internal_partition->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Seeks a certain offset of the (partition) data
 * This function is not multi-thread safe acquire write lock before call
 * Returns the offset if seek is successful or -1 on error
 */
off64_t libvsmbr_internal_partition_seek_offset(
         libvsmbr_internal_partition_t *internal_partition,
         off64_t offset,
         int whence,
         libcerror_error_t **error )
{
	static char *function = "libvsmbr_internal_partition_seek_offset";

	if( internal_partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	if( ( whence != SEEK_CUR )
	 && ( whence != SEEK_END )
	 && ( whence != SEEK_SET ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported whence.",
		 function );

		return( -1 );
	}
	if( whence == SEEK_CUR )
	{
		offset += internal_partition->current_offset;
	}
	else if( whence == SEEK_END )
	{
		offset += (off64_t) internal_partition->size;
	}
	if( offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid offset value out of bounds.",
		 function );

		return( -1 );
	}
	internal_partition->current_offset = offset;

	return( offset );
}

/* Seeks a certain offset of the (partition) data
 * Returns the offset if seek is successful or -1 on error
 */
off64_t libvsmbr_partition_seek_offset(
         libvsmbr_partition_t *partition,
         off64_t offset,
         int whence,
         libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                                       = "libvsmbr_partition_seek_offset";

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	internal_partition = (libvsmbr_internal_partition_t *) partition;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	offset = libvsmbr_internal_partition_seek_offset(
	          internal_partition,
	          offset,
	          whence,
	          error );

	if( offset == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset.",
		 function );

		offset = -1;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( offset );
}

/* Retrieves the partition offset
 * Returns 1 if successful or -1 on error
 */
int libvsmbr_partition_get_offset(
     libvsmbr_partition_t *partition,
     off64_t *offset,
     libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                             = "libvsmbr_partition_get_offset";
	int result                                        = 1;

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	internal_partition = (libvsmbr_internal_partition_t *) partition;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libvsmbr_partition_values_get_offset(
	     internal_partition->partition_values,
	     offset,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve offset.",
		 function );

		result = -1;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the partition size
 * Returns 1 if successful or -1 on error
 */
int libvsmbr_partition_get_size(
     libvsmbr_partition_t *partition,
     size64_t *size,
     libcerror_error_t **error )
{
	libvsmbr_internal_partition_t *internal_partition = NULL;
	static char *function                             = "libvsmbr_partition_get_size";
	int result                                        = 1;

	if( partition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid partition.",
		 function );

		return( -1 );
	}
	internal_partition = (libvsmbr_internal_partition_t *) partition;

#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libvsmbr_partition_values_get_size(
	     internal_partition->partition_values,
	     size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve size.",
		 function );

		result = -1;
	}
#if defined( HAVE_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_partition->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

