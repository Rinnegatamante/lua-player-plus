/*
 * Zip.h: Header for Zip file access
 *
 * This file is part of "Phoenix Game Engine".
 *
 * Copyright (C) 2008 Phoenix Game Engine
 * Copyright (C) 2008 InsertWittyName <tias_dp@hotmail.com>
 * Copyright (C) 2008 MK2k <@mk2k.net>
 *
 * Phoenix Game Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 
 * Phoenix Game Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with Phoenix Game Engine.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __ZIP_H__
#define __ZIP_H__

/** @defgroup Zip Zip Library
 *  @{
 */

#include <zlib.h>

/**
 * A zip
 */
typedef void Zip;

/**
 * A file within a zip
 */
typedef struct
{
	unsigned char *data;	/**<  The file data */
	int size;				/**<  Size of the data */
	
} ZipFile;

/**
 * Open a Zip file
 *
 * @param filename - Path of the zip to load.
 *
 * @returns A pointer to a ::Zip struct or NULL on error.
 */
Zip* ZipOpen(const char *filename);

/**
 * Close a Zip file
 *
 * @param zip - A valid (previously opened) ::Zip
 *
 * @returns 1 on success, 0 on error
 */
int ZipClose(Zip *zip);

/**
 * Read a file from a zip
 *
 * @param zip - A valid (previously opened) ::Zip
 *
 * @param filename - The file to read within the zip
 *
 * @param password - The password of the file (pass NULL if no password)
 *
 * @returns A ::ZipFile struct containing the file
 */
ZipFile* ZipFileRead(Zip *zip, const char *filename, const char *password);

/**
 * Extract all files from a zip
 *
 * @param zip - A valid (previously opened) ::Zip file
 *
 * @param password - The password of the file (pass NULL if no password)
 *
 * @returns 1 on success, 0 on error.
 */
int ZipExtract(Zip *zip, const char *password);

/**
 * Free the file data previously loaded from a zip
 *
 * @param file - A valid (previously read) ::ZipFile
 */
void ZipFileFree(ZipFile *file);

/** @} */

#endif // __ZIP_H__