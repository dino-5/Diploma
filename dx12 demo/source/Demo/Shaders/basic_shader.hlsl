struct PS_Input
{
	float4 pos: SV_POSITION;
    float2 uv : UV;
};

struct General 
{
    float scale;
    float3 translate;
    uint sampler_index;
};

struct Vertex
{
    float3 pos;
    float2 uv;
};

struct PassInfo
{
    uint vertexBufferIndex;
    uint constantBufferIndex;
    uint textureBufferIndex;
};

ConstantBuffer<PassInfo> passInfo : register(b0, space10);
SamplerState g_sampler : register(s0);
SamplerState g_sampler1 : register(s1);

float2 getTextureResolution(Texture2D tex)
{
    float2 textureResolution;
    tex.GetDimensions(textureResolution.x, textureResolution.y);
    return textureResolution;
}

float2 uvToTextureCoords(float2 uv, Texture2D tex)
{
    float2 textureResolution = getTextureResolution(tex);
    uv = uv * textureResolution + 0.5;
    return uv;
}

float2 TextureToUvCoords(float2 uv, Texture2D tex)
{
    float2 textureResolution = getTextureResolution(tex);
    uv = (uv - 0.5) / textureResolution;
    return uv;
}

void splitUvIntoIntegerFraction(float2 uv, out float2 iuv, out float2 fuv)
{
    iuv = floor(uv);
    fuv = uv - iuv;
}

float4 bilinearWithNonlinearTexCoords(Texture2D tex, SamplerState smpl, float2 uv)
{
    uv = uvToTextureCoords(uv, tex);
    float2 iuv, fuv;
    splitUvIntoIntegerFraction(uv, iuv, fuv);

    uv = iuv + fuv * fuv * (3.0 - 2.0 * fuv);
    uv = TextureToUvCoords(uv, tex);

    return tex.Sample(smpl, uv);
}

float4 textureBicubic(Texture2D tex, SamplerState sampl, float2 uv)
{
    float2 texture_size = getTextureResolution(tex);
    float2 uv_grid = uvToTextureCoords(uv, tex) -1;
    float2 index = floor(uv_grid);
    float2 fraction = uv_grid - index;
    float2 one_frac = 1.0 - fraction;

    float2 w0 = 1.0 / 6.0 * one_frac * one_frac * one_frac;
    float2 w1 = 2.0 / 3.0 - 0.5 * fraction * fraction * (2.0 - fraction);
    float2 w2 = 2.0 / 3.0 - 0.5 * one_frac * one_frac * (2.0 - one_frac);
    float2 w3 = 1.0 / 6.0 * fraction * fraction * fraction;

    float2 g0 = w0 + w1;
    float2 g1 = w2 + w3;
    float2 mult = 1.0 / texture_size;
    float2 h0 = mult * ((w1 / g0) - 0.5 + index); //h0 = w1/g0 - 1, move from [-0.5, texture_size-0.5] to [0,1]
    float2 h1 = mult * ((w3 / g1) + 1.5 + index); //h1 = w3/g1 + 1, move from [-0.5, texture_size-0.5] to [0,1]

        // Fetch the four linear interpolations
        // Weighting and fetching is interleaved for performance and stability reasons
    float4 tex000 = tex.Sample(sampl, h0);
    float4 tex100 = tex.Sample(sampl, float2(h1.x, h0.y));
    tex000 = tex100 * (1-g0.x) + tex000 *(g0.x); // Weigh along the x-direction

    float4 tex010 = tex.Sample(sampl, float2(h0.x, h1.y));
    float4 tex110 = tex.Sample(sampl, float2(h1.x, h1.y));
    tex010 = tex110 * (1 - g0.x) + tex010 * (g0.x); // Weigh along the x-direction

    return tex010 * (1 - g0.y) + tex000 * (g0.y); // Weigh along the y-direction
}


float4 sampleTex(Texture2D tex, uint index, float2 uv)
{
    if(index==1)
        return tex.Sample(g_sampler1, uv);
    if(index==3)
        return bilinearWithNonlinearTexCoords(tex, g_sampler1, uv);
    if(index==2)
        return textureBicubic(tex, g_sampler1, uv);
    return tex.Sample(g_sampler, uv);
}

PS_Input VS_Basic(uint index : SV_VertexID)
{
    PS_Input ret;
    StructuredBuffer<Vertex> vertexBuffer = ResourceDescriptorHeap[passInfo.vertexBufferIndex];
    ConstantBuffer<General> buffer = ResourceDescriptorHeap[passInfo.constantBufferIndex];
    Vertex vertex = vertexBuffer.Load(index);
    vertex.pos.xyz += buffer.translate;
    vertex.pos.xy *= buffer.scale;
    ret.pos = float4(vertex.pos, 1.0f);
    ret.uv = vertex.uv;
    return ret;
}


float4 PS_Basic(PS_Input input): SV_Target
{
    Texture2D tex = ResourceDescriptorHeap[passInfo.textureBufferIndex];
    ConstantBuffer<General> buffer = ResourceDescriptorHeap[passInfo.constantBufferIndex];

    return sampleTex(tex, buffer.sampler_index, input.uv);

}