//********************************************************************************************
//*
//*    This file is part of the opengl extensions library. This library is
//*    distributed with Egoboo.
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

/// @file egolib/Extensions/ogl_texture.c
/// @ingroup _ogl_extensions_
/// @brief Implements OpenGL texture loading using SDL_image
/// @details Basic loading and managing OpenGL textures.
///   Uses SDL_image to load .tif, .png, .bmp, .dib, .xpm, and other formats into OpenGL texures

#include "egolib/Extensions/ogl_texture.h"
#include "egolib/Extensions/ogl_debug.h"
#include "egolib/Extensions/SDL_GL_extensions.h"
#include "egolib/Math/_Include.hpp"
#include "egolib/Graphics/TextureManager.hpp"

#include "egolib/egoboo_setup.h"
#include "egolib/strutil.h"
#include "egolib/Image/Image.hpp"
#include "egolib/vfs.h"

bool IMG_test_alpha(SDL_Surface *surface)
{
    // test to see whether an image requires alpha blending

    if (!surface)
    {
        throw std::invalid_argument("nullptr == surface");
    }

    // Alias.
    SDL_PixelFormat *format = surface->format;

    // If the surface has a per-surface color key, it is partially transparent.
    Uint32 colorKey;
    int rslt = SDL_GetColorKey(surface, &colorKey);
    if (rslt < -1)
    {
        // If a value smaller than -1 is returned, an error occured.
        throw std::invalid_argument("SDL_GetColorKey failed");
    }
    else if (rslt >= 0)
    {
        // If a value greater or equal than 0 is returned, the surface has a color key.
        return true;
    }
    /*
    else if (rslt == -1)
    {
    // If a value of -1 is returned, the surface has no color key: Continue.
    }
    */

    // If it is an image without an alpha channel, then there is no alpha in the image.
    if (!format->palette && 0x00 == format->Amask)
    {
        return false;
    }

    // grab the info for scanning the surface
    Uint32 bitMask = format->Rmask | format->Gmask | format->Bmask | format->Amask;
    int bytesPerPixel = format->BytesPerPixel;
    int width = surface->w;
    int height = surface->h;
    int pitch = surface->pitch;

    const char *row_ptr = static_cast<const char *>(surface->pixels);
    for (int y = 0; y < height; ++y)
    {
        const char *char_ptr = row_ptr;
        for (int x = 0; x < width; ++x)
        {
            Uint32 *ui32_ptr = (Uint32 *)char_ptr;
            Uint32 pixel = (*ui32_ptr) & bitMask;
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, format, &r, &g, &b, &a);

            if (0xFF != a)
            {
                return true;
            }
            char_ptr += bytesPerPixel;
        }
        row_ptr += pitch;
    }

    return false;
}

