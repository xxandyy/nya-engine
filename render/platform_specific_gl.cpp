//https://code.google.com/p/nya-engine/

#include "platform_specific_gl.h"
#include "render.h"
#include <string>

namespace nya_render
{

#ifndef DIRECTX11
bool has_extension(const char *name)
{
    const char *exts=(const char*)glGetString(GL_EXTENSIONS);
    if(!exts)
        return false;

    if(std::string(exts).find(name)==std::string::npos)
        return false;
    
    return true;
}
#endif

#ifndef NO_EXTENSIONS_INIT

void *get_exact_extension(const char*ext_name)
{
    #ifdef _WIN32
        return (void*)wglGetProcAddress(ext_name);
    #else
        return (void*)glXGetProcAddressARB((const GLubyte *)ext_name);
    #endif
}

void *get_extension(const char*ext_name)
{
    if(!ext_name)
    {
        log()<<"invalid extension name\n";
        return 0;
    }

    void *extention = get_exact_extension(ext_name);
    if(extention)
        return extention;

    const static std::string arb("ARB");
    const std::string ext_name_arb = std::string(ext_name)+arb;
    extention = get_exact_extension(ext_name_arb.c_str());
    if(extention)
        return extention;

    const static std::string ext("EXT");
    const std::string ext_name_ext = std::string(ext_name)+ext;
    extention = get_exact_extension(ext_name_ext.c_str());
    if(!extention)
        log()<<"unable to initialise extension "<<ext_name<<"\n";

    return extention;
}

#endif

namespace { bool ignore_platform_restrictions=false; }
void set_ignore_platform_restrictions(bool ignore) { ignore_platform_restrictions=ignore; }
bool is_platform_restrictions_ignored() { return ignore_platform_restrictions; }
}
