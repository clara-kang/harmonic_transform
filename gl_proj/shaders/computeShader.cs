#version 430

layout (local_size_x = 32, local_size_y = 32) in;

layout(binding = 1, rgba8ui) readonly uniform uimage2D i_image;
layout(binding = 3, r8ui) readonly uniform uimage2D ptrs;
layout(binding = 2) writeonly uniform image2D o_image;


uniform vec4 A[4];
uniform vec4 B[4];

void main(){
    ivec2 index;
    index = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_LocalInvocationID.xy);
    vec4 trans_index = imageLoad(ptrs, index);
    //vec2 color;
    //color.r = float(index.x) / float(imageSize(i_image).x);
    //color.g = float(index.y) / float(imageSize(i_image).y);
    //imageStore(o_image, index, vec4(color, 1.0, 1.0));
    if (trans_index.x == 1)
    {
        ivec2 trans_loc;
        trans_loc.x = int(dot(A[int(trans_index.x)], vec4(1.0, index.x, index.y, index.x * index.y)));
        trans_loc.y = int(dot(B[int(trans_index.x)], vec4(1.0, index.x, index.y, index.x * index.y)));
        vec4 color = imageLoad(i_image, trans_loc);
        imageStore(o_image, index, vec4(color) / 255.0);
    }
    else
    {
        imageStore(o_image, index, vec4(1.0, 1.0, 1.0, 1.0));
    }
}
