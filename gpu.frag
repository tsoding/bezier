#version 130

#define MARKER_COLOR vec4(0.75, 0.0, 0.0, 1.0)
#define BEZIER_CURVE_COLOR vec3(0.0, 0.75, 0.0)

// TODO: at some combinations of p1, p2, p3 nothing is drawn
// TODO: explore math for 4 control points
uniform vec2 p1;
uniform vec2 p2;
uniform vec2 p3;
uniform float marker_radius;
uniform float bezier_curve_threshold;

void main()
{
    if (length(gl_FragCoord.xy - p1) < marker_radius ||
        length(gl_FragCoord.xy - p2) < marker_radius ||
        length(gl_FragCoord.xy - p3) < marker_radius)
    {
        gl_FragColor = MARKER_COLOR;
    }
    else
    {
        // TODO: document this method of rendering in a LaTeX document
        vec2 p0 = gl_FragCoord.xy;
        float a = p3.x - 2 * p2.x + p1.x;
        float b = 2 * (p2.x - p1.x);
        float c = p1.x - p0.x;
        float dx = b * b - 4.0f * a * c;

        if (dx >= 0.0f) {
            float t1 = (-b + sqrt(dx)) / (2 * a);
            float t2 = (-b - sqrt(dx)) / (2 * a);
            float y1 = p1.y + 2 * t1 * (p2.y - p1.y) + t1 * t1 * (p3.y - 2 * p2.y + p1.y);
            float y2 = p1.y + 2 * t2 * (p2.y - p1.y) + t2 * t2 * (p3.y - 2 * p2.y + p1.y);
            if ((0.0f <= t1 && t1 <= 1.0f && abs(p0.y - y1) < bezier_curve_threshold) ||
                (0.0f <= t2 && t2 <= 1.0f && abs(p0.y - y2) < bezier_curve_threshold))
            {
                float r1 = abs(p0.y - y1) / bezier_curve_threshold;
                float r2 = abs(p0.y - y2) / bezier_curve_threshold;

                if (0.0 <= r1 && r1 <= 1.0) {
                    gl_FragColor = vec4(0.0, 0.75, 0.0, mix(1.0, 0.0, r1));
                }

                if (0.0 <= r2 && r2 <= 1.0) {
                    gl_FragColor = vec4(0.0, 0.75, 0.0, mix(1.0, 0.0, r2));
                }
            }
            else
            {
                gl_FragColor = vec4(0.0);
            }
        }
        else
        {
            gl_FragColor = vec4(0.0);
        }
    }
}
