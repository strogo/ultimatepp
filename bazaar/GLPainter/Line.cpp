#include "GLPainter.h"

namespace Upp {

void GLDrawPolylines(const GLContext2D& dd, Pointf at, const GLVertexData& mesh, Sizef scale,
                    double width, Color color, double alpha)
{
	static GLCode program(R"(
		#version 330 core
	    in      vec4 pos;
		uniform vec2 offset;
		uniform vec2 scale;
		uniform vec2 scale2;
		uniform vec2 width;
	    void main()
	    {
			vec2 v = scale2 * pos.zw;
			gl_Position = vec4(scale * (scale2 * pos.xy + width * normalize(vec2(-v.y, v.x))) + offset, 0, 1);
	    }
	)", R"(
		#version 330 core
		uniform vec4 color;
		void main()
		{
			gl_FragColor = color;
		}
	)");

	static int ioffset = program["offset"];
	static int iscale = program["scale"];
	static int iscale2 = program["scale2"];
	static int iwidth = program["width"];
	static int icolor = program["color"];

	alpha *= dd.alpha;
	program(ioffset, Pointf(dd.vs) * at + Sizef(-1, 1))
	       (iscale, dd.vs)
	       (iscale2, scale)
	       (iwidth, Sizef(width / 2, width / 2))
	       (icolor, color, alpha)
	;
	
	if(alpha == 1) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		mesh.Draw(program);
	}
	else {
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
	
		glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_NEVER, 1, 1);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		
		mesh.Draw(program);
		
		GLDrawStencil(color, dd.alpha * alpha);
	}
}

};