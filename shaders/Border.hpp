#pragma once

#include <string>

// makes a stencil without corners
inline const std::string FRAGBORDER2 = R"#(
precision highp float;
varying vec4 v_color;
varying vec2 v_texcoord;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform vec2 fullSizeUntransformed;
uniform float radius;
uniform float radiusOuter;
uniform float thick;

uniform vec4 gradient[10];
uniform int gradientLength;
uniform float angle;
uniform float alpha;

vec4 getColorForCoord(vec2 normalizedCoord) {
    if (gradientLength < 2)
        return gradient[0];

    float finalAng = 0.0;

    if (angle > 4.71 /* 270 deg */) {
        normalizedCoord[1] = 1.0 - normalizedCoord[1];
        finalAng = 6.28 - angle;
    } else if (angle > 3.14 /* 180 deg */) {
        normalizedCoord[0] = 1.0 - normalizedCoord[0];
        normalizedCoord[1] = 1.0 - normalizedCoord[1];
        finalAng = angle - 3.14;
    } else if (angle > 1.57 /* 90 deg */) {
        normalizedCoord[0] = 1.0 - normalizedCoord[0];
        finalAng = 3.14 - angle;
    } else {
        finalAng = angle;
    }

    float sine = sin(finalAng);

    float progress = (normalizedCoord[1] * sine + normalizedCoord[0] * (1.0 - sine)) * float(gradientLength - 1);
    int bottom = int(floor(progress));
    int top = bottom + 1;

    return gradient[top] * (progress - float(bottom)) + gradient[bottom] * (float(top) - progress);
}

void main() {

    highp vec2 pixCoord = vec2(gl_FragCoord);
    highp vec2 pixCoordOuter = pixCoord;
    highp vec2 originalPixCoord = v_texcoord;
    originalPixCoord *= fullSizeUntransformed;
    float additionalAlpha = 1.0;
    float sqrt2 = 1.4142135;

    vec4 pixColor = vec4(1.0, 1.0, 1.0, 1.0);

    bool done = false;

    pixCoord -= topLeft + fullSize * 0.5;
    pixCoord *= vec2(lessThan(pixCoord, vec2(0.0))) * -2.0 + 1.0;

    // center the pixes dont make it top-left
    pixCoord += vec2(1.0, 1.0) / fullSize;
    pixCoordOuter = pixCoord;

    pixCoord -= fullSize * 0.5 - radius;
    pixCoordOuter -= fullSize * 0.5 - radiusOuter;

    if (min(pixCoord.x, pixCoord.y) > (-sqrt2 + 1.0) * thick && radius > 0.0) {
        float s = pixCoord.x + pixCoord.y;
        float sO = pixCoordOuter.x + pixCoordOuter.y;
        if (s < radius) {
            float normalized = smoothstep(1.0, 0.0, (radius - thick * sqrt2) - s);
            additionalAlpha *= normalized;
        } else
            additionalAlpha = 0.0;
        done = true;
    } 

    // now check for other shit
    if (!done) {
        // distance to all straight bb borders
        float distanceT = originalPixCoord[1];
        float distanceB = fullSizeUntransformed[1] - originalPixCoord[1];
        float distanceL = originalPixCoord[0];
        float distanceR = fullSizeUntransformed[0] - originalPixCoord[0];

        // get the smallest
        float smallest = min(min(distanceT, distanceB), min(distanceL, distanceR));
        
        if (smallest > thick)
            discard;
    }

    if (additionalAlpha == 0.0)
        discard;

    pixColor = getColorForCoord(v_texcoord);
    pixColor.rgb *= pixColor[3];

    pixColor *= alpha * additionalAlpha;

    gl_FragColor = pixColor;
}
)#";
