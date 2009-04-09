/*
 * Copyright (C) 2009 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FILESYSTEM_DIRECTORY_H
#define __FILESYSTEM_DIRECTORY_H

#include <api/VMCopy.h>
#include <dirent.h>
#include <List.h>
#include "File.h"

/**
 * In-memory directory object.
 */
class Directory : public File
{
    public:
    
	/**
	 * Constructor function.
	 */
	Directory() : File(DirectoryFile)
	{
	}

	/**
	 * Destructor function.
	 */
	~Directory()
	{
	    entries.clear(true);
	}
    
	/**
	 * Read directory entries.
	 * @param msg Read request.
	 * @return Number of bytes read on success, Error on failure.
	 */
	Error read(FileSystemMessage *msg)
	{
	    Size count = 0, bytes = 0;
	    Dirent *dent = (Dirent *) msg->buffer;
	    Error e;
	
	    /* Loop our list of Dirents. */
	    for (ListIterator<Dirent> i(&entries); i.hasNext(); i++)
	    {
		/* Make sure we start at the correct offset. */
		if (count++ < msg->offset / sizeof(Dirent))
		{
		    continue;
		}
		/* Can we read another entry? */
		else if (bytes + sizeof(Dirent) <= msg->size)
		{
		    if ((e = VMCopy(msg->procID, Write, (Address) i.current(),
					    	        (Address) (dent++), sizeof(Dirent))) < 0)
		    {
			return e;
		    }
		    bytes += e;
		}
		else break;
	    }
	    /* Report results. */
	    msg->size = bytes;
	    return ESUCCESS;
	}

	/**
	 * Adds a directory entry.
	 * @param name Name of the entry to add.
	 * @param type File type.
	 */
	void addEntry(char *name, u8 type)
	{
	    Dirent *d = new Dirent;
	    strlcpy(d->d_name, name, DIRLEN);
	    d->d_type = type;
	    entries.insertTail(d);
	}
	
	/**
	 * Clears the list of entries.
	 */
	void clear()
	{
	    entries.clear(true);
	}

    private:

	/** List of directory entries. */
	List<Dirent> entries;
};

#endif /* __FILESYSTEM_DIRECTORY_H */
