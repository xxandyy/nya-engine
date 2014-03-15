//https://code.google.com/p/nya-engine/

#pragma once

#include "math/matrix.h"

namespace nya_render
{

class transform
{
public:
    void set_projection_matrix(const nya_math::mat4 &mat);
    void set_modelview_matrix(const nya_math::mat4 &mat);
    void set_orientation_matrix(const nya_math::mat4 &mat);

public:
    const nya_math::mat4 &get_projection_matrix() { return m_has_orientation?m_orientated_proj:m_projection; }
    const nya_math::mat4 &get_modelview_matrix() { return m_modelview; }
    const nya_math::mat4 &get_modelviewprojection_matrix();

public:
    static transform &get()
    {
        static transform tr;
        return tr;
    }

    transform(): m_has_orientation(false),m_recalc_mvp(false) {}

private:
    nya_math::mat4 m_projection;
    nya_math::mat4 m_modelview;

    bool m_has_orientation;
    nya_math::mat4 m_orientation;
    nya_math::mat4 m_orientated_proj;

    bool m_recalc_mvp;
    nya_math::mat4 m_modelviewproj;
};

}