bool IMG_test_alpha_key(SDL_Surface *surface, Uint32 key)
{
    // test to see whether an image requires alpha blending
    if (!surface)
    {
        return false;
    }
    // save this alias
    SDL_PixelFormat *format = surface->format;

    // If there is no key specified (or the image has an alpha channel), use the basic version.
    if (INVALID_KEY == key || format->Aloss < 8)
    {
        return IMG_test_alpha(surface);
    }

    // If the overall alpha marks the surface as not fully opaque,
    // it is partially transparent and hence requires alpha blending.
    if (0xff != format->alpha)
    {
        return true;
    }

    // grab the info for scanning the surface
    Uint32 bitMask = format->Rmask | format->Gmask | format->Bmask | format->Amask;
    int bytesPerPixel = format->BytesPerPixel;
    int w = surface->w;
    int h = surface->h;
    int pitch = surface->pitch;

    const char *row_ptr = static_cast<const char *>(surface->pixels);
    for (int iy = 0; iy < h; iy++)
    {
        const char *char_ptr = row_ptr;
        for (int ix = 0; ix < w; ix++)
        {
            Uint32 *ui32_ptr = (Uint32 *)char_ptr;
            Uint32 pixel = (*ui32_ptr) & bitMask;

            if (pixel == key)
            {
                return true;
            }
            else
            {
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
                if (0xFF != a)
                {
                    return true;
                }
            }

            // advance to the next entry
            char_ptr += bytesPerPixel;
        }
        row_ptr += pitch;
    }

    return false;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

struct CErrorImage
{
private:
    const static size_t width = 2;
    const static size_t height = 2;
    GLubyte _bytes[height][width][4];
public:
    size_t getWidth() const throw()
        { return width; }
    
    size_t getHeight() const throw()
        { return height; }
    
    const GLubyte *getBytes() const throw()
        { return &(_bytes[0][0][0]); }
public:
    CErrorImage() throw()
    {
        for (size_t i = 0; i < height; i++)
        {
            for (size_t j = 0; j < width; j++)
            {
                if (0 == ((i & 0x1) ^ (j & 0x1)))
                {
                    _bytes[i][j][0] = (GLubyte)255;
                    _bytes[i][j][1] = (GLubyte)0;
                    _bytes[i][j][2] = (GLubyte)0;
                }
                else
                {
                    _bytes[i][j][0] = (GLubyte)0;
                    _bytes[i][j][1] = (GLubyte)255;
                    _bytes[i][j][2] = (GLubyte)255;
                }

                _bytes[i][j][3] = (GLubyte)255;
            }
        }
    }
};

struct CErrorTexture
{
    /// The OpenGL texture target of this error texture.
    GLenum _target;
    /// The OpenGL ID of this error texture.
    GLuint _id;
    char *_name;
    const char *getName()
    {
        return _name;
    }
    int getSourceWidth()
    {
        return 2;
    }
    int getSourceHeight()
    {
        return 2;
    }
    int getWidth()
    {
        return 2;
    }
    int getHeight()
    {
        return 2;
    }
    // Construct this error texture.
    CErrorTexture(const char *name, GLenum target)
    {
        _name = strdup(name);
        if (!_name)
        {
            throw std::runtime_error("unable to create error texture");
        }
        _target = target;
        if (_target != GL_TEXTURE_1D && _target != GL_TEXTURE_2D)
        {
            free(_name);
            throw std::invalid_argument("invalid texture target");
        }
        while (GL_NO_ERROR != glGetError())
        {
            /* Nothing to do. */
        }
        glGenTextures(1, &_id);
        if (GL_NO_ERROR != glGetError())
        {
            free(_name);
            throw std::runtime_error("unable to create error texture");
        }
        glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


        glBindTexture(target, _id);
        if (GL_NO_ERROR != glGetError())
        {
            free(_name);
            glDeleteTextures(1, &_id);
            throw std::runtime_error("unable to bind error texture");
        }

        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (GL_NO_ERROR != glGetError())
        {
            free(_name);
            glDeleteTextures(1, &_id);
            throw std::runtime_error("unable to set error texture parameters");
        }
        CErrorImage errorImage;
        // Create the image.
        if (target == GL_TEXTURE_1D)
        {
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, errorImage.getWidth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, errorImage.getBytes());
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, errorImage.getWidth(), errorImage.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, errorImage.getBytes());
        }

        glPopClientAttrib();
        if (GL_NO_ERROR != glGetError())
        {
            free(_name);
            glDeleteTextures(1, &_id);
            throw std::runtime_error("unable to upload error image into error texture");
        }
    }
    // Destruct this error texture.
    ~CErrorTexture()
    {
        glDeleteTextures(1, &_id);
    }
};

static CErrorTexture *_errorTexture1D = nullptr;
static CErrorTexture *_errorTexture2D = nullptr;

void initializeErrorTextures()
{
    if (!_errorTexture1D)
    {
        _errorTexture1D = new CErrorTexture("<error texture 1D>", GL_TEXTURE_1D);
    }
    if (!_errorTexture2D)
    {
        try
        {
            _errorTexture2D = new CErrorTexture("<error texture 2D>", GL_TEXTURE_2D);
        }
        catch (...)
        {
            delete _errorTexture1D;
            _errorTexture1D = nullptr;
            std::rethrow_exception(std::current_exception());
        }
    }
}

void uninitializeErrorTextures()
{
    delete _errorTexture2D;
    _errorTexture2D = nullptr;
    delete _errorTexture1D;
    _errorTexture1D = nullptr;
}

//--------------------------------------------------------------------------------------------

GLuint get1DErrorTextureID()
{
    return _errorTexture1D->_id;
}

