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
    vec4 N, S, E, W;
    vec2 N_s, S_n, E_w, W_e;
    vec2 new_pos, temp_pos;
    float new_theta, new_r;
    float theta1, theta2;
    index = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_LocalInvocationID.xy);
    vec2 P = imageLoad(i_image, index).xy;
    W = imageLoad(i_image, ivec2(index.x - 1, index.y));
    W_e = W.xy + (cos(W.z) * vec2(1.0, 0) + sin(W.z) * vec2(0, 1.0)) * W.w * 1.5;
    E = imageLoad(i_image, ivec2(index.x + 1, index.y));
    E_w = E.xy + (cos(E.z - PI) * vec2(1.0, 0) + sin(E.z - PI) * vec2(0, 1.0)) * E.w;
    S = imageLoad(i_image, ivec2(index.x, index.y - 1));
    S_n = S.xy + (cos(S.z + PI / 2.0) * vec2(1.0, 0) + sin(S.z + PI / 2.0) * vec2(0, 1.0)) * S.w;
    N = imageLoad(i_image, ivec2(index.x, index.y + 1));
    N_s = N.xy + (cos(N.z - PI / 2.0) * vec2(1.0, 0) + sin(N.z - PI / 2.0) * vec2(0, 1.0)) * N.w;

    // if center
    if (index.x != 0 && index.x != imageSize(i_image).x - 1 && index.y != 0 && index.y != imageSize(i_image).y - 1)
    {
        new_pos = 0.25 * (N.xy + S.xy + E.xy + W.xy);
        theta1 = acos(dot(normalize(N.xy - S.xy), vec2(1, 0))) - PI/2.0;
        theta2 = acos(dot(normalize(E.xy - W.xy), vec2(1, 0)));
        new_theta = (theta1 + theta2) / 2.0;
        new_r = 0.25 * (length(N.xy - P) + length(S.xy - P) + length(E.xy - P) + length(W.xy - P));
        imageStore(o_image, index, vec4(round(new_pos.x), round(new_pos.y), new_theta, new_r));
        return;
    }
    else if (index.x == 0 && index.y != 0 && index.y != imageSize(i_image).y - 1)
    // No W, has N and S
    {
        temp_pos = 0.5 * E_w + 0.25 * (N.xy + S.xy);
    }
    else if (index.x == imageSize(i_image).x - 1 && index.y != 0 && index.y != imageSize(i_image).y - 1)
    // No E, has N and S
    {
        temp_pos = 0.5 * W_e + 0.25 * (N.xy + S.xy);
    }
    else if (index.y == imageSize(i_image).y - 1)
    // No N
    {
        // No N, no E
        if (index.x == imageSize(i_image).x - 1)
        {
            temp_pos = 0.5 * (W_e + S_n);
        }
        // No N, No W
        else if (index.x == 0)
        {
            temp_pos = 0.5 * (S_n + E_w);
        }
        else
        // No N, has E, W
        {
            temp_pos = 0.5 * S_n + 0.25 * (E.xy + W.xy);
        }
    }
    else if (index.y == 0)
    // No S
    {
        // No S, No E
        if (index.x == imageSize(i_image).x - 1)
        {
            temp_pos = 0.5 * (N_s + W_e);
        }
        // No S, No W
        else if (index.x == 0)
        {
            temp_pos = 0.5 * (N_s + E_w);
        }
        else
        // No S, has E, W
        {
            temp_pos = 0.5 * N_s + 0.25* ( E.xy + W.xy);
        }
    }
    vec2 dir_fromO = normalize(temp_pos - circle.center);
    new_pos = circle.center + circle.radius * dir_fromO;
    imageStore(o_image, index, vec4(round(new_pos.x), round(new_pos.y), 0, 0));
}
