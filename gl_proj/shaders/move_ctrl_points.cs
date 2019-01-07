#version 430 core

layout (local_size_x = 32, local_size_y = 32) in;

layout(binding = 1, rgba32f) readonly uniform image2D i_image;
layout(binding = 2) writeonly uniform image2D o_image;

struct Circle
{
    vec2 center;
    float radius;
};

uniform Circle circle;

void main(){
    float PI = 3.14159;
    ivec2 index;
    vec4 N, S, E, W, P;
    vec2 new_pos, E_w, W_e, S_n, N_s, dir_fromO;
    float new_theta, new_r;
    index = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_LocalInvocationID.xy);
    P = imageLoad(i_image, index);
    if (index.x != 0 && index.x != imageSize(i_image).x - 1 && index.y != 0 && index.y != imageSize(i_image).y - 1)
    {
        N = imageLoad(i_image, ivec2(index.x, index.y + 1));
        S = imageLoad(i_image, ivec2(index.x, index.y - 1));
        E = imageLoad(i_image, ivec2(index.x + 1, index.y));
        W = imageLoad(i_image, ivec2(index.x - 1, index.y));
        new_pos = 0.25 * (N.xy + S.xy + E.xy + W.xy);
        float theta1 = acos(dot(normalize(N.xy - S.xy), vec2(1, 0))) - PI / 2.0;
        float theta2 = acos(dot(normalize(E.xy - W.xy), vec2(1, 0)));
        new_theta = (theta1 + theta2) / 2.0;
        new_r = 0.25 * (length(N.xy - new_pos) + length(S.xy - new_pos) + length(E.xy - new_pos) + length(W.xy - new_pos));
    }
    else if (index.x == 0 && index.y != 0 && index.y != imageSize(i_image).y - 1)
    // No W
    {
        N = imageLoad(i_image, ivec2(index.x, index.y + 1));
        S = imageLoad(i_image, ivec2(index.x, index.y - 1));
        E = imageLoad(i_image, ivec2(index.x + 1, index.y));
        E_w = E.xy + (sin(E.z - PI / 2.0) * vec2(1.0, 0) + cos(E.z - PI / 2.0) * vec2(0, 1.0)) * E.w;
        dir_fromO = normalize(E_w - circle.center);
        new_pos = circle.center + circle.radius * dir_fromO;
        float theta1 = acos(dot(normalize(N.xy - S.xy), vec2(1, 0))) - PI / 2.0;
        float theta2 = acos(dot(normalize(P.xy - W.xy), vec2(1, 0)));
        new_theta = (theta1 + theta2) / 2.0;
        new_r = 0.333 * (length(N.xy - new_pos) + length(S.xy - new_pos) + length(E.xy - new_pos));
    }
    else if (index.x == imageSize(i_image).x - 1 && index.y != 0 && index.y != imageSize(i_image).y - 1)
    // No E
    {
        N = imageLoad(i_image, ivec2(index.x, index.y + 1));
        S = imageLoad(i_image, ivec2(index.x, index.y - 1));
        W = imageLoad(i_image, ivec2(index.x - 1, index.y));
        W_e = W.xy + (sin(W.z) * vec2(1.0, 0) + cos(W.z) * vec2(0, 1.0)) * W.w;
        dir_fromO = normalize(W_e - circle.center);
        new_pos = circle.center + circle.radius * dir_fromO;
        float theta1 = acos(dot(normalize(N.xy - S.xy), vec2(1, 0))) - PI / 2.0;
        float theta2 = acos(dot(normalize(P.xy - W.xy), vec2(1, 0)));
        new_theta = (theta1 + theta2) / 2.0;
        new_r = 0.333 * (length(N.xy - new_pos) + length(S.xy - new_pos) + length(new_pos - W.xy));
    }
    else if (index.y == imageSize(i_image).y - 1)
    // No N
    {
        S = imageLoad(i_image, ivec2(index.x, index.y - 1));
        S_n = S.xy + (sin(S.z + PI / 2.0) * vec2(1.0, 0) + cos(S.z + PI / 2.0) * vec2(0, 1.0)) * S.w;
        dir_fromO = normalize(S_n - circle.center);
        new_pos = circle.center + circle.radius * dir_fromO;
        float theta1 = acos(dot(normalize(N.xy - P.xy), vec2(1, 0))) - PI / 2.0;
        float theta2;
        // No E
        if (index.x == imageSize(i_image).x - 1)
        {
            W = imageLoad(i_image, ivec2(index.x - 1, index.y));
            theta2 = acos(dot(normalize(new_pos - W.xy), vec2(1, 0)));
            new_r = 0.5 * (length(S.xy - new_pos) + length(W.xy - new_pos));
        }
        // No W
        else if (index.x == 0)
        {
            E = imageLoad(i_image, ivec2(index.x + 1, index.y));
            theta2 = acos(dot(normalize(new_pos - W.xy), vec2(1, 0)));
            new_r = 0.5 * (length(S.xy - new_pos) + length(E.xy - new_pos) + length(W.xy - new_pos));
        }
        else
        {
            W = imageLoad(i_image, ivec2(index.x - 1, index.y));
            E = imageLoad(i_image, ivec2(index.x + 1, index.y));
            float theta2 = acos(dot(normalize(E.xy - W.xy), vec2(1, 0)));
            new_r = 0.333 * (length(S.xy - new_pos) + length(E.xy - new_pos) + length(W.xy - new_pos));
        }
        new_theta = (theta1 + theta2) / 2.0;
    }
    else if (index.y == 0)
    // No S
    {
        N = imageLoad(i_image, ivec2(index.x, index.y + 1));
        N_s = N.xy + (sin(N.z - PI / 2.0) * vec2(1.0, 0) + cos(N.z - PI / 2.0) * vec2(0, 1.0)) * N.w;
        dir_fromO = normalize(N_s - circle.center);
        new_pos = circle.center + circle.radius * dir_fromO;
        float theta1 = acos(dot(normalize(N.xy - P.xy), vec2(1, 0))) - PI / 2.0;
        float theta2;
        // No E
        if (index.x == imageSize(i_image).x - 1)
        {
            W = imageLoad(i_image, ivec2(index.x - 1, index.y));
            theta2 = acos(dot(normalize(new_pos - W.xy), vec2(1, 0)));
            new_r = 0.5 * (length(N.xy - new_pos) + length(W.xy - new_pos));
        }
        // No W
        else if (index.x == 0)
        {
            E = imageLoad(i_image, ivec2(index.x + 1, index.y));
            theta2 = acos(dot(normalize(new_pos - W.xy), vec2(1, 0)));
            new_r = 0.5 * (length(N.xy - new_pos) + length(E.xy - new_pos) + length(W.xy - new_pos));
        }
        else
        {
            W = imageLoad(i_image, ivec2(index.x - 1, index.y));
            E = imageLoad(i_image, ivec2(index.x + 1, index.y));
            float theta2 = acos(dot(normalize(E.xy - W.xy), vec2(1, 0)));
            new_r = 0.333 * (length(N.xy - new_pos) + length(E.xy - new_pos) + length(W.xy - new_pos));
        }
        new_theta = (theta1 + theta2) / 2.0;
    }
    imageStore(o_image, index, ivec4(new_pos.x, new_pos.y, new_theta, new_r));
}
