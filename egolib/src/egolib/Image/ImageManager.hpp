//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

#pragma once

#include "egolib/platform.h"
#include "egolib/vfs.h"
#include "egolib/Image/ImageLoader.hpp"

/**
 * @brief
 *  An image manager.
 * @author
 *  Michael Heilmann
 * @todo
 *  The image manager currently abstracts away the SDL_image/SDL image loading facilities.
 *  It is - in the end - just a minor improvement over the previous code, just enough to get going.
 */
class ImageManager
{
private:
    typedef std::vector<std::unique_ptr<ImageLoader>> Vector;
    /// Vector of available image loaders, ordered by priority from highest to lowest.
    Vector _loaders;
    
    struct Iterator : public std::iterator<std::forward_iterator_tag, ImageLoader>
    {
        ImageManager::Vector::const_iterator _inner;
    public:
        Iterator(const ImageManager::Vector::const_iterator& inner) :
            _inner(inner)
        {}
    public:
        // Forward iterator.

        // Prefix increment.
        Iterator& operator++()
        {
            ++_inner;
            return *this;
        }

        // Postfix increment.
        Iterator operator++(int)
        {
            Iterator copy = *this;
            ++_inner;
            return copy;
        }

    public:

        // Any iterator.

        Iterator() :
            _inner()
        {}

        Iterator(const Iterator& other) :
            _inner(other._inner)
        {}

        Iterator& operator=(const Iterator& other)
        {
            _inner = other._inner;
            return *this;
        }

        bool operator==(const Iterator& other) const
        {
            return _inner == other._inner;
        }

        bool operator!=(const Iterator& other) const
        {
            return _inner != other._inner;
        }

        reference operator*() const
        {
            auto x = (*_inner).get();
            return *x;
        }

        reference operator->() const
        {
            auto x = (*_inner).get();
            return *x;
        }
    };

private:
    /**
     * @brief
     *  The image manager singleton.
     * @remark
     *  Intentionally private.
     */
    static ImageManager *_singleton;

    /**
     * @brief
     *  If the image manager was initialized with SDL_image support.
     */
    bool _withSDL_image;
    
    /**
     * @brief
     *  Construct this image manager.
     * @remark
     *  Intentionally private.
     */
    ImageManager();
    
    /**
     * @brief
     *  Destruct this image manager.
     * @remark
     *  Intentionally private.
     */
    virtual ~ImageManager();

public:
    /**
     * @brief
     *  Get an iterator pointing to the first loader supporting one of the specified extensions
     *  if such a loader exists, <tt>end()</tt> otherwise. The search range is <tt>[start, end())</tt>.
     */
    Iterator find(std::unordered_set<std::string> extensions, Iterator start)
    {
        auto it = start;
        while (it != end())
        {
            auto found = extensions.find((*it).getExtension());
            if (found != extensions.end())
            {
                return it;
            }
        }
        return end();
    }

    /**
     * @brief
     *  Get an iterator pointing to the first loader supporting one of the specified extensions
     *  if such a loader exists, <tt>end()</tt> otherwise. The search range is <tt>[start(), end())</tt>.
     * @remark
     *  <tt>o.find(s)</tt> is equivalent to <tt>o.find(s,o.begin())</tt>.
     */
    Iterator find(std::unordered_set<std::string> extensions)
    {
        return find(extensions, begin());
    }
    
    /**
     * @brief
     *  Get an iterator pointing to the beginning of the loader list.
     * @return
     *  an iterator pointing to the beginning of the loader list
     *  (<tt>end</tt> is returned if the loader list is empty).
     */
    Iterator begin() const
    {
        return Iterator(_loaders.begin());
    }
    
    /**
     * @brief
     *  Get an iterator pointing to the end of the loader list.
     * @return
     *  an iterator pointing to the end of the loader list
     */
    Iterator end() const
    {
        return Iterator(_loaders.end());
    }
    
    /**
     * @brief
     *  Initialize the image manager singleton.
     * @remark
     *  No effect if the image manager is initialized.
     */
    static void initialize();
    
    /**
     * @brief
     *  Destruct the image manager singleton.
     * @remark
     *  No effect if the image manager is not initialized.
     */
    static void uninitialize();
    
    /**
     * @brief
     *  Get the image manager singleton.
     * @return
     *  the image loader singleton
     * @throw logic_error
     *  if the image manager singleton is not initialized.
     */
    static ImageManager& get();

};