GLuint get2DErrorTextureID()
{
    return _errorTexture2D->_id;
}

bool isErrorTextureID(GLuint id)
{
    return id == _errorTexture1D->_id
        || id == _errorTexture2D->_id;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
oglx_texture_t *oglx_texture_t::ctor(oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }

    // The texture is the 2D error texture.
    self->_type = Ego::TextureType::_2D;
    self->_id = _errorTexture2D->_id;
    // The texture coordinates of this texture are repeated along the s and t axes.
    self->_textureAddressModeS = Ego::TextureAddressMode::Repeat;
    self->_textureAddressModeT = Ego::TextureAddressMode::Repeat;
    // The size (width and height) of this texture is the size of the error image.
    self->_width = _errorTexture2D->getWidth();
    self->_height = _errorTexture2D->getHeight();
    // The size (width and height) the source of this texture is the size of the error image as well.
    self->_sourceWidth = _errorTexture2D->getSourceWidth();
    self->_sourceHeight = _errorTexture2D->getSourceHeight();
    // The texture has the empty string as its name and the source is not available.
    strncpy(self->name, _errorTexture2D->getName(), SDL_arraysize(self->name));
    // The error texture has no source.
    self->source = nullptr;
    // (The error texture has no alpha component).
    self->_hasAlpha = false;

    return self;
}

void oglx_texture_t::dtor(oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    release(self);
}

oglx_texture_t *oglx_texture_t::create()
{
    oglx_texture_t *self = static_cast<oglx_texture_t *>(malloc(sizeof(oglx_texture_t)));
    if (!self)
    {
        throw std::bad_alloc();
    }
    if (!oglx_texture_t::ctor(self))
    {
        free(self);
        throw std::bad_alloc();
    }
    return self;
}

void oglx_texture_t::destroy(oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    oglx_texture_t::dtor(self);
    free(self);
}

//--------------------------------------------------------------------------------------------
GLuint oglx_texture_t::load(oglx_texture_t *self, const std::string& name, SDL_Surface *source, Uint32 key)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }

    // Bind this texture to the backing error texture.
    oglx_texture_t::release(self);

    // If no source is provided, keep this texture bound to the backing error texture.
    if (!source)
    {
        return INVALID_GL_ID;
    }

    /// @todo MH: *sigh* ... just ... do ... not ... do ... that.
    /* Set the color key, if valid. */
    if (nullptr != source->format && nullptr != source->format->palette && INVALID_KEY != key)
    {
        SDL_SetColorKey(source, SDL_SRCCOLORKEY, key);
    }

    // Convert the source into a format suited for OpenGL.
    SDL_Surface *new_source;
    try
    {
        new_source = SDL_GL_convert_surface(source);
    }
    catch (...)
    {
        return INVALID_GL_ID;
    }
    if (!new_source)
    {
        return INVALID_GL_ID;
    }
    // Generate a new OpenGL texture ID.
    Ego::OpenGL::Utilities::clearError();
    GLuint id;
    while (GL_NO_ERROR != glGetError())
    {
        /* Nothing to do. */
    }
    glGenTextures(1, &id);
    if (Ego::OpenGL::Utilities::isError())
    {
        return INVALID_GL_ID;
    }
    // Use default texture address mode.
    auto textureAddressModeS = Ego::TextureAddressMode::Repeat;
    auto textureAddressModeT = Ego::TextureAddressMode::Repeat;
    // Determine the texture type.
    auto type = ((1 == source->h) && (source->w > 1)) ? Ego::TextureType::_1D : Ego::TextureType::_2D;
    // Test if the image requires alpha blending.
    bool hasAlpha = IMG_test_alpha_key(new_source, key);
    /* Set texture address mode and texture filtering. */
    Ego::OpenGL::Utilities::bind(id, type, textureAddressModeS, textureAddressModeT);
    /* Upload the texture data. */
    if (type == Ego::TextureType::_2D)
    {
        if (g_ogl_textureParameters.textureFilter.mipMapFilter > Ego::TextureFilter::None)
        {
            Ego::OpenGL::Utilities::upload_2d_mipmap(true, new_source->w, new_source->h, new_source->pixels);
        }
        else
        {
            Ego::OpenGL::Utilities::upload_2d(true, new_source->w, new_source->h, new_source->pixels);
        }
    }
    else if (type == Ego::TextureType::_1D)
    {
        Ego::OpenGL::Utilities::upload_1d(true, new_source->w, new_source->pixels);
    }
    else
    {
        EGOBOO_ASSERT(0); /// @todo This code is in fact unreachable. Raise a std::runtime_error.
    }

    // Store the appropriate data.
    self->_textureAddressModeS = textureAddressModeS;
    self->_textureAddressModeT = textureAddressModeT;
    self->_type = type;
    self->_id = id;
    self->_width = new_source->w;
    self->_height = new_source->h;
    self->source = source;
    self->_sourceWidth = source->w;
    self->_sourceHeight = source->h;
    self->_hasAlpha = hasAlpha;
    strncpy(self->name, name.c_str(), SDL_arraysize(self->name));
    self->name[SDL_arraysize(self->name) - 1] = '\0';

    SDL_FreeSurface(new_source);

    return self->_id;
}

