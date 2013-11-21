//https://code.google.com/p/nya-engine/

#include "fbo.h"
#include "platform_specific_gl.h"

namespace
{
#ifdef DIRECTX11
#else
    int default_fbo_idx=0;
#endif
}

namespace nya_render
{
    struct fbo_obj
    {
        int color_tex_idx;
        int depth_tex_idx;

#ifdef DIRECTX11
#else
        unsigned int fbo_idx;

        unsigned int color_target_idx;
        unsigned int color_gl_target;
        unsigned int depth_target_idx;

        fbo_obj(): color_tex_idx(-1),depth_tex_idx(-1),fbo_idx(0),color_target_idx(0),color_gl_target(GL_TEXTURE_2D),depth_target_idx(0) {}
#endif

    public:
        static int add() { return get_fbo_objs().add(); }
        static fbo_obj &get(int idx) { return get_fbo_objs().get(idx); }
        static void remove(int idx) { return get_fbo_objs().remove(idx); }

    private:
        typedef render_objects<fbo_obj> fbo_objs;
        static fbo_objs &get_fbo_objs()
        {
            static fbo_objs objs;
            return objs;
        }
    };

#ifndef DIRECTX11
  #ifdef NO_EXTENSIONS_INIT
    #define fbo_glGenFramebuffers glGenFramebuffers
    #define fbo_glBindFramebuffer glBindFramebuffer
	#define fbo_glDeleteFramebuffers glDeleteFramebuffers
	#define fbo_glFramebufferTexture2D glFramebufferTexture2D
  #else
    PFNGLGENFRAMEBUFFERSPROC fbo_glGenFramebuffers;
	PFNGLBINDFRAMEBUFFERPROC fbo_glBindFramebuffer;
	PFNGLDELETEFRAMEBUFFERSPROC fbo_glDeleteFramebuffers;
	PFNGLFRAMEBUFFERTEXTURE2DPROC fbo_glFramebufferTexture2D;
  #endif

bool check_init_fbo()
{
    static bool initialised=false;
    static bool failed=true;
    if(initialised)
        return !failed;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&default_fbo_idx);

    //if(!has_extension("GL_EXT_framebuffer_object"))
    //    return false;

  #ifndef NO_EXTENSIONS_INIT
    fbo_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)get_extension("glGenFramebuffers");
    if(!fbo_glGenFramebuffers)
        return false;

	fbo_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)get_extension("glBindFramebuffer");
    if(!fbo_glBindFramebuffer)
        return false;

	fbo_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)get_extension("glDeleteFramebuffers");
    if(!fbo_glDeleteFramebuffers)
        return false;

	fbo_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)get_extension("glFramebufferTexture2D");
    if(!fbo_glFramebufferTexture2D)
        return false;
  #endif

    initialised=true;
    failed=false;

    return true;
}
#endif

void fbo::set_color_target(const texture &tex,cubemap_side side)
{
    if(m_fbo_idx<0)
        m_fbo_idx=fbo_obj::add();

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);
    fbo.color_tex_idx=tex.m_tex;

#ifndef DIRECTX11
    if(!fbo.fbo_idx)
    {
        if(!check_init_fbo())
            return;

        fbo_glGenFramebuffers(1,&fbo.fbo_idx);
    }
#endif

#ifdef DIRECTX11
#else
    if(!fbo.fbo_idx)
        return;

    fbo.color_gl_target=GL_TEXTURE_2D;
    switch(side)
    {
        case cube_positive_x: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
        case cube_negative_x: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
        case cube_positive_y: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
        case cube_negative_y: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
        case cube_positive_z: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
        case cube_negative_z: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
        default: break;
    }
#endif
}

void fbo::set_color_target(const texture &tex) { set_color_target(tex,cubemap_side(-1)); }

void fbo::set_depth_target(const texture &tex)
{
    if(m_fbo_idx<0)
        m_fbo_idx=fbo_obj::add();

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);
    fbo.depth_tex_idx=tex.m_tex;

#ifndef DIRECTX11
    if(!fbo.fbo_idx)
    {
        if(!check_init_fbo())
            return;
        
        fbo_glGenFramebuffers(1,&fbo.fbo_idx);
    }
#endif
}

void fbo::release()
{
	if(m_fbo_idx<0)
		return;

    const fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

#ifdef DIRECTX11
#else
    if(fbo.fbo_idx)
    {
        fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
        fbo_glDeleteFramebuffers(1,&fbo.fbo_idx);
    }
#endif

    fbo_obj::remove(m_fbo_idx);
    m_fbo_idx=-1;
}

void fbo::bind()
{
	if(m_fbo_idx<0)
		return;

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

#ifdef DIRECTX11
#else
    if(!fbo.fbo_idx)
    {
        fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
        return;
    }

    fbo_glBindFramebuffer(GL_FRAMEBUFFER,fbo.fbo_idx);

    if(fbo.color_tex_idx>=0)
    {
        const texture_obj &tex=texture_obj::get(fbo.color_tex_idx);

        const bool color_target_was_invalid=fbo.color_target_idx==0;

        if(fbo.color_target_idx!=tex.tex_id)
        {
            if(fbo.color_gl_target==GL_TEXTURE_2D)
            {
                if(tex.gl_type==GL_TEXTURE_2D)
                    fbo.color_target_idx=tex.tex_id;
                else
                    fbo.color_target_idx=0;
            }
            else
            {
                if(tex.gl_type!=GL_TEXTURE_2D)
                    fbo.color_target_idx=tex.tex_id;
                else
                    fbo.color_target_idx=0;
            }
        }
        else
        {
            //ToDo: check if tex.gl_type changed
        }

        if(color_target_was_invalid && fbo.color_target_idx && fbo.depth_target_idx)
        {
#ifndef OPENGL_ES
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
#endif
        }

        fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,fbo.color_gl_target,fbo.color_target_idx,0);
    }
    else if(fbo.color_target_idx)
    {
        fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,0,0);
        fbo.depth_tex_idx=0;
    }

    if(fbo.depth_tex_idx>=0)
    {
        const texture_obj &tex=texture_obj::get(fbo.depth_tex_idx);
        if(fbo.depth_target_idx!=tex.tex_id)
        {
            if(tex.gl_type!=GL_TEXTURE_2D)
            {
                if(fbo.depth_target_idx)
                {
                    fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,tex.tex_id,0);
                    fbo.depth_target_idx=0;
                }
            }
            else
            {
                fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,tex.tex_id,0);
                fbo.depth_target_idx=tex.tex_id;
#ifndef OPENGL_ES
                if(!fbo.color_target_idx && fbo.depth_target_idx)
                {
                    glDrawBuffer(GL_NONE);
                    glReadBuffer(GL_NONE);
                }
#endif
            }
        }
    }
    else if(fbo.depth_target_idx)
    {
        fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,0,0);
        fbo.depth_target_idx=0;
    }

#endif
}

void fbo::unbind()
{
#ifdef DIRECTX11
#else
    fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
#endif
}

}
