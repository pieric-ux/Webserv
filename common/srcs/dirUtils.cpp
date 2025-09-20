// TODO: Don't forget to add 42 header !

/**
 * @file DirUtils.cpp
 * @brief
 */

#include "dirUtils.hpp"
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <stdexcept>
#include <string>

namespace utils
{

/**
 * @brief 
 *
 * @param filename 
 */
Directory::Directory(const std::string &filename) : dir(NULL)
{
	errno = 0;
	this->dir = opendir(filename.c_str());
	if (!dir)
		throw std::runtime_error("Unable to open directory " + filename + ": " + strerror(errno));
}

/**
 * @brief 
 */
Directory::~Directory()
{
	if (this->dir)
		closedir(this->dir);
}

/**
 * @brief 
 *
 * @return 
 */
DirectoryIterator	Directory::begin()
{
	return (DirectoryIterator(this));
}

/**
 * @brief 
 *
 * @return 
 */
DirectoryIterator	Directory::end()
{
	return (DirectoryIterator());
}

/**
 * @brief 
 *
 * @return 
 */
DIR *Directory::getDir() const
{
	return (this->dir);
}

/**
 * @brief 
 */
DirectoryIterator::DirectoryIterator() : dir(NULL), entry(NULL) {}

/**
 * @brief 
 *
 * @param init_dir 
 */
DirectoryIterator::DirectoryIterator(Directory *init_dir) : dir(init_dir), entry(NULL)
{
	if (this->dir)
		++(*this);
}

/**
 * @brief 
 */
DirectoryIterator::~DirectoryIterator() {}

/**
 * @brief 
 *
 * @param rhs 
 */
DirectoryIterator::DirectoryIterator(const DirectoryIterator &rhs) : dir(rhs.dir), entry(rhs.entry) {}

/**
 * @brief 
 *
 * @param rhs 
 * @return 
 */
DirectoryIterator	&DirectoryIterator::operator=(const DirectoryIterator &rhs)
{
	if (this != &rhs)
	{
		this->dir = rhs.dir;
		this->entry = rhs.entry;
	}
	return (*this);
}

/**
 * @brief 
 *
 * @return 
 */
struct dirent *DirectoryIterator::operator*() const
{
	return (this->entry);
}

/**
 * @brief 
 *
 * @return 
 */
DirectoryIterator &DirectoryIterator::operator++()
{
	if(this->dir->getDir())
	{
		errno = 0;
		this->entry = readdir(this->dir->getDir());
		if (!entry && errno)
			throw std::runtime_error("Unable to read directory: " + std::string(strerror(errno)));
	}
	return (*this);
}

/**
 * @brief 
 *
 * @return 
 */
DirectoryIterator DirectoryIterator::operator++(int)
{
	DirectoryIterator tmp = *this;
	++(*this);
	return (tmp);
}

/**
 * @brief 
 *
 * @param rhs 
 * @return 
 */
bool DirectoryIterator::operator!=(const DirectoryIterator &rhs) const
{
	return (this->entry != rhs.entry);
}

} // !utils