GLuint oglx_texture_t::load(oglx_texture_t *self, SDL_Surface *source, Uint32 key)
{
    std::ostringstream stream;
    stream << "<source " << (void *)source << ">";
    return oglx_texture_t::load(self, stream.str().c_str(), source, key);
}

GLuint oglx_texture_t::load(oglx_texture_t *self, const std::string& filename, Uint32 key)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    // Release OpenGL/SDL resources and assign the error texture.
    oglx_texture_t::release(self);
    SDL_Surface *image = IMG_Load_RW(vfs_openRWopsRead(filename.c_str()), 1);
    return load(self, filename, image, key);
}

Ego::TextureAddressMode oglx_texture_t::getTextureAddressModeS(oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_textureAddressModeS;
}

Ego::TextureAddressMode oglx_texture_t::getTextureAddressModeT(oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_textureAddressModeT;
}

GLuint  oglx_texture_t::getTextureID(const oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_id;
}

int  oglx_texture_t::getSourceHeight(const oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_sourceHeight;
}

int  oglx_texture_t::getSourceWidth(const oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_sourceWidth;
}

int  oglx_texture_t::getWidth(const oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_width;
}

int  oglx_texture_t::getHeight(const oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_height;
}

bool oglx_texture_t::hasAlpha(const oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    return self->_hasAlpha;
}

void  oglx_texture_t::release(oglx_texture_t *self)
{
    if (!self)
    {
        throw std::invalid_argument("nullptr == self");
    }
    if (isErrorTextureID(self->_id))
    {
        return;
    }

    // Delete the OpenGL texture and assign the error texture.
    glDeleteTextures(1, &(self->_id));
    Ego::OpenGL::Utilities::isError();

    // Delete the source if it exist.
    if (self->source)
    {
        SDL_FreeSurface(self->source);
        self->source = nullptr;
    }

    // The texture is the 2D error texture.
    self->_type = Ego::TextureType::_2D;
    self->_id = _errorTexture2D->_id;
    // The texture coordinates of this texture are repeated along the s and t axes.
    self->_textureAddressModeS = Ego::TextureAddressMode::Repeat;
    self->_textureAddressModeT = Ego::TextureAddressMode::Repeat;
    // The size (width and height) of this texture is the size of the error image.
    self->_width = _errorTexture2D->getWidth();
    self->_height = _errorTexture2D->getHeight();
    // The size (width and height) the source of this texture is the size of the error image as well.
    self->_sourceWidth = _errorTexture2D->getSourceWidth();
    self->_sourceHeight = _errorTexture2D->getSourceHeight();
    // The texture has the empty string as its name and the source is not available.
    strcpy(self->name, _errorTexture2D->getName());
    // (The error texture has no alpha component).
    self->_hasAlpha = false;
}

//--------------------------------------------------------------------------------------------
void oglx_texture_t::bind(oglx_texture_t *texture)
{
    /// @author BB
    /// @details a oglx_texture_t wrapper for oglx_bind_to_tex_params() function

    if (!texture)
    {
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        Ego::OpenGL::Utilities::bind(texture->_id, texture->_type, texture->_textureAddressModeS, texture->_textureAddressModeT);
    }
    Ego::OpenGL::Utilities::isError();
}

